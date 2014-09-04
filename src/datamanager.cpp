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
//	if (!m_DB.Initialize("127.0.0.1", "agile", "agile"))
	if (!m_pDB->Initialize("192.168.128.10", "agile", "agile"))
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

	// Import contents to temp table
	int rowCount = m_pDB->ImportCSV(pFileName, tempTable);
	if (rowCount < 0)
		fprintf(stderr, "Failed to import records from CSV file (%s) into table (%s). Error: %d", pFileName, tempTable, rowCount);
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
