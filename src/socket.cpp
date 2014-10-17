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

int CSocket::PollIn(int timeout)
{
    int err = APC_SOCKET_NOERR;

    pollfd p;
    p.fd = m_Descriptor;
    p.events = POLLIN; // There is data to read
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

// TODO: Thread safety for interrupt events
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
        memset(&addr, 0, sizeof (addr));
        addr.sun_family = AF_LOCAL;
        strncpy(addr.sun_path, m_Endpoint.c_str(), sizeof (addr.sun_path) - 1);
        if (bind(m_Descriptor, (sockaddr*) & addr, sizeof (addr)) == -1)
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
        if (bind(m_Descriptor, (sockaddr*) & addr, sizeof (addr)) == -1)
            return false;
    }

    // Set socket as passive listener (and begin listening for connections)
    if (listen(m_Descriptor, queueLen) == -1)
        return false;

    return true;
}

// TODO: Thread safety for interrupt events

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