#include <fstream>

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

bool CConfig::GetOpt(const string& name, string& value)
{
    return ((m_pInterface != NULL) ? m_pInterface->GetOpt(name, value) : false);
}

bool CConfig::SetOpt(const string& name, string& value)
{
    return ((m_pInterface != NULL) ? m_pInterface->SetOpt(name, value) : false);
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Base implementation of IConfig interface
CConfigBase::~CConfigBase()
{

}

// TODO: Implement some kind of "default value" store??
bool CConfigBase::GetOpt(const string& name, string& value)
{
    OptionIterator iter = m_Options.find(name);
    if (iter != m_Options.end())
    {
        value = iter->second;
        return true;
    }
    return false; // Option is not set
}

bool CConfigBase::SetOpt(const string& name, string& value)
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
    CLog::Write(APC_LOG_FLAG_INFO, "Config", "Option %s set to %s", name.c_str(), value.c_str());
    return true;
}

//////////////////////////////////////////////////////////////////////////////////////////////////////
// File-based implementation of IConfig interface
CFileConfig::CFileConfig(const char* path)
{
    m_Path = path;
}

CFileConfig::CFileConfig(const string& path)
{
    m_Path = path;
}

CFileConfig::~CFileConfig()
{

}
    
bool CFileConfig::Load()
{
    // Load configuration options from a file

    ifstream file;
    file.open(m_Path.c_str());
    if (!file)
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "Config", "Configuration file (%s) could not be opened.", m_Path.c_str());
        return false;
    }
    
    string line;
    size_t lineNo = 0;
    while (getline(file, line))
    {
        lineNo++;
        string temp = line;

        if (temp.empty())
            continue;

            RemoveComment(temp);
        if (OnlyWhitespace(temp))
            continue;

        ParseLine(temp, lineNo);
    }

    file.close();
    
    CLog::Write(APC_LOG_FLAG_INFO, "Config", "Parsed configuration file (%s)", m_Path.c_str());

    return true;
}

bool CFileConfig::Save()
{
    // Save configuration options to a file
    return false;
}

void CFileConfig::RemoveComment(string& line)
{
    if (line.find(';') != line.npos)
    line.erase(line.find(';'));
}

bool CFileConfig::OnlyWhitespace(const string& line)
{
    return (line.find_first_not_of(' ') == line.npos);
}

bool CFileConfig::ValidLine(const string& line)
{
    std::string temp = line;
    temp.erase(0, temp.find_first_not_of("\t "));
    if (temp[0] == '=')
            return false;

    for (size_t i = temp.find('=') + 1; i < temp.length(); i++)
            if (temp[i] != ' ')
                    return true;

    return false;
}

void CFileConfig::ExtractKey(string& key, size_t sepPos, const string& line)
{
    key = line.substr(0, sepPos);
    if (key.find('\t') != line.npos || key.find(' ') != line.npos)
          key.erase(key.find_first_of("\t "));
}

void CFileConfig::ExtractValue(string& value, size_t sepPos, const string& line)
{
    value = line.substr(sepPos + 1);
    value.erase(0, value.find_first_not_of("\t "));
    value.erase(value.find_last_not_of("\t ") + 1);
}

void CFileConfig::ParseLine(const string& line, size_t lineNo)
{
    if (line.find('=') == line.npos)
        CLog::Write(APC_LOG_FLAG_ERROR, "Config", "Couldn't find separator on line: %d", lineNo);

    if (!ValidLine(line))
        CLog::Write(APC_LOG_FLAG_ERROR, "Config", "CFG: Bad format for line: %d", lineNo);
    
    std::string temp = line;
    // Erase leading whitespace from the line.
    temp.erase(0, temp.find_first_not_of("\t "));
    size_t sepPos = temp.find('=');

    std::string key, value;
    ExtractKey(key, sepPos, temp);
    ExtractValue(value, sepPos, temp);

    SetOpt(key.c_str(), value);
}