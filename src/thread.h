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
    virtual void Stop();
    virtual bool WaitComplete(int timeout = -1);
    virtual bool IsComplete();
protected:
    virtual int Run() = 0;
    pthread_t m_ThreadId;
    int m_Result;
    bool m_Complete;
    bool m_QuitFlag;
private:
    static void* StartProc(void* pParam);
    void Complete();
    CThreadSyncEvent m_QuitEvent;
};

#endif	/* THREAD_H */
