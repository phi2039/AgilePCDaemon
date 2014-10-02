#include "sync.h"

// Simple Thread Synchronization Event Wrapper
//////////////////////////////////////////////
CThreadSyncEvent::CThreadSyncEvent() : 
m_Signaled(false)
{
  pthread_mutex_init(&m_Mutex, NULL);
  pthread_cond_init(&m_Cond, NULL);
}

CThreadSyncEvent::~CThreadSyncEvent()
{
  pthread_mutex_destroy(&m_Mutex);
  pthread_cond_destroy(&m_Cond);
}

int CThreadSyncEvent::Wait(int timeout /*=-1*/)
{
  pthread_mutex_lock(&m_Mutex);
  if (!m_Signaled)
    pthread_cond_wait(&m_Cond, &m_Mutex);
  pthread_mutex_unlock(&m_Mutex);
  
  return 0;
}

void CThreadSyncEvent::Set()
{
  pthread_mutex_lock(&m_Mutex);
  m_Signaled = true;
  pthread_cond_signal(&m_Cond);
  pthread_mutex_unlock(&m_Mutex);    
}

void CThreadSyncEvent::Reset()
{
  m_Signaled = false;
}
