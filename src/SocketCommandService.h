#ifndef CSOCKETCOMMANDSERVICE_H
#define	CSOCKETCOMMANDSERVICE_H

#include "thread.h"
#include "socket.h"

class CSocketCommandService : public CThread
{
public:
    CSocketCommandService();
    virtual ~CSocketCommandService();
    virtual bool Initialize();
protected:
    virtual int Run();
    virtual void Shutdown();
    CServerSocket* m_pLocalSocket;
private:

};

#endif	/* CSOCKETCOMMANDSERVICE_H */

