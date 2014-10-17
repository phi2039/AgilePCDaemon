#ifndef CSOCKETCOMMANDSERVICE_H
#define	CSOCKETCOMMANDSERVICE_H

#include <queue>

#include "thread.h"
#include "socket.h"

using namespace std;

class CPacketReader
{
public:
  CPacketReader(CIOSocket* pSocket, unsigned int maxData);
  int Read(void* pBuffer, unsigned int bytesToRead, int timeout = APC_SOCKET_TIMEOUT_NOWAIT);
  int GetBytesLeft() {return m_BytesLeft;}
  void SkipBytes(unsigned int bytesToSkip);
protected:
  CIOSocket* m_pSocket;
  unsigned int m_BytesLeft;
};

class IClientCallback
{
public:
  virtual ~IClientCallback() {};
  virtual void OnReceive(CPacketReader& reader) = 0;
};


class CClientSession : public CThread
{
public:
    CClientSession(CIOSocket* pSocket, IClientCallback* pCallback);
    virtual ~CClientSession();
    virtual void Stop();
protected:
    virtual int Run();
    CIOSocket* m_pSocket;
    IClientCallback* m_pCallback;
private:
};

class CSocketCommandService : public CThread, public IClientCallback
{
public:
    CSocketCommandService();
    virtual ~CSocketCommandService();
    virtual bool Initialize();
    void OnReceive(CPacketReader& reader);
protected:
    virtual int Run();
    virtual void Shutdown();
    CServerSocket* m_pLocalSocket;
    queue<CClientSession*> m_Clients;
private:

};

#endif	/* CSOCKETCOMMANDSERVICE_H */

