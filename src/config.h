#ifndef CONFIG_H
#define	CONFIG_H

#include <string>
#include <map>

#include <sstream>

using namespace std;

class CConvert 
{
public:
    // Convert T, which should be a primitive, to a std::string
    template <typename T>
    static std::string T_to_string(T const &val) {
        ostringstream ostr;
        ostr << val;

        return ostr.str();
    }

    // Convert a std::string to type T (except for std::string)
    template <typename T>
    static T string_to_T(string const &val) 
    {
        std::istringstream istr(val);
        T returnVal;
        if (!(istr >> returnVal))
            return returnVal; // TODO: Deal with invalid values somehow?

        return returnVal;
    }

    static std::string string_to_T(string const &val) 
    {
        return val;
    }
};

//////////////////////////////////////////////////////////////////////////////////////////////////////
// Configuration interface

class IConfig 
{
public:

    virtual ~IConfig() {}
    virtual bool Load() = 0;
    virtual bool Save() = 0;
    virtual bool GetOpt(const string& name, string& value) = 0;
    virtual bool SetOpt(const string& name, string& value) = 0;
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
    static bool GetOpt(const string& name, string& value);
    static bool SetOpt(const string& name, string& value);
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
    virtual bool GetOpt(const string& name, string& value);
    virtual bool SetOpt(const string& name, string& value);
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
    CFileConfig(const string& path);
    virtual ~CFileConfig();
    virtual bool Load();
    virtual bool Save();
protected:
    void RemoveComment(string& line);
    bool OnlyWhitespace(const string& line);
    bool ValidLine(const string& line);
    void ExtractKey(string& key, size_t const &sepPos, const string& line);
    void ExtractValue(string& value, size_t const &sepPos, const string& line);
    bool ParseLine(const string& line, size_t lineNo);
    
private:
    string m_Path;
};

#endif	/* CONFIG_H */

