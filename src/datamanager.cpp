#include <fstream>
#include <iostream>
#include <sstream>
#include <locale>
#include <math.h>
#include <string.h>
#include <time.h>

#include "datamanager.h"
#include "logging.h"
#include "config.h"

using namespace std;

CDataManager::CDataManager() :
    m_pDB(NULL)
{

}

CDataManager::~CDataManager()
{
    if (m_pDB)
    {
        m_pDB->Disconnect();
        delete m_pDB;
    }
}

bool CDataManager::Initialize()
{
    m_pDB = new CDatabase();

    string host = "127.0.0.1";
    string user = "agile";
    string password = "agile";
    
    // Read configuration values from configuration manager
    // CConfig will not modify values (keep default) if no value is found
    CConfig::GetOpt("dm_hostname", host);
    CConfig::GetOpt("dm_user", user);
    CConfig::GetOpt("dm_password", password);   

    if (!m_pDB->Initialize(host.c_str(), user.c_str(), password.c_str()))
    {
        return false;
    }
    
    // TODO: Dynamic connection management? Maybe the driver already does this...

    if (!m_pDB->Connect())
    {
        return false;
    }
    
    // TODO: Cleanup any leftovers from the last instance...

    return true;
}

// TODO: Rationalize return values

int CDataManager::LoadDataFile(const char* pFileName)
{
    std::string sqlText;

    char tempTable[] = "csv_import_temp";
    //	// Create temp table (if it already exists, truncate it...)
    //	// TODO: TRUNCATE temp table if it already exists
    //	// TODO: Use dynamic schema name
    //	sqlText = "CREATE TEMPORARY TABLE `agilepc`.`";
    //	sqlText.append(tempTable);
    //	sqlText.append("` (`dummy1` int(11) DEFAULT NULL, `dummy2` int(11) DEFAULT NULL, `ts` datetime NOT NULL, `val` double NOT NULL, PRIMARY KEY(`ts`)) ENGINE = InnoDB DEFAULT CHARSET = latin1");
    //	if (!m_pDB->ExecSQL(sqlText.c_str()))
    //		return -1;

    // TODO: Use temp table and validate...currently pushing directly into data table
    // Import contents to temp table
    int rowCount = ImportCSV_Internal(pFileName, tempTable);
    if (rowCount < 0)
        CLog::Write(APC_LOG_FLAG_ERROR, "DataManager", "Failed to import records from CSV file (%s) into table (%s). Error: %d", pFileName, tempTable, rowCount);
    else
    {
        // TODO: Transfer data to data table
    }

    //	// Drop temp table
    //	sqlText = "DROP TABLE `agilepc`.`";
    //	sqlText.append(tempTable);
    //	sqlText.append("`");
    //	m_pDB->ExecSQL(sqlText.c_str());

    return rowCount;
}

// ** TEMP**
// Parse the data file manually and INSERT each record

struct data_point
{
    std::tm meas_time;
    float meas_val;
};

bool ParseDataPoint(const char* pText, data_point& point)
{
    // Parse the line
    // 276,09/01/14 16:10:00,8.890,0.006,4.54
    // Line#,Date,"Current (S-FS-TRMSA 10564073:10518723-1), A, Jon's AC","Current (91-U30-CVIA 10564073:10549926-2), mA","Battery (U30 BATTERY 10564073:10564073-B), V"
    char fields[5][48];
    std::sscanf(pText, "%[^','],%[^','],%[^','],%[^','],%s", fields[0], fields[1], fields[2], fields[3]);

    // Fields:
    //	0 - line number (ignore)
    //	1 - timestamp
    //	2 - current (sensor)
    //	3 - current (battery) (ignore)
    //  4 - voltage (battery) (ignore)

    // Parse the measurement timestamp
    // Field 1: 09/06/14 17:15:00
    point.meas_time.tm_mon = atoi(strtok(fields[1], "/")); // Month
    point.meas_time.tm_mday = atoi(strtok(NULL, "/")); // Day
    point.meas_time.tm_year = atoi(strtok(NULL, " ")) + 100; // Year
    point.meas_time.tm_hour = atoi(strtok(NULL, ":")); // Hour
    point.meas_time.tm_min = atoi(strtok(NULL, ":")); // Minute
    point.meas_time.tm_sec = atoi(strtok(NULL, "\0")); // Second

    // Parse the measurement value
    point.meas_val = atof(fields[2]);

    return true;
}

// TODO: If we're going to use individual INSERT statements, use a Prepared Statement and batch the execution

int CDataManager::ImportCSV_Internal(const char* pFileName, const char* pTableName)
{
    std::fstream inFile;
    std::string inLine;

    // Open input file
    inFile.open(pFileName, std::fstream::in);
    if (!inFile.is_open())
    {
        CLog::Write(APC_LOG_FLAG_ERROR, "DataManager", "Failed to open input file (%s)", pFileName);
        return -1;
    }

    // Skip the first line of the file
    std::getline(inFile, inLine);

    // Read each line from the file and parse the entry
    unsigned int recordCount = 0;
    unsigned int errorCount = 0;
    for (std::getline(inFile, inLine); inLine.length() > 0; std::getline(inFile, inLine))
    {
        data_point entry;
        ParseDataPoint(inLine.c_str(), entry);

        // Insert the record
        char values[32];
        sprintf(values, "'1','1', '%02d-%02d-%02d %02d:%02d:%02d', '%f'", entry.meas_time.tm_year + 1900, entry.meas_time.tm_mon, entry.meas_time.tm_mday, entry.meas_time.tm_hour, entry.meas_time.tm_min, entry.meas_time.tm_sec, entry.meas_val);
        std::string sqlText = "INSERT INTO `agilepc`.`data_point` (`measurement`,`device`, `measurement_time`, `measured_value`) VALUES (";
        sqlText.append(values);
        sqlText.append(")");
        if (!m_pDB->ExecSQL(sqlText.c_str()))
        {
            // TODO: Do something with this record??
            errorCount++;
        }

        CLog::Write(APC_LOG_FLAG_DEBUG, "DataManager", "%s", values);
        recordCount++;
    }

    char timeString[32];
    time_t now_t;
    time(&now_t);
    tm *now;
    now = localtime(&now_t);
    strftime(timeString, sizeof (timeString), "%x %X", now);
    CLog::Write(APC_LOG_FLAG_INFO, "DataManager", "%s: Read %d lines from input file (%s) - %d errors", timeString, recordCount, pFileName, errorCount);

    // Close input file
    inFile.close();

    return recordCount;
}
