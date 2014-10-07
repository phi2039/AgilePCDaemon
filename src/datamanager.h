#ifndef DATAMANAGER_H
#define DATAMANAGER_H

#include "database.h"

class CDataManager
{
public:
    CDataManager();
    virtual ~CDataManager();
    bool Initialize();
    int LoadDataFile(const char* pFileName);
protected:
    int ImportCSV_Internal(const char* pFileName, const char* pTableName);
    CDatabase* m_pDB;
private:
};
#endif // DATAMANAGER_H