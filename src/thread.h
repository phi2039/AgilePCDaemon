#ifndef THREAD_H
#define	THREAD_H

#include <pthread.h>
#include "sync.h"

class CThread
{
public:
    CThread();
    virtual ~CThread() {}
    virtual bool Start();
    virtual int Stop(int timeout = -1);
    virtual bool IsComplete();
    virtual int GetResult();
protected:
    virtual int Run() = 0;
    virtual void OnStop();
    pthread_t m_ThreadId;
    int m_Result;
    bool m_Complete;
    bool m_QuitFlag;
private:
    static void* StartProc(void* pParam);
};

#endif	/* THREAD_H */
