#include "logging.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Global logging facility (wrapper around ILog implementation)
ILog* CLog::m_pInterface = NULL;
bool CLog::m_IsOpen = false;

CLog::CLog()
{
    
}

void CLog::SetInterface(ILog* pLog)
{
    if (m_pInterface)
    {
        m_pInterface->Close();
        delete m_pInterface;
    }
    m_pInterface = pLog;
    m_IsOpen = pLog->Open();
}

bool CLog::Open()
{
    return (m_pInterface != NULL) ? m_pInterface->Open() : false;
}

// TODO: Speed up detection of mask match - don't waste time on unlogged messages
void CLog::Write(int logLevel, const char* facility, const char* format, ...)
{
    if (m_pInterface != NULL) 
    {    
        va_list args;
        va_start(args, format);
        m_pInterface->WriteV(logLevel, facility, format, args);
        va_end(args);
    }
}

void CLog::Close()
{
    if (m_pInterface != NULL) 
        m_pInterface->Close();
    m_IsOpen = false;
}

void CLog::SetMask(int level)
{
    if (m_pInterface != NULL) 
        m_pInterface->SetMask(level);
}

int CLog::GetMask()
{
    return (m_pInterface != NULL) ? m_pInterface->GetMask() : 0;
}

bool CLog::IsOpen()
{
    return m_IsOpen;    
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Base implementation of ILog interface
CLogBase::CLogBase() :
    m_Mask(0)
{
    
}

CLogBase::~CLogBase()
{
    // TODO: Is this calling the method I think it is...?
    // TODO: Figure out how to properly call the inheriting class' method..
    // Close();)
}

void CLogBase::SetMask(int mask)
{
    m_Mask = mask;
}

int CLogBase::GetMask()
{
    return m_Mask;
}

bool CLogBase::Open()
{
    return true;
}

void CLogBase::Close()
{
    
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// File-based implementation of ILog interface

CFileLog::CFileLog(const char* path) : 
    m_pFile(NULL)
{
    if (path)
        m_Path = path;
    else
        m_Path = "";
}

CFileLog::~CFileLog()
{
    // TODO: Do this in the base class
    Close(); // Make sure we clean up
}

bool CFileLog::Open()
{
    if (m_pFile)
        return true; // Already open
    
    m_pFile = fopen(m_Path.c_str(), "a+"); // Open output file for reading and writing (at end of file)
    return (m_pFile != NULL);
}

void CFileLog::Write(int logLevel, const char* facility, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteV(logLevel, facility, format, args);
    va_end(args);
}

void CFileLog::WriteV(int logLevel, const char* facility, const char* format, va_list args)
{
    if (!m_pFile)
        return;
    
    fprintf(m_pFile, "%s::", facility);
    vfprintf(m_pFile, format, args);
    fprintf(m_pFile, "\n");
}

void CFileLog::Close()
{
    if (m_pFile)
        fclose(m_pFile);
    m_pFile = NULL;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Console-based implementation of ILog interface
CConsoleLog::CConsoleLog()
{

}

CConsoleLog::~CConsoleLog()
{

}

void CConsoleLog::Write(int logLevel, const char* facility, const char* format, ...)
{
    va_list args;
    va_start(args, format);
    WriteV(logLevel, facility, format, args);
    va_end(args);
}

void CConsoleLog::WriteV(int logLevel, const char* facility, const char* format, va_list args)
{
    printf("%s::", facility);
    vprintf(format, args);
    printf("\n");    
}