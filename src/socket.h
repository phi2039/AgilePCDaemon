#ifndef SOCKET_H
#define	SOCKET_H

#include <string>
#include <list>

using namespace std;

enum
{
    SKT_SIGNAL_INTR = 1
};
enum
{
    APC_SOCKET_TIMEOUT_INF    = -1,
    APC_SOCKET_TIMEOUT_NOWAIT = 0
};

enum
{
  APC_SOCKET_NOERR          = 0,
  APC_SOCKET_SEND_ERROR     = -100,
  APC_SOCKET_RCV_ERROR      = -101,
  APC_SOCKET_NOT_CONN       = -102,
  APC_SOCKET_INVALID_STATE  = -103,
  APC_SOCKET_TIMEOUT        = -104,
  APC_SOCKET_INVARG         = -105
};

class CSocket
{
public:
    CSocket(int domain, int type, int protocol);
    CSocket(int desc);
    virtual ~CSocket();
    int PollIn(int timeout);
    void Close();
    void Shutdown();
    operator int() {return m_Descriptor;}
    int GetDescriptor() {return m_Descriptor;}
protected:
    int m_Descriptor;
private:
};

class CIOSocket : public CSocket
{
public:
    CIOSocket(int domain, int type, int protocol);
    CIOSocket(int desc);
    virtual ~CIOSocket();
    size_t Read(void* pBuf, size_t bufLen, int timeout = -1);
    size_t Write(void* pBuf, size_t bufLen);
    int GetReadSize();
protected:
private:
};

class CServerSocket : public CSocket
{
public:
    CServerSocket(const string& path);
    CServerSocket(const string& address, int port);
    virtual ~CServerSocket();
    bool Open(int queueLen = 5);
    CIOSocket* Accept(int timeout = -1);
protected:
    string m_Endpoint;
    int m_Port;
private:
};

class CClientSocket : public CIOSocket
{
public:
    CClientSocket(const string& path);
    CClientSocket(const string& address, int port);
    virtual ~CClientSocket();
    bool Connect();
protected:
    string m_Endpoint;
    int m_Port;
private:
};

class CMultiSocketServer
{
public:
    CMultiSocketServer();
    virtual ~CMultiSocketServer();
    bool Open(const string& path, int queueLen = 5);
    bool Open(const string& address, int port, int queueLen = 5);
    void Close(int id);
    void CloseAll();
    CIOSocket* Accept(int timeout = -1);
protected:
    bool Open(CServerSocket* pSocket, int queueLen);
    void Close(CServerSocket* pSocket);
    list<CServerSocket*> m_Sockets;
private:
};

#endif