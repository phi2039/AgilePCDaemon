#ifndef LOGGING_H
#define	LOGGING_H

#include <stdio.h>
#include <limits.h>
#include <string>
#include <stdarg.h>

#include "sync.h"

// Log Level Definitions
///////////////////////////////////////////////////////
enum 
{
  APC_LOG_FLAG_ERROR     = 0x1,
  APC_LOG_FLAG_WARNING   = 0x1 << 1,
  APC_LOG_FLAG_INFO      = 0x1 << 2,
  APC_LOG_FLAG_DEBUG     = 0x1 << 3
  ///////
  // User flags start at bit 15        
};

enum
{
  APC_LOG_LEVEL_NONE     = 0x0,
  APC_LOG_LEVEL_ERROR    = APC_LOG_FLAG_ERROR,
  APC_LOG_LEVEL_WARNING  = APC_LOG_LEVEL_ERROR | APC_LOG_FLAG_WARNING,
  APC_LOG_LEVEL_INFO     = APC_LOG_LEVEL_ERROR | APC_LOG_FLAG_WARNING | APC_LOG_FLAG_INFO,
  APC_LOG_LEVEL_DEBUG    = APC_LOG_LEVEL_ERROR | APC_LOG_FLAG_WARNING | APC_LOG_FLAG_INFO | APC_LOG_FLAG_DEBUG
};

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Logging interface
class ILog
{
public:
    virtual ~ILog() {}
    virtual bool Open() = 0;
    virtual void Write(int logLevel, const char* facility, const char* format, ...) = 0;
    virtual void WriteV(int logLevel, const char* facility, const char* format, va_list args) = 0;
    virtual void Close() = 0;
    virtual void SetMask(int level) = 0;
    virtual int GetMask() = 0;
protected:
private:
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Global logging facility (wrapper around generic ILog implementation)
class CLog
{
public:
    static void SetInterface(ILog* pLog);
    static bool Open();
    static void Write(int logLevel, const char* facility, const char* format, ...);
    static void Close();
    static void SetMask(int level);
    static int GetMask();
    static bool IsOpen();
protected:
private:
     CLog(); // This is a singleton
     static ILog* m_pInterface;
     static bool m_IsOpen;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Base implementation of ILog interface
class CLogBase : public ILog
{
public:
    CLogBase();
    virtual ~CLogBase();
    virtual bool Open();
    virtual void Write(int logLevel, const char* facility, const char* format, ...);
    virtual void Close();
    virtual void SetMask(int level);
    virtual int GetMask();
protected:
    pthread_mutex_t m_WriteMutex;
    int m_Mask;
private:
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// File-based implementation of ILog interface
class CFileLog : public CLogBase
{
public:
    CFileLog(const char* path);
    virtual ~CFileLog();
    virtual bool Open();
    virtual void WriteV(int logLevel, const char* facility, const char* format, va_list args);
    virtual void Close();
protected:
private:
    FILE* m_pFile;
    string m_Path;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Console-based implementation of ILog interface
class CConsoleLog : public CLogBase
{
public:
    CConsoleLog();
    virtual ~CConsoleLog();
    virtual void WriteV(int logLevel, const char* facility, const char* format, va_list args);
protected:
private:
};

#endif	/* LOGGING_H */