#ifndef APPLICATION_H
#define APPLICATION_H

#include <string>
#include <vector>

#include "datamanager.h"
#include "SocketCommandService.h"

using namespace std;

// TEMP: File monitor for uploaded files
// NOTE: Not thread safe!!
class CUploadMonitor
{
public:
    CUploadMonitor(const string& loadPath, const string& archivePath, const string& tempsPath, CDataManager* pDataManager, bool archiveFlag = true);
    virtual ~CUploadMonitor();
    bool CheckUploads();
private:
    int GetDirectory(string dir, vector<string> &files);
    string m_LoadPath;
    string m_ArchivePath;
    string m_TempPath;
    CDataManager* m_pDataManager;
    bool m_ArchiveFlag;
};
// ~TEMP

class CApplication 
{
public:
    CApplication();
    virtual ~CApplication();
    bool Initialize(bool daemon);
    void ReloadConfig();
    int Run();
    void Quit();
protected:
    void Close();
    CDataManager* m_pDataManager;
    bool m_QuitFlag;
    // TEMP:
    CUploadMonitor* m_pUploads;
    
    CSocketCommandService* m_pSocketSvc;
private:

};

#endif // APPLICATION_H