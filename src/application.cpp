// Library Includes
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h>
#include <errno.h>
#include <dirent.h>

#include "config.h"
#include "application.h"
#include "logging.h"

using namespace std;

CUploadMonitor::CUploadMonitor(const string& loadPath, const string& archivePath, const string& tempPath, CDataManager* pDataManager, bool archiveFlag /*=true*/) :
    m_LoadPath(loadPath),
    m_ArchivePath(archivePath),
    m_TempPath(tempPath),
    m_pDataManager(pDataManager),
    m_ArchiveFlag(archiveFlag)
{

}

CUploadMonitor::~CUploadMonitor()
{
    
}

// TODO: Find and delete old archive files??

bool CUploadMonitor::CheckUploads()
{
    vector<string> files = vector<string>();

    if (GetDirectory(m_LoadPath, files))
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Could not open upload directory: %s. Error: %s", m_LoadPath.c_str(), strerror(errno));
        return false;
    }

    // TODO: Add "ignore" list for files living in the import directory that can't be processed
    for (unsigned int i = 0; i < files.size(); i++)
    {
        if (files[i].c_str()[0] != '.')
        {
            int err;

            struct stat fs;
            memset(&fs, 0, sizeof(struct stat));
            if ((0 == stat((m_LoadPath + files[i].c_str()).c_str(), &fs)) && (fs.st_size > 0)) // Skip zero-length files and try them again later (these are still being uploaded)
            {
                CLog::Write(APC_LOG_FLAG_INFO, "Application", "Importing %s (size: %d)", (m_LoadPath + files[i].c_str()).c_str(), fs.st_size);

                // Move to temp directory
                err = rename((m_LoadPath + files[i].c_str()).c_str(), (m_TempPath + files[i].c_str()).c_str());
                if (err)
                {
                    CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Unable to move input file (%s). Error: %s", (m_LoadPath + files[i].c_str()).c_str(), strerror(errno));
                }
                else // Import file
                {
                    m_pDataManager->LoadDataFile((m_TempPath + files[i].c_str()).c_str());
                }

                if (m_ArchiveFlag) // Archive or delete input files?
                {
                    err = rename((m_TempPath + files[i].c_str()).c_str(), (m_ArchivePath + files[i].c_str()).c_str());
                    if (err)
                    {
                        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Unable to archive input file (%s). Error: %s", files[i].c_str(), strerror(errno));
                    }
                }
                else
                {
                    err = remove((m_TempPath + files[i].c_str()).c_str());
                    if (err)
                    {
                        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Unable to delete input file (%s). Error: %s", (m_LoadPath + files[i].c_str()).c_str(), strerror(errno));
                    }                    
                }
            }
        }
    }
    return true;
}

int CUploadMonitor::GetDirectory(string dir, vector<string> &files)
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Main Application Class
CApplication::CApplication() :
    m_pDataManager(NULL),
    m_pUploads(NULL),
    m_QuitFlag(false),
    m_pSocketSvc(NULL)
{

}

CApplication::~CApplication()
{
    // Clean-up DataManager (should have done this already)
    if (m_pDataManager)
        delete m_pDataManager;
    
    // Clean-up UploadManager (should have done this already)
    if (m_pUploads)
        delete m_pUploads;
}

bool CApplication::Initialize(bool daemon)
{
    // TODO: Pass command-line arguments (config overrides)
    // - Logging type
    // - Log level
    // - Configuration file
    
    // Initialize logging
//    CLog::SetInterface(new CFileLog("/var/log/agilepcd"));
//    CLog::SetInterface(new CSysLog("agilepcd"));
    CLog::SetInterface(new CConsoleLog());
    CLog::SetMask(APC_LOG_LEVEL_INFO);
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Log Opened");

    // Initialize the configuration store
    string configFile = "/etc/agilepc/agilepc.cnf";
    CConfig::SetInterface(new CFileConfig(configFile.c_str()));
    
    // Check for new log-level from configuration file
   
    // Create and initialize DataManager
    m_pDataManager = new CDataManager();

    if (!m_pDataManager->Initialize())
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Failed to initialize DataManager");
        return false;
    }
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "DataManager Initialized");

    
// TEMP: Initialize upload manager
    // TODO: Use more appropriate default values
    string loadPath = "/home/freedom_ftp/upload/";
    string tempPath = "/tmp/";
    string archivePath = "/home/freedom_ftp/archive/";

    CConfig::GetOpt("app_load_path", loadPath);
    CConfig::GetOpt("app_archive_path", archivePath);
    CConfig::GetOpt("app_temp_path", tempPath);
    // TODO: Read archive flag
    if (access(loadPath.c_str(), R_OK))
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Could not access upload directory (%s). Error: %s", loadPath.c_str(), strerror(errno));
        return false;
    }
    if (access(archivePath.c_str(), R_OK))
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Could not access archive directory (%s). Error: %s", archivePath.c_str(), strerror(errno));
        return false;
    }
    if (access(tempPath.c_str(), R_OK))
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "Application", "Could not access temp directory (%s). Error: %s", tempPath.c_str(), strerror(errno));
        return false;
    }    
    m_pUploads = new CUploadMonitor(loadPath, archivePath, tempPath, m_pDataManager);
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Upload Monitor Initialized. Monitoring upload directory: %s", loadPath.c_str());
// ~TEMP
   
    // Initialize command listener
    m_pSocketSvc = new CSocketCommandService();
    m_pSocketSvc->Initialize();
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Socket Control Service Initialized");
    
    return true;
}

void CApplication::ReloadConfig()
{
    // TODO: Reload config (and reinit logging?)
}

// TODO: Implement simple path-handling class...
int CApplication::Run()
{
    // Start Services
    // TODO: Run DM in thread?
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "DataManager Starting...");
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "DataManager Started");

// TEMP
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Upload Monitor Starting...");
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Upload Monitor Started");
 // ~TEMP 
    
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Socket Command Service Starting...");
    m_pSocketSvc->Start();
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Socket Command Service Started");
    
    // Main application loop
    while (!m_QuitFlag)
    {
        // TEMP: Check for uploaded data files
        // TODO: Put this in a service/thread
        if (!m_pUploads->CheckUploads())
            return 1; // This is all we do right now...no point hanging around...
        sleep(1); // Only poll once a second or so...
    }
    
     // Clean up services/resources
    Close();
    
   CLog::Write(APC_LOG_FLAG_INFO, "Application", "Exiting...");

    return 0; // No Error
}

void CApplication::Close()
{
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Shutting down...");
    if (m_pDataManager)
    {
        delete m_pDataManager;
        m_pDataManager = NULL;
        CLog::Write(APC_LOG_FLAG_INFO, "Application", "DataManager Stopped");
    }
    
    if (m_pUploads)
    {
        delete m_pUploads;
        m_pUploads = NULL;
        CLog::Write(APC_LOG_FLAG_INFO, "Application", "Upload Monitor Stopped");
    }

    if (m_pSocketSvc)
    {
        m_pSocketSvc->Stop();
        delete m_pSocketSvc;
        m_pSocketSvc = NULL;
        CLog::Write(APC_LOG_FLAG_INFO, "Application", "Socket Command Service Stopped");
    }    
   
    // Last but not least
    CLog::Write(APC_LOG_FLAG_INFO, "Application", "Closing Log...");
    CLog::Close();
}

void CApplication::Quit()
{
    m_QuitFlag = true;
}
