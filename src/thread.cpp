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


int CThread::GetResult()
{
    return m_Result;
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
    
    return NULL;
}

void CThread::OnStop() 
{

}

// TODO: Implement timeout/kill mechanism
int CThread::Stop(int timeout /*-1*/)
{
    if (!m_ThreadId || m_Complete)
        return 0; // Thread is not running
    
    CLog::Write(APC_LOG_FLAG_DEBUG, "Thread", "Stopping thread");
    
    // Signal quit
    m_QuitFlag = true;

    // Allow derived class to perform cleanup/signaling before exiting
    // (i.e. unblock blocking operations so it can check the quit flag)
    OnStop();
    
    //pthread_timedjoin_np(m_ThreadId, NULL, abstime); // Wait for thread to complete
    pthread_join(m_ThreadId, NULL); // Wait for thread to complete (forever))
    m_ThreadId = 0;
    
    CLog::Write(APC_LOG_FLAG_DEBUG, "Thread", "Stopped thread");
}