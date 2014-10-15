#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>
#include <netdb.h>

#include "socket.h"

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

// TODO: Timeout and event
size_t CIOSocket::Read(void* pBuf, size_t bufLen, unsigned int timeout /*, Event* pSignalEvent*/)
{
    if (!m_Descriptor)
        return -1;
    
    int read = recv(m_Descriptor, pBuf, bufLen, 0);
    if (read == -1)
        return -1;
    
    return read;
}

size_t CIOSocket::Write(void* pBuf, size_t bufLen)
{
    if (!m_Descriptor)
        return -1;
    
    int sent = send(m_Descriptor, pBuf, bufLen, 0);
    if (sent == -1)
        return -1;
    
    return sent;    
}

///////////////////////////////////////////////////////////////////////////////////////
// Server (Listener) Socket
CServerSocket::CServerSocket(const string& path) :
    CSocket(AF_LOCAL, SOCK_STREAM, 0),
    m_Endpoint(path),
    m_Port(-1)
{
    
}

CServerSocket::CServerSocket(const string& address, int port) :
    CSocket(AF_INET, SOCK_STREAM, IPPROTO_TCP),
    m_Endpoint(address),
    m_Port(port)
{
    
}

CServerSocket::~CServerSocket()
{

}

bool CServerSocket::Open(int queueLen /*=5*/)
{
    if (!m_Descriptor)
        return false;
    
    // Bind
    // TODOL Helper functions for repeated code...
     if (m_Port == -1) // Local socket
    {
        unlink(m_Endpoint.c_str()); // Clean up from prior instance, if necessary
        
        sockaddr_un addr;
        memset(&addr,0,sizeof(addr));
        addr.sun_family = AF_LOCAL;
        strncpy(addr.sun_path, m_Endpoint.c_str(), sizeof(addr.sun_path) - 1);
        if (bind(m_Descriptor, (sockaddr*) &addr, sizeof(addr)) == -1)
            return false;
    }
    else // Network socket
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        hostent* pHost = gethostbyname(m_Endpoint.c_str());
        if (pHost == NULL) // Could not resolve name
            return false;
        addr.sin_addr.s_addr = *((unsigned long*) pHost->h_addr_list[0]); // Use the first available address
        addr.sin_port = htons(m_Port); // Assign in network byte order
        if (bind(m_Descriptor, (sockaddr*) &addr, sizeof(addr)) == -1)
            return false;        
    }   
    
    // Set socket as passive listener
    if (listen(m_Descriptor, queueLen) == -1)
        return false;
}

// TODO: Timeout and event
CIOSocket* CServerSocket::Accept(unsigned int timeout /*, Event* pSignalEvent*/)
{
    if (!m_Descriptor)
        return NULL;
    
    int clientDesc = accept(m_Descriptor, NULL, 0);
    if (clientDesc == -1)
        return NULL;

    return new CIOSocket(clientDesc);
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
        memset(&addr,0,sizeof(addr));
        addr.sun_family = AF_LOCAL;
        strncpy(addr.sun_path, m_Endpoint.c_str(), sizeof(addr.sun_path) - 1);
        if (connect(m_Descriptor, (sockaddr*) &addr, sizeof(addr)) == -1)
            return false;
    }
    else // Network socket
    {
        sockaddr_in addr;
        memset(&addr, 0, sizeof(addr));
        addr.sin_family = AF_INET;
        hostent* pHost = gethostbyname(m_Endpoint.c_str());
        if (pHost == NULL) // Could not resolve name
            return false;
        addr.sin_addr.s_addr = *((unsigned long*) pHost->h_addr_list[0]); // Use the first available address
        addr.sin_port = htons(m_Port); // Assign in network byte order
        if (connect(m_Descriptor, (sockaddr*) &addr, sizeof(addr)) == -1)
            return false;        
    }
    return true;
}