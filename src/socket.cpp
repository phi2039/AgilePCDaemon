#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/eventfd.h>
#include <netinet/in.h>
#include <netdb.h>
#include <poll.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <map>

#include "socket.h"
#include "logging.h"

// http://stackoverflow.com/questions/3444729/using-accept-and-select-at-the-same-time
///////////////////////////////////////////////////////////////////////////////////////
// Socket Base Class

CSocket::CSocket(int domain, int type, int protocol)
{
    m_Descriptor = socket(domain, type, protocol);
    if (m_Descriptor == -1)
        m_Descriptor = 0;
}

CSocket::CSocket(int desc) :
m_Descriptor(desc)
{

}

CSocket::~CSocket()
{
    Close();
}

void CSocket::Close()
{
    if (m_Descriptor)
    {
        close(m_Descriptor);
        m_Descriptor = 0;
    }
}

void CSocket::Shutdown()
{
    shutdown(m_Descriptor, SHUT_RD); // Unblock any poll() operations
}

// TODO: Should we set some sort of flag/event here that lets others know we are likely blocking?
//   So that we know we need to Shutdown() first...
int CSocket::PollIn(int timeout)
{
    int err = APC_SOCKET_NOERR;

    pollfd p;
    p.fd = m_Descriptor;
    p.events = POLLIN | POLLHUP; // There is data to read or the remote endpoint disconnected
    p.revents = 0; // Reset flags

    // Wait for data on the socket
    int result = poll(&p, 1, timeout);
    if (result == 0)
        return APC_SOCKET_TIMEOUT;
    else if (result == -1) // An error occurred in poll()
        return APC_SOCKET_RCV_ERROR;
    else if (p.revents & POLLHUP) // Client disconnected
        return APC_SOCKET_NOT_CONN;

    socklen_t errlen = sizeof (int);
    getsockopt(m_Descriptor, SOL_SOCKET, SO_ERROR, (void*) &err, &errlen);
    if (err)
        return APC_SOCKET_RCV_ERROR;

    return APC_SOCKET_NOERR;
}

///////////////////////////////////////////////////////////////////////////////////////
// IO Socket Class

CIOSocket::CIOSocket(int domain, int type, int protocol) :
CSocket(domain, type, protocol)
{

}

CIOSocket::CIOSocket(int desc) :
CSocket(desc)
{

}

CIOSocket::~CIOSocket()
{

}

size_t CIOSocket::Read(void* pBuf, size_t bufLen, int timeout /*=-1*/)
{
    if (!m_Descriptor)
        return -1;

    int result = PollIn(timeout);
    if (result < 0) // Error
        return result;

    int read = recv(m_Descriptor, pBuf, bufLen, 0);
    if (read == -1)
        return APC_SOCKET_RCV_ERROR;

    return read;
}

size_t CIOSocket::Write(void* pBuf, size_t bufLen)
{
    if (!m_Descriptor)
        return -1;

    // TODO: poll() to see if FIFO space is available?)
    int sent = send(m_Descriptor, pBuf, bufLen, 0);
    if (sent == -1)
        return -1;

    return sent;
}

int CIOSocket::GetReadSize()
{
    int err = APC_SOCKET_NOERR;

    int dataLen = 0;
    if (ioctl(m_Descriptor, FIONREAD, &dataLen) == -1)
    {
        socklen_t errlen = sizeof (err);
        getsockopt(m_Descriptor, SOL_SOCKET, SO_ERROR, (void*) &err, &errlen);
        if (err)
            return APC_SOCKET_RCV_ERROR;
    }
    return dataLen;
}

///////////////////////////////////////////////////////////////////////////////////////
// Server (Listener) Socket

CServerSocket::CServerSocket(int domain, int type, int protocol) :
CSocket(domain, type, protocol)
{

}

CServerSocket::~CServerSocket()
{

}

bool CServerSocket::Listen(int queueLen)
{
    // Set socket as passive listener (and begin listening for connections)
    if (listen(m_Descriptor, queueLen) == -1)
        return false;

    return true;
}

CIOSocket* CServerSocket::Accept(int timeout /*=-1*/)
{
    if (!m_Descriptor)
        return NULL;

    int result = PollIn(timeout);
    if (result < 0) // Error
        return NULL;

    int clientDesc = accept(m_Descriptor, NULL, 0);
    if (clientDesc == -1)
        return NULL;

    return new CIOSocket(clientDesc);
}

///////////////////////////////////////////////////////////////////////////////////////
// TCP Implementation of Server Socket Class

CTcpServerSocket::CTcpServerSocket(const string& host, int port) :
CServerSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
m_Port(0),
m_Host(host)
{
    // TODO: This is UGLY!!! (but it works))
    char portString[7];
    snprintf(portString, sizeof(portString)-1, "%d", port);
    m_Endpoint = host;
    m_Endpoint.append(":");
    m_Endpoint.append(portString);
}

CTcpServerSocket::~CTcpServerSocket()
{

}

bool CTcpServerSocket::Open(int queueLen /*=5*/)
{
    // Bind Endpoint
    sockaddr_in addr;
    memset(&addr, 0, sizeof (addr));
    addr.sin_family = AF_INET;
    hostent* pHost = gethostbyname(m_Host.c_str());
    if (pHost == NULL) // Could not resolve name
        return false;
    addr.sin_addr.s_addr = *((unsigned long*) pHost->h_addr_list[0]); // Use the first available address
    addr.sin_port = htons(m_Port); // Assign in network byte order
    if (bind(m_Descriptor, (sockaddr*) & addr, sizeof (addr)) == -1)
        return false;

    return Listen(queueLen);
}


///////////////////////////////////////////////////////////////////////////////////////
// UNIX Domain Implementation of Server Socket Class

CLocalServerSocket::CLocalServerSocket(const string& path) :
CServerSocket(AF_LOCAL, SOCK_STREAM, 0),
m_Path(path)
{
    m_Endpoint = path;
}

CLocalServerSocket::~CLocalServerSocket()
{

}

bool CLocalServerSocket::Open(int queueLen /*=5*/)
{
    if (!m_Descriptor)
        return false;

    // Bind Endpoint
    unlink(m_Path.c_str()); // Clean up from prior instance, if necessary

    sockaddr_un addr;
    memset(&addr, 0, sizeof (addr));
    addr.sun_family = AF_LOCAL;
    strncpy(addr.sun_path, m_Path.c_str(), sizeof (addr.sun_path) - 1);
    if (bind(m_Descriptor, (sockaddr*) & addr, sizeof (addr)) == -1)
        return false;

    return Listen(queueLen);
}

///////////////////////////////////////////////////////////////////////////////////////
// Client Socket Class

CClientSocket::CClientSocket(const string& path) :
CIOSocket(AF_LOCAL, SOCK_STREAM, 0),
m_Endpoint(path),
m_Port(-1)
{

}

CClientSocket::CClientSocket(const string& address, int port) :
CIOSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
m_Endpoint(address),
m_Port(port)
{

}

CClientSocket::~CClientSocket()
{

}

bool CClientSocket::Connect()
{
    if (!m_Descriptor) // No socket
        return false;

    if (m_Port == -1) // Local socket
    {
        unlink(m_Endpoint.c_str()); // Clean up from prior instance, if necessary

        sockaddr_un addr;
        memset(&addr, 0, sizeof (addr));
        addr.sun_family = AF_LOCAL;
        strncpy(addr.sun_path, m_Endpoint.c_str(), sizeof (addr.sun_path) - 1);
        if (connect(m_Descriptor, (sockaddr*) & addr, sizeof (addr)) == -1)
            return false;
    }
    else // Network socket
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof (addr));
        addr.sin_family = AF_INET;
        hostent* pHost = gethostbyname(m_Endpoint.c_str());
        if (pHost == NULL) // Could not resolve name
            return false;
        addr.sin_addr.s_addr = *((unsigned long*) pHost->h_addr_list[0]); // Use the first available address
        addr.sin_port = htons(m_Port); // Assign in network byte order
        if (connect(m_Descriptor, (sockaddr*) & addr, sizeof (addr)) == -1)
            return false;
    }
    return true;
}

