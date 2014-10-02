#ifndef SYNC_H
#define	SYNC_H

#include <pthread.h>

// Simple Thread Synchronization Event Wrapper
//////////////////////////////////////////////
class CThreadSyncEvent
{
public:
  CThreadSyncEvent();
  virtual ~CThreadSyncEvent();
  int Wait(int timeout = -1);
  void Set();
  void Reset();
protected:
  pthread_mutex_t m_Mutex;
  pthread_cond_t m_Cond;
  bool m_Signaled;
};

// Simple Mutex Wrapper
//////////////////////////////////////////////
class CMutexLock
{
public:
  inline CMutexLock(pthread_mutex_t m) :
    m_Mutex(m)
  {
    pthread_mutex_lock(&m_Mutex);
  }
  inline ~CMutexLock()
  {
    pthread_mutex_unlock(&m_Mutex);  
  }
protected:
  pthread_mutex_t m_Mutex;
};


#endif	/* SYNC_H */

