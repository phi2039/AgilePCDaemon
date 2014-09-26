// Library Includes
#include <sys/types.h>
#include <errno.h>
#include <vector>
#include <string.h>
#include <errno.h>
#include <dirent.h>

// TODO: Implement standard logging/error code (PrintLog, PrintError, etc...) with debugging support
#include "application.h"
#include "logging.h"

using namespace std;

int getdir(string dir, vector<string> &files)
{
    DIR *dp;
    struct dirent *dirp;
    if ((dp = opendir(dir.c_str())) == NULL)
    {
        APC_LOG(APC_LOG_FLAG_ERROR, "Error (%d) opening %s", errno, dir.c_str());
        return errno;
    }

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
    // TODO: Create and initialize SettingsManager

    // Create and initialize DataManager
    m_pDataManager = new CDataManager();

    if (!m_pDataManager->Initialize())
    {
        APC_LOG_0(APC_LOG_FLAG_ERROR, "Failed to initialize DataManager");
        return false;
    }
    APC_LOG_0(APC_LOG_FLAG_INFO, "DataManager Started...");

    // TEMP: Check that upload directory exists...
    if (access("/home/freedom_ftp/upload/", R_OK))
    {
        APC_LOG(APC_LOG_FLAG_ERROR, "Could not open upload directory. Error: %s", strerror(errno));
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

    //string loadPath = "ftp_dir/";
    string loadPath = "/home/freedom_ftp/upload/";
    string tempPath = "/tmp/";
    string archivePath = "/home/freedom_ftp/archive/";

    APC_LOG(APC_LOG_FLAG_INFO, "Monitoring upload directory: %s", loadPath.c_str());

    while (!m_QuitFlag)
    {
        vector<string> files = vector<string>();

        if (getdir(loadPath, files))
        {
            APC_LOG(APC_LOG_FLAG_ERROR, "Could not open upload directory: %s. Error: %s", loadPath.c_str(), strerror(errno));
            return 1;
        }

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
                    APC_LOG(APC_LOG_FLAG_ERROR, "Unable to move input file (%s). Error: %s", (loadPath + files[i].c_str()).c_str(), strerror(errno));
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
                    APC_LOG(APC_LOG_FLAG_ERROR, "Unable to archive input file (%s). Error: %s", files[i].c_str(), strerror(errno));
                }

                // TODO: Delete input file
                //err = remove((tempPath + files[i].c_str()).c_str());
                //if (err)
                //{
                //	fprintf(stderr, "Unable to delete input file (%s). Error: %s\r\n", (loadPath + files[i].c_str()).c_str(), strerror(errno));
                //}
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