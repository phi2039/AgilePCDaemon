#include <unistd.h>

#include "SocketCommandService.h"
#include "logging.h"
#include "config.h"

CSocketCommandService::CSocketCommandService() :
m_pLocalSocket(NULL)
{

}

CSocketCommandService::~CSocketCommandService()
{
    if (m_pLocalSocket)
        delete m_pLocalSocket;
}

bool CSocketCommandService::Initialize()
{
    // Open the socket for communication
    string socketPath = "/tmp/agilepc";
    CConfig::GetOpt("cmd_local_file", socketPath); // Check for path override in config file

    m_pLocalSocket = new CServerSocket(socketPath);
    if (!m_pLocalSocket->Open())
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "SocketCommandService", "Failed to open server socket at %s", socketPath.c_str());
        return false;
    }

    CLog::Write(APC_LOG_FLAG_INFO, "SocketCommandService", "Listening on local socket at %s", socketPath.c_str());
    return true;
}

// socat - UNIX-CONNECT:/tmp/agilepc
int CSocketCommandService::Run()
{
    // Listen for and service connections
    // TODO: There must be a better way to do this...
    while (!m_QuitFlag)
    {
        // TODO: Use wait with timeout instead of sleep, so that we can trigger a speedier exit...
        sleep(1);

        // TODO: Wait on connection OR quit event
        // TODO: Why can't we wait on this forever and have shutdown() kick us out???
        CIOSocket* pClient = m_pLocalSocket->Accept(1000); // Wait forever...quit event will interrupt
        if (pClient != NULL)
        {
            CClientSession* pSession = new CClientSession(pClient, this);
            pSession->Start();
            
            // TODO: Capture client in "open connections" list...
            m_Clients.push(pSession);
        }
        // TODO: Clean-up idle connections (THIS IS A MEMORY LEAK CURRENTLY!!!)
    }
    Shutdown();
    return 0;
}

void CSocketCommandService::OnReceive(CPacketReader& reader)
{
    char buf[512];
    int bytesRead = reader.Read(buf, sizeof (buf) - 1);
    buf[bytesRead] = '\0'; // NULL terminate
    CLog::Write(APC_LOG_FLAG_INFO, "SocketCommandService", "Read - %s", buf);  
    
    // Parse command

     // Pass command to handler

     // If response requested, register callback

     // If no response requested close connection

}

void CSocketCommandService::Shutdown()
{
    // Clean up local socket, if it exists
    if (m_pLocalSocket)
    {
        m_pLocalSocket->Shutdown();
        m_pLocalSocket->Close();
        delete m_pLocalSocket;
        m_pLocalSocket = NULL;
    }
    
    CLog::Write(APC_LOG_FLAG_INFO, "SocketCommandService", "Disconnecting clients...");
    while (!m_Clients.empty())
    {
        // Clean up client connections
        CClientSession* pSession = m_Clients.front();
        if (!pSession->IsComplete())
        {
            pSession->Stop();
            CLog::Write(APC_LOG_FLAG_INFO, "SocketCommandService", "Disconnected 1 client");
        }
        m_Clients.pop();
    }
}

/////////////////////////////////////////////////////////////////////////////////
// I/O Packet Reader
// Abstraction of read()/recv()
/////////////////////////////////////////////////////////////////////////////////

CPacketReader::CPacketReader(CIOSocket* pSocket, unsigned int maxData) : // No waiting by default
    m_pSocket(pSocket),
    m_BytesLeft(maxData)
{

}

int CPacketReader::Read(void* pBuffer, unsigned int bytesToRead, int timeout /*=APC_SOCKET_TIMEOUT_NOWAIT*/)
{
    if (!bytesToRead)
        return 0; // Ask for none...get none...
    
    if (bytesToRead > m_BytesLeft)
        bytesToRead = m_BytesLeft; // Limit caller to what is known to be available

    int result = m_pSocket->Read(pBuffer, bytesToRead, timeout);
    if (result < 0) // Error
        return result;
    
    m_BytesLeft -= result; // Update state

    return result;
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////
// Client Session Handling

CClientSession::CClientSession(CIOSocket* pSocket, IClientCallback* pCallback) :
    m_pSocket(pSocket),
    m_pCallback(pCallback)
{

}

CClientSession::~CClientSession()
{
    if (m_pSocket)
        delete m_pSocket; // We're responsible for disposing of the provided connection
}

int CClientSession::Run()
{
    if (!m_pSocket || !m_pCallback) // Have to have something to listen to and somewhere to report...
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "ClientSession", "Unable to start receive thread. Socket: %s, Callback: %s", (m_pSocket == NULL) ? "Invalid" : "Valid", (m_pCallback == NULL) ? "Invalid" : "Valid");
        return -1;
    }
    
    CLog::Write(APC_LOG_FLAG_INFO, "ClientSession", "Async receive thread started...");

    // Poll the socket and process packets
    while (true)
    {
        if (m_QuitFlag) // shutdown() will unblock any open requests, so this should be sufficient for quitting
            break;

        // Wait for data on the socket
        int result = m_pSocket->PollIn(APC_SOCKET_TIMEOUT_INF);
        if (result < 0) // Something went wrong
        {
            if (result == APC_SOCKET_NOT_CONN) // Client disconnected
            {
                CLog::Write(APC_LOG_FLAG_INFO, "ClientSession", "Client disconnected");
                delete this; // Self-destruct
                break;
            }
            else
            {
                CLog::Write(APC_LOG_FLAG_ERROR, "ClientSession", "Receive thread error while polling. Result: %d", result);
                continue; // TODO: try to repair the problem, or bail...
            }
        }
        else // Data to read
        {
            int dataLen = m_pSocket->GetReadSize();
            if (dataLen == 0)
            {
                CLog::Write(APC_LOG_FLAG_DEBUG, "ClientSession", "Zero-length data"); // Should not happen...ever...
                continue;
            }
            else // Handle received data via callback
            {                
                CPacketReader reader(m_pSocket, dataLen); // Create a reader interface for the callback (so they can read data directly)
                CLog::Write(APC_LOG_FLAG_DEBUG, "ClientSession", "Received %d bytes (Async) - Notifying client", dataLen);
                m_pCallback->OnReceive(reader);
            }
        }
    }
    CLog::Write(APC_LOG_FLAG_INFO, "ClientSession", "Async receive thread stopped...");
    return 0;
}

// TODO: Is this sequence correct?
void CClientSession::Stop()
{
    // TODO: Synchronize this??
    
    CLog::Write(APC_LOG_FLAG_INFO, "ClientSession", "Async receive thread stopping...");
    m_QuitFlag = true; // Make sure this is set before unblocking poll() (so the polling loops exit...)

    if (m_pSocket)
       m_pSocket->Shutdown();

    CThread::Stop(); // After this, no one will be using the socket
    
    if (m_pSocket)
    {
        m_pSocket->Close();
        delete m_pSocket;
        m_pSocket = NULL;
    }
}
