#include "config.h"
#include "logging.h"

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Global configuration facility (wrapper around generic IConfig implementation)
IConfig* CConfig::m_pInterface = NULL;
void CConfig::SetInterface(IConfig* pConfig)
{
    if (m_pInterface)
        delete m_pInterface;
    
    m_pInterface = pConfig;
    if (pConfig)
        pConfig->Load();
}

bool CConfig::Load()
{
    return ((m_pInterface != NULL) ? m_pInterface->Load() : false);
}

bool CConfig::Save()
{
    return ((m_pInterface != NULL) ? m_pInterface->Save() : false);
}

bool CConfig::GetOpt(const char* name, string& value)
{
    return ((m_pInterface != NULL) ? m_pInterface->GetOpt(name, value) : false);
}

bool CConfig::SetOpt(const char* name, string& value)
{
    return ((m_pInterface != NULL) ? m_pInterface->SetOpt(name, value) : false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Base implementation of IConfig interface
CConfigBase::~CConfigBase()
{

}

bool CConfigBase::GetOpt(const char* name, string& value)
{
    OptionIterator iter = m_Options.find(name);
    if (iter != m_Options.end())
    {
        value = iter->second;
        return true;
    }
    return false;
}

bool CConfigBase::SetOpt(const char* name, string& value)
{
    OptionIterator iter = m_Options.find(name);
    if (iter != m_Options.end())
    {
        iter->second = value;
    }
    else
    {
        m_Options.insert(make_pair<string,string>(name, value));
    }
    return false;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// File-based implementation of IConfig interface
CFileConfig::CFileConfig(const char* path)
{
    m_Path = path;
}

CFileConfig::~CFileConfig()
{

}
    
bool CFileConfig::Load()
{
    // Load configuration options from a file
    return false;
}

bool CFileConfig::Save()
{
    // Save configuration options to a file
    return false;
}
