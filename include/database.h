#ifndef DATABASE_H
#define DATABASE_H

#include <string>

#include <my_global.h>
#include <my_sys.h>

#include <mysql.h>

class CDatabase
{
    public:
		CDatabase();
		virtual ~CDatabase();

		bool Initialize(const char* pHost, const char* pUserName, const char* pPassword, unsigned int port = 3306);
		bool Connect(const char* pDatabase = NULL);
        void Disconnect();

		int ImportCSV(const char* pFileName, const char* pTableName);
		bool ExecSQL(const char* pSQLText);
    protected:
    private:
		MYSQL* m_pMySQL;
        std::string m_Host;
        unsigned int m_Port;
        std::string m_UserName;
        std::string m_Password;
		std::string m_Database;
};

#endif // DATABASE_H
