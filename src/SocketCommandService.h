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
    virtual void Close();
protected:
// CThread Methods    
    virtual int Run();
    virtual void OnStop();    
// Local Implementation    
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
    virtual void Shutdown();
// IClientCallback Overrides    
    void OnReceive(CPacketReader& reader);
protected:
// CThread Overrides
    virtual int Run();
    virtual void OnStop();
// Local Implementation
    virtual bool AddClient(CClientSession* pSession);
    virtual void Close();
    //CServerSocket* m_pLocalSocket;
    queue<CClientSession*> m_Clients;
    
    CMultiSocketServer* m_pSocketServer;
private:

};

#endif	/* CSOCKETCOMMANDSERVICE_H */

