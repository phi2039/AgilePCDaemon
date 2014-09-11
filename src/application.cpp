
// TODO: Implement standard logging/error code (PrintLog, PrintError, etc...) with debugging support

// Library Includes
#include <sys/types.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <errno.h>

// Local Includes
#ifdef _WIN32
#include "dirent.h"
#else
#include <dirent.h>

#endif

#include "application.h"
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

int getdir(string dir, vector<string> &files)
{
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(dir.c_str())) == NULL) {
		printf("Error (%d) opening %s\r\n",errno,dir.c_str());
		return errno;
	}

	while ((dirp = readdir(dp)) != NULL) {
		files.push_back(string(dirp->d_name));
	}
	closedir(dp);
	return 0;
}

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

	while (!m_QuitFlag)
	{
		string loadPath = "ftp_dir/";
		string tempPath = "/tmp/";

		vector<string> files = vector<string>();

		getdir(loadPath, files);

		// "/mnt/hgfs/AgilePCDaemon/AC_Test_2014_08_27_17_12_36_EDT.csv"
		for (unsigned int i = 0; i < files.size(); i++)
		{
			if (files[i].c_str()[0] != '.')
			{
				int err;
				// Move to temp directory
				err = rename((loadPath + files[i].c_str()).c_str(), (tempPath + files[i].c_str()).c_str());
				if (err)
				{
					fprintf(stderr, "Unable to move input file (%s). Error: %s\r\n", (loadPath + files[i].c_str()).c_str(), strerror(errno));
				}
//				else
				{
					// Import file
					m_pDataManager->LoadDataFile((tempPath + files[i].c_str()).c_str());
				}
				// Delete input file
				err = remove((tempPath + files[i].c_str()).c_str());
				if (err)
				{
					fprintf(stderr, "Unable to delete input file (%s). Error: %s\r\n", (loadPath + files[i].c_str()).c_str(), strerror(errno));
				}
			}
		}
#ifdef _WIN32
		Sleep(10000); // NOTE: This is Windows-only
#else
		sleep(10);
#endif
	}

	// TODO: Clean up resources
	delete m_pDataManager;
	m_pDataManager = NULL;

	return 0; // No Error
}
