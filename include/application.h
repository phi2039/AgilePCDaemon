#ifndef APPLICATION_H
#define APPLICATION_H

#include "datamanager.h"

#include <vector>
#include <string>

class CFSDirectory
{
public:
	CFSDirectory(const char* pPath);
	virtual ~CFSDirectory();
	std::vector<std::string> List(const char* pExtension = NULL);
protected:
	std::string m_Path;
private:
};

class CApplication
{
public:
	CApplication();
	virtual ~CApplication();
	int Run();
protected:
	CDataManager* m_pDataManager;
	bool m_QuitFlag;
private:

};

#endif // APPLICATION_H