// Library Includes
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "application.h"
#include "logging.h"
#include "config.h"

using namespace std;

int getdir(string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL)
        return errno;

    while ((dirp = readdir(dp)) != NULL)
    {
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

bool CApplication::Initialize()
{
    // Initialize logging
    // TODO: Detect or get passed daemon state. If we have a console, use it...
//    CLog::SetInterface(new CFileLog("/var/log/agilepcd"));
    CLog::SetInterface(new CConsoleLog());
//    CLog::SetInterface(new CSysLog("agilepcd"));
    CLog::SetMask(APC_LOG_LEVEL_INFO);
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Log Opened");

    // Initialize the configuration manager
    string configFile = "";
    CConfig::SetInterface(new CFileConfig(configFile.c_str()));
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Loaded configuration from %s --- %s", configFile.c_str());
   
    // Create and initialize DataManager
    m_pDataManager = new CDataManager();

    if (!m_pDataManager->Initialize())
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Failed to initialize DataManager");
        return false;
    }
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "DataManager Started...");

    // TEMP: Check that upload directory exists...
    if (access("/home/freedom_ftp/upload/", R_OK))
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Could not open upload directory. Error: %s", strerror(errno));
        return false;
    }

    // TODO: Define IService interface
    // TODO: Create and initialize TagService

    // TODO: Create and initialize TcpService

    // TODO: Create and initialize FileMonitorService

    return true;
}

int CApplication::Run()
{
    // Main application loop

    string loadPath = "/home/freedom_ftp/upload/";
    string tempPath = "/tmp/";
    string archivePath = "/home/freedom_ftp/archive/";

    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Monitoring upload directory: %s", loadPath.c_str());
    
    while (!m_QuitFlag)
    {
        vector<string> files = vector<string>();

        if (getdir(loadPath, files))
        {
            CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Could not open upload directory: %s. Error: %s", loadPath.c_str(), strerror(errno));
            return 1;
        }

        // TODO: Add "ignore" list for files living in the import directory that can't be processed
        for (unsigned int i = 0; i < files.size(); i++)
        {
            if (files[i].c_str()[0] != '.')
            {
                int err;
                
                struct stat fs;
                memset(&fs, 0, sizeof(struct stat));
                if ((0 == stat((loadPath + files[i].c_str()).c_str(), &fs)) && (fs.st_size > 0)) // Skip zero-length files and try them again later (these are still being uploaded)
                {
                    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Importing %s (size: %d)", (loadPath + files[i].c_str()).c_str(), fs.st_size);
                    
                    // Move to temp directory
                    err = rename((loadPath + files[i].c_str()).c_str(), (tempPath + files[i].c_str()).c_str());
                    if (err)
                    {
                        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Unable to move input file (%s). Error: %s", (loadPath + files[i].c_str()).c_str(), strerror(errno));
                    }
                    else
                    {
                        // Import file
                        m_pDataManager->LoadDataFile((tempPath + files[i].c_str()).c_str());
                    }
                    // TEMP: Archive input file
                    err = rename((tempPath + files[i].c_str()).c_str(), (archivePath + files[i].c_str()).c_str());
                    if (err)
                    {
                        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Unable to archive input file (%s). Error: %s", files[i].c_str(), strerror(errno));
                    }

                    // TODO: Delete input file
                    //err = remove((tempPath + files[i].c_str()).c_str());
                    //if (err)
                    //{
                    //	CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Unable to delete input file (%s). Error: %s\r\n", (loadPath + files[i].c_str()).c_str(), strerror(errno));
                    //}
                }
            }
        }
    }

    // TODO: Clean up resources
    delete m_pDataManager;
    m_pDataManager = NULL;

    return 0; // No Error
}

void CApplication::Quit()
{
    m_QuitFlag = true;
}