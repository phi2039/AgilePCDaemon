#include "application.h"

// TODO: Implement standard logging/error code (PrintLog, PrintError, etc...) with debugging support

using namespace std;

//CFSDirectory::CFSDirectory(const char* pPath)
//{
//	if (pPath)
//		m_Path = pPath;
//}
//
//CFSDirectory::~CFSDirectory()
//{
//
//}
//
//std::vector<std::string> CFSDirectory::List(const char* pExtension /*=NULL*/)
//{
//	vector<string> result;
//	//string lcExtension(strToLower(extension));
//
//	//DIR *dir;
//	//struct dirent *ent;
//
//	//if ((dir = opendir(directoryLocation.c_str())) == NULL) {
//	//	throw std::exception("readDirectory() - Unable to open directory.");
//	//}
//
//	//while ((ent = readdir(dir)) != NULL)
//	//{
//	//	string entry(ent->d_name);
//	//	string lcEntry(strToLower(entry));
//
//	//	// Check extension matches (case insensitive)
//	//	size_t pos = lcEntry.rfind(lcExtension);
//	//	if (pos != string::npos && pos == lcEntry.length() - lcExtension.length()) {
//	//		result.push_back(entry);
//	//	}
//	//}
//
//	//if (closedir(dir) != 0) {
//	//	throw std::exception("readDirectory() - Unable to close directory.");
//	//}
//
//	return result;
//}
//



CApplication::CApplication() :
	m_pDataManager(NULL),
	m_QuitFlag(false)
{

}

CApplication::~CApplication()
{
	// Clean-up DataManager (should have done this already)
	if (m_pDataManager)
		delete m_pDataManager;
}

// TODO: Rationalize return values
int CApplication::Run()
{
	// TODO: Create and initialize SettingsManager
	
	// Create and initialize DataManager
	m_pDataManager = new CDataManager();

	if (!m_pDataManager->Initialize())
	{
		fprintf(stderr, "Failed to initialize DataManager\r\n");
		return 1;
	}

	// TODO: Define IService interface
	// TODO: Create and initialize TagService

	// TODO: Create and initialize TcpService

	// TODO: Create and initialize FileMonitorService

	// Main application loop

	//while (!m_QuitFlag)
	//{
	// TODO: Read directory contents

	// TODO: For each file, move to tmp directory; load into db; delete;
//		m_pDataManager->LoadDataFile("C:/Users/CLANCE/Downloads/AC_Test_2014_08_27_17_12_36_EDT.csv");
	m_pDataManager->LoadDataFile("/mnt/hgfs/AgilePCDaemon/AC_Test_2014_08_27_17_12_36_EDT.csv");
	//	Sleep(10000); // NOTE: This is Windows-only
	//}

	// TODO: Clean up resources
	delete m_pDataManager;
	m_pDataManager = NULL;

	return 0; // No Error
}