///////////////////////////////////////////////////////////////////////////////////////
// Multi-point Socket Server Class

CMultiSocketServer::CMultiSocketServer()
{

}

CMultiSocketServer::~CMultiSocketServer()
{
    CloseAll();
}

bool CMultiSocketServer::Open(const string& path, int queueLen /*=5*/)
{
    CServerSocket* pSocket = new CLocalServerSocket(path);
    return Open(pSocket, queueLen);
}

bool CMultiSocketServer::Open(const string& address, int port, int queueLen /*=5*/)
{
    CServerSocket* pSocket = new CTcpServerSocket(address, port);
    return Open(pSocket, queueLen);
}

bool CMultiSocketServer::Open(CServerSocket* pSocket, int queueLen)
{
    if (!pSocket)
        return false;

    if (pSocket->Open(queueLen))
    {
        m_Sockets[pSocket->GetEndpoint()] = pSocket; // Add to the list of open sockets
        return true;
    }

    delete pSocket;
    return false;
}

CServerSocket* CMultiSocketServer::FindSocket(const string& endpoint)
{
    ServerSocketIterator it = m_Sockets.find(endpoint);
    if (it != m_Sockets.end())
        return it->second;

    return NULL;
}

CIOSocket* CMultiSocketServer::Accept(int timeout /*-1*/)
{
    // TODO: There is definitely a better way to do this...no need to rebuild these every time...
    int socketCount = m_Sockets.size();
    pollfd* pFd = (pollfd*) malloc(sizeof (pollfd) * socketCount);
    for (ServerSocketIterator it = m_Sockets.begin(); it != m_Sockets.end(); ++it)
    {
        pFd->fd = it->second->GetDescriptor();
        pFd->events = POLLIN;
        pFd->revents = 0;
    }

    CIOSocket* pResult = NULL;

    // Wait for data on any of the sockets
    int result = poll(pFd, socketCount, timeout);
    if (result == 0)
        pResult = NULL;
    else if (result == -1) // An error occurred in poll()
        pResult = NULL;
    else // There are events
    {
        // Loop through open descriptors and accept the first one found
        // TODO: There is probably a better way to prioritize...but this is simplest...
        for (int s = 0; s < socketCount; s++)
        {
            if (pFd[s].revents & POLLIN)
            {
                // TODO: What happens when an error occurs here?
                int err = 0;
                socklen_t errlen = sizeof (int); // Check for socket errors
                getsockopt(pFd[s].fd, SOL_SOCKET, SO_ERROR, (void*) &err, &errlen);
                if (!err)
                {
                    int clientDesc = accept(pFd[s].fd, NULL, 0);
                    if (clientDesc > 0)
                    {
                        pResult = new CIOSocket(clientDesc);
                        break;
                    }
                }
            }
            pResult = NULL;
        }
    }
    free(pFd);
    return pResult;
}

void CMultiSocketServer::Shutdown(const string& endpoint)
{
    // Find the descriptor in the open socket list
    CServerSocket* pSocket = FindSocket(endpoint);
    if (pSocket)
        pSocket->Shutdown();
}

void CMultiSocketServer::ShutdownAll()
{
    for (ServerSocketIterator it = m_Sockets.begin(); it != m_Sockets.end(); ++it)
    {
        CServerSocket* pSocket = it->second;
        pSocket->Shutdown();
    }
}

// TODO: Synchronize list operations?
void CMultiSocketServer::Close(const string& endpoint)
{
    // Find the descriptor in the open socket list
    CServerSocket* pSocket = FindSocket(endpoint);
    if (pSocket)
    {
        m_Sockets.erase(endpoint);
        pSocket->Close();
        delete pSocket;
    }
}

// TODO: If anyone tries to access this list now...things will be bad...
void CMultiSocketServer::CloseAll()
{
    for (ServerSocketIterator it = m_Sockets.begin(); it != m_Sockets.end(); ++it)
    {
        CServerSocket* pSocket = it->second;
        // TODO: Make sure it is Shutdown() first???
        pSocket->Close();
        delete pSocket;
    }
    m_Sockets.clear();
}