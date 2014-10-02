#ifndef CONFIG_H
#define	CONFIG_H

#include <string>
#include <map>

using namespace std;

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration interface
class IConfig
{
public:
    virtual ~IConfig() {}
    virtual bool Load() = 0;
    virtual bool Save() = 0;
    virtual bool GetOpt(const char* name, string& value) = 0;
    virtual bool SetOpt(const char* name, string& value) = 0;
protected:
private:
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Global configuration facility (wrapper around generic IConfig implementation)
class CConfig
{
public:
    static void SetInterface(IConfig* pConfig);
    static bool Load();
    static bool Save();
    static bool GetOpt(const char* name, string& value);
    static bool SetOpt(const char* name, string& value);
protected:
private:
    CConfig(); // Singleton
    static IConfig* m_pInterface;
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Base implementation of IConfig interface
typedef map<string, string> OptionMap;
typedef OptionMap::iterator OptionIterator;
typedef pair<string, string> OptionPair;

class CConfigBase : public IConfig
{
public:
    virtual ~CConfigBase();
    virtual bool GetOpt(const char* name, string& value);
    virtual bool SetOpt(const char* name, string& value);
protected:
    OptionMap m_Options;
private:    
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// File-based implementation of IConfig interface
class CFileConfig : public CConfigBase
{
public:
    CFileConfig(const char* path);
    virtual ~CFileConfig();
    virtual bool Load();
    virtual bool Save();    
protected:
private:
    string m_Path;
};

#endif	/* CONFIG_H */

