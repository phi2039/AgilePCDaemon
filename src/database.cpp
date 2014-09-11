
// Library Includes
#include <stdio.h>

// Local Includes
#include "database.h"

CDatabase::CDatabase() :
	m_pMySQL(NULL),
    m_Host(""),
    m_Port(0),
    m_UserName(""),
    m_Password(""),
	m_Database("ANY")
{

}

CDatabase::~CDatabase()
{
	if (m_pMySQL)
		Disconnect();
}

bool CDatabase::Initialize(const char* pHost, const char* pUserName, const char* pPassword, unsigned int port /*=3306*/)
{
	if (!pUserName || !pPassword)
		return false; // Invalid/missing credentials (no default)

	m_Host = (pHost == NULL) ? "localhost" : pHost; // default to localhost
	m_UserName = pUserName;
	m_Password = pPassword;
	m_Port = port;

	return true;
}

bool CDatabase::Connect(const char* pDatabase /*=NULL*/)
{
	// TODO: Write a helper function to compare connection request to current connection...otherwise assume request is for the same resource
	if (m_pMySQL)
		return true;

	m_pMySQL = mysql_init(NULL);
	if (!m_pMySQL)
		return false; // Initialization Error

	unsigned long clientFlags = CLIENT_LOCAL_FILES // Permit "LOAD DATA LOCAL"
	| CLIENT_MULTI_STATEMENTS // ?? Include this?
	| CLIENT_REMEMBER_OPTIONS
	| CLIENT_MULTI_RESULTS; // For stored procedure calls

	// Reference: http://dev.mysql.com/doc/refman/5.6/en/mysql-real-connect.html
	// mysql_options(&mysql,MYSQL_READ_DEFAULT_GROUP,"agilepcd");
	// TODO: Set reconnect flag to retry statements on faliure
	// MYSQL_OPT_RECONNECT 
	// TODO: Just use NULL MYSQL?? (Late Binding)
	if (!mysql_real_connect(m_pMySQL, m_Host.c_str(), m_UserName.c_str(), m_Password.c_str(), pDatabase, m_Port, NULL, clientFlags))
	{
		printf("Failed to connect to database: Error: %s\n", mysql_error(m_pMySQL));
		mysql_close(m_pMySQL); // Clean up connection resource
		return false; // Connection error
		// TODO: Handle/return specific error conditions
	}
	if (pDatabase)
		m_Database = pDatabase;

	return true;
}

void CDatabase::Disconnect()
{
	mysql_close(m_pMySQL);
	m_pMySQL = NULL;
	m_Database = "ANY";
}

// TODO: Error handling/return values
bool CDatabase::ExecSQL(const char* pSQLText)
{
	if (0 != mysql_query(m_pMySQL, pSQLText))
	{
		printf("Failed to execute SQL(%s): Error: %s\n", pSQLText, mysql_error(m_pMySQL));
		return false; // Connection error
		// TODO: Handle/return specific error conditions
	}
	return true;
}
