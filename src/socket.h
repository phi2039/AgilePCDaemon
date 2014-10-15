#ifndef SOCKET_H
#define	SOCKET_H

#include <string>

using namespace std;

class CSocket
{
public:
    CSocket(int domain, int type, int protocol);
    CSocket(int desc);
    virtual ~CSocket();
    void Close();
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
    size_t Read(void* pBuf, size_t bufLen, unsigned int timeout /*, Event* pSignalEvent*/);
    size_t Write(void* pBuf, size_t bufLen);
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
    CIOSocket* Accept(unsigned int timeout /*, Event* pSignalEvent*/);
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

#endif