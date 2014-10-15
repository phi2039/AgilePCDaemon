#include "thread.h"
#include "logging.h"

CThread::CThread() :
    m_ThreadId(0),
    m_Result(0),
    m_Complete(false),
    m_QuitFlag(false)
{
        
}

bool CThread::Start()
{
    m_Complete = false;
    int status = pthread_create(&m_ThreadId, NULL, CThread::StartProc, (void*) this);
    if (status == 0)
    {
        CLog::Write(APC_LOG_FLAG_DEBUG, "Thread", "Started new thread");
        return true;
    }
    CLog::Write(APC_LOG_FLAG_ERROR, "Thread", "Failed to start thread. Error: %d", status);
    m_Complete = true; // Must be complete...even though nothing happened...
    return false;
}

bool CThread::IsComplete()
{
    return m_Complete;
}

void* CThread::StartProc(void* pParam)
{
    // TODO: Use "assert" to make sure this is a valid cast?
    // http://www.cs.gmu.edu/~rcarver/ModernMultithreading/LectureNotes/Chapter1PthreadsThreadClass.pdf
    CThread* pThread = (CThread*)pParam;
    pThread->m_QuitFlag = false;
    
    pThread->m_Result = pThread->Run();
    
    CLog::Write(APC_LOG_FLAG_DEBUG, "Thread", "Thread exited with status %d", pThread->m_Result);
    pThread->m_ThreadId = 0;
    pThread->m_Complete = true;
}

// TODO: Implement timeout/kill mechanism
void CThread::Stop()
{
    if (!m_ThreadId || m_Complete)
        return; // Thread is not running
    
    CLog::Write(APC_LOG_FLAG_DEBUG, "Thread", "Stopping thread");
    m_QuitFlag = true;
    // TODO: Trigger quit event
    
    void* pResult;
    pthread_join(m_ThreadId, NULL); // Wait for thread to complete
    
    CLog::Write(APC_LOG_FLAG_DEBUG, "Thread", "Stopped thread");
    m_ThreadId = 0;
}