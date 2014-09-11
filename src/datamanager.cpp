#include <fstream>
#include <iostream>
#include <sstream>
#include <locale>
#include <math.h>
#include <string.h>
#include <time.h>

#include "datamanager.h"

// TODO: Pass IConfigManager interface
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

	// TODO: Read configuration values from config file
	if (!m_pDB->Initialize("127.0.0.1", "agile", "agile"))
//	if (!m_pDB->Initialize("192.168.128.10", "agile", "agile"))
		return false;

	// TODO: Dynamic connection management? Maybe the driver already does this...
	
	if (!m_pDB->Connect())
		return false;

	// TODO: Cleanup any leftovers from the last instance...

	return true;
}

// TODO: Rationalize return values
int CDataManager::LoadDataFile(const char* pFileName)
{
	std::string sqlText;

	char tempTable[] = "csv_import_temp";
	// Create temp table (if it already exists, truncate it...)
	// TODO: TRUNCATE temp table if it already exists
	// TODO: Use dynamic schema name
	sqlText = "CREATE TEMPORARY TABLE `agilepc`.`";
	sqlText.append(tempTable);
	sqlText.append("` (`dummy1` int(11) DEFAULT NULL, `dummy2` int(11) DEFAULT NULL, `ts` datetime NOT NULL, `val` double NOT NULL, PRIMARY KEY(`ts`)) ENGINE = InnoDB DEFAULT CHARSET = latin1");
	if (!m_pDB->ExecSQL(sqlText.c_str()))
		return -1;

	// TODO: Use temp table and validate...currently pushing directly into data table
	// Import contents to temp table
	int rowCount = ImportCSV_Internal(pFileName, tempTable);
	if (rowCount < 0)
		fprintf(stderr, "Failed to import records from CSV file (%s) into table (%s). Error: %d\r\n", pFileName, tempTable, rowCount);
	else
	{
		// TODO: Transfer data to data table
	}

	// Drop temp table
	sqlText = "DROP TABLE `agilepc`.`";
	sqlText.append(tempTable);
	sqlText.append("`");
	m_pDB->ExecSQL(sqlText.c_str());

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
	char fields[4][32];
	std::sscanf(pText, "%[^','],%[^','],%[^','],%s", fields[0], fields[1], fields[2], fields[3]);

	// Fields:
	//	0 - dummy
	//	1 - dummy
	//	2 - timestamp
	//	3 - value

	// Parse the measurement timestamp
	// 2014-08-26 17:15:00
	point.meas_time.tm_year = atoi(strtok(fields[2], "-")) - 1900; // Year
	point.meas_time.tm_mon = atoi(strtok(NULL, "-")); // Month
	point.meas_time.tm_mday = atoi(strtok(NULL, " ")); // Day
	point.meas_time.tm_hour = atoi(strtok(NULL, ":")); // Hour
	point.meas_time.tm_min = atoi(strtok(NULL, ":")); // Minute
	point.meas_time.tm_sec = atoi(strtok(NULL, "\0")); // Second

	// Parse the measurement value
	point.meas_val = atof(fields[3]);

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
		fprintf(stderr, "Failed to open input file (%s)\r\n", pFileName);
		return -1;
	}

	// Read each line from the file and parse the entry
	int recordCount = 0;
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
		m_pDB->ExecSQL(sqlText.c_str());

		//printf("%s\r\n", entry.meas_time.tm_year + 1900, values);
		recordCount++;
	}

	printf("Read %d lines from input file (%s)\r\n", recordCount, pFileName);

	// Close input file
	inFile.close();

	return recordCount;
}
