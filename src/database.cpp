
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

// TODO: MySQL thread safety...

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
    if (m_pMySQL) // Already connected
        return true;
    
    m_pMySQL = mysql_init(NULL);
    if (!m_pMySQL)
        return false; // Allocation error (should never happen)

    if (pDatabase)
        m_Database = pDatabase;
    
    // Set Client Library Options
    // MYSQL_READ_DEFAULT_GROUP: Client name to display on server and in error logs
    // MYSQL_OPT_RECONNECT: Reconnect flag to retry statements on failure
    my_bool reconnect = true;
    if (SetMySQLOption(MYSQL_READ_DEFAULT_GROUP, "agilepcd") && 
        SetMySQLOption(MYSQL_OPT_RECONNECT, &reconnect)) 
    {
        unsigned long clientFlags = CLIENT_LOCAL_FILES // Permit "LOAD DATA LOCAL"
                | CLIENT_MULTI_STATEMENTS // ?? Include this?
                | CLIENT_REMEMBER_OPTIONS
                | CLIENT_MULTI_RESULTS; // For stored procedure calls

        // Reference: http://dev.mysql.com/doc/refman/5.6/en/mysql-real-connect.html
        // TODO: Is NULL pDatabase OK?
        if (mysql_real_connect(m_pMySQL, m_Host.c_str(), m_UserName.c_str(), m_Password.c_str(), pDatabase, m_Port, NULL, clientFlags))
        {
            printf("Connected to MySQL server (database: %s) on %s:%d as %s\r\n", m_Database.c_str(), m_Host.c_str(), m_Port, m_UserName.c_str());
            return true;
        }
    }

    printf("Failed to connect to MySQL server (database: %s) on %s:%d as. Error: %s\n", m_Database.c_str(), m_Host.c_str(), m_Port, m_UserName.c_str(), mysql_error(m_pMySQL));
    // TODO: Handle/return specific error conditions?
    mysql_close(m_pMySQL);
    m_pMySQL = NULL;
    return false;
}

void CDatabase::Disconnect()
{
    if (m_pMySQL)
    {
        mysql_close(m_pMySQL);
        m_pMySQL = NULL;
    }
    m_Database = "ANY";
}

// TODO: Error handling/return values
bool CDatabase::ExecSQL(const char* pSQLText)
{
    // TODO: Check connection status and handle as necessary...
    if (0 != mysql_query(m_pMySQL, pSQLText))
    {
        printf("Failed to execute SQL(%s): Error: %s\n", pSQLText, mysql_error(m_pMySQL));
        return false; // Error
        // TODO: Handle/return specific error conditions
    }
    return true;
}

bool CDatabase::SetMySQLOption(mysql_option opt, const void* val)
{
    if (0 != mysql_options(m_pMySQL, opt, val))
    {
        printf("Failed to set MySQL library option (%d). Error: %s\r\n", opt, mysql_error(m_pMySQL));
        return false;
    }
    return true;
}