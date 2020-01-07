
#include "sqlconnection.h"
#include <string.h>
#include "Logger.h"

//////////////////////////////////MySqlConnection/////////////////////////////

MysqlConnection::MysqlConnection() : mysql_(NULL)
{
    mysql_ = (MYSQL*)malloc(sizeof(MYSQL));
}

MysqlConnection::~MysqlConnection()
{
    if (mysql_ != NULL)
    {
        mysql_close(mysql_);
        free (mysql_);
        mysql_ = NULL;
    }
    return;
}

bool MysqlConnection::Init(const char* szHost, int nPort, const char* szUser, const char* szPasswd, const char* szDb)
{
	LOG_INFO("enter Init \n");
    if ((mysql_ = mysql_init(mysql_)) == NULL)
    {
        LOG_ERROR("init mysql failed %s, %d", this->GetErrInfo(), errno);
        return false;
    }
#if 0
	char cAuto = 1;
	if (!mysql_options(mysql_, MYSQL_OPT_RECONNECT, &cAuto))
	{
		LOG_INFO("mysql_options MYSQL_OPT_RECONNECT failed.");
	}

	unsigned int uTimeout = 3;
	if (!mysql_options(mysql_, MYSQL_OPT_CONNECT_TIMEOUT, &uTimeout))
	{
		TRACE("mysql_options MYSQL_OPT_CONNECT_TIMEOUT failed.");
	}
#endif

    if (mysql_real_connect(mysql_, szHost, szUser, szPasswd, szDb, nPort, NULL, 0) == NULL)
    {
        LOG_ERROR("connect mysql failed %s", this->GetErrInfo());
	   return false;
    }

    char cAuto = 1;
    if (!mysql_options(mysql_, MYSQL_OPT_RECONNECT, &cAuto))
    {
        LOG_INFO("mysql_options MYSQL_OPT_RECONNECT failed.");
    }

    return true;
}

bool MysqlConnection::Execute(const char* szSql)
{
    if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
    {
        if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)//代表断开连接
        {
            Reconnect();
	    }
        return false;
    }

    return true;
}

bool MysqlConnection::Execute(const char* szSql, SqlRecordSet& recordSet)
{
    if (mysql_real_query(mysql_, szSql, strlen(szSql)) != 0)
    {
        if (mysql_errno(mysql_) == CR_SERVER_GONE_ERROR)
        {
            Reconnect();
        }

        return false;
    }
    MYSQL_RES* pRes = mysql_store_result(mysql_);//返回执行的结果
    if (!pRes)
    {
        return NULL;
    }
    recordSet.SetResult(pRes);

    return true;
}

int MysqlConnection::EscapeString(const char* pSrc, int nSrcLen, char* pDest)
{
    if (!mysql_) return 0;
    return mysql_real_escape_string(mysql_, pDest, pSrc, nSrcLen);
}

int MysqlConnection::EscapeString(const std::string& strSrc, std::string& strDest)
{
    if (!mysql_) return 0;

    char szDest[2048] = {0};
    int nRet = mysql_real_escape_string(mysql_, szDest, strSrc.c_str(), strSrc.size());
    if (nRet <= 0)
    {
        return nRet;
    }
    strDest = std::string(szDest);
    return nRet;
}

void MysqlConnection::Close()
{
    mysql_close(mysql_);
    mysql_ = NULL;
    LOG_INFO("disconnect with mysql.");
}

const char* MysqlConnection::GetErrInfo()
{
    return mysql_error(mysql_);
}

void MysqlConnection::Reconnect()
{
    mysql_ping(mysql_);
}

bool MysqlConnection::Transaction(char** sqls, int size)
{
    int ret = 0;
    ret = mysql_real_query(mysql_,"SET AUTOCOMMIT =0;",(unsigned int)strlen("SET AUTOCOMMIT =0;"));
    if (ret != 0)
    {
        LOG_ERROR("disable autocommit failed.");
        return false;
    }

    ret = mysql_real_query(mysql_,"begin;",(unsigned int)strlen("begin;"));
    if (ret != 0)
    {
        LOG_ERROR("start transaction failed.");
        return false;
    }
 
    for (int i = 0; i < size; i++ )
    {
        ret = mysql_real_query(mysql_, sqls[i], (unsigned int)strlen(sqls[i]));
        if (ret != 0)
        {
            LOG_ERROR("execute sql=%s failed", sqls[i]);
            mysql_real_query(mysql_, "rollback;", (unsigned int)strlen("rollback;"));
            return false;
        }
    }

    ret = mysql_real_query(mysql_, "commit;", (unsigned int)strlen("commit;"));
    if (ret != 0)
    {
        LOG_ERROR("commit transaction failed.");
        return false;
    }

    return true;
}


//////////////////////////////////MySqlConnection END/////////////////////////////
