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

int CSocketCommandService::Run()
{
    // Listen for and service connections
    // TODO: There must be a better way to do this...
    while (true)
    {
        // TODO: Use wait with timeout instead of sleep, so that we can trigger a speedier exit...
        sleep(1);

        // Check for new connections
        // Wait on connection OR quit event
        
        // Read command
        
        // Parse command
        
        // Pass command to handler
        
        // If response requested, register callback
        
        // If no response requested close connection
        
        if (m_QuitFlag)
          break;

    }
    Shutdown();
    return 0;
}

void CSocketCommandService::Shutdown()
{
    // Clean up local socket, if it exists
    if (m_pLocalSocket)
    {
        m_pLocalSocket->Close();
        m_pLocalSocket = NULL;
    }
}
