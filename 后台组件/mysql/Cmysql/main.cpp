
#include "Logger.h"
#include "sqlconnection.h"
#include "glo_def.h"


int main(int argc, char** argv)
{
    if(!Logger::instance()->init(std::string(argv[1])))
    {
        printf("init log module failed.\n");
        return -2;
    }
    else
    {
        printf("init log module success!\n");
    }

    const char* pPlayerInfoTable = "\
                                 CREATE TABLE IF NOT EXISTS playerinfo( \
				 id		int(16)		NOT NULL primary key auto_increment, \
				 userid		int(16)		NOT NULL default 0, \
				 areaid		int(4)		NOT NULL default 0, \
				 ptname		varchar(48)     NOT NULL default '', \
				 rolename	varchar(48)     NOT NULL default '', \
				 dataversion    int(16)	        NOT NULL default 0, \
				 level		int(16)		NOT NULL default 0, \
				 exp		int(16)		NOT NULL default 0, \
				 gold		int(16)		NOT NULL default 0,	\
				 diamond	int(16)		NOT NULL default 0,	\
				 ap		int(16)		NOT NULL default 0, \
				 playerbase     blob,			\
				 herolist	blob,			\
				 gatelist       blob,			\
				 playerbag	blob,			\
				 playerlineup   blob,			\
				 extrainfo      blob,			\
				 actorlist	blob,			\
				 INDEX rolename_index (rolename),       \
				 INDEX ptname_index (ptname),           \
				 INDEX userid_index(userid)             \
				 )";


    MysqlConnection* mysqlconn = new MysqlConnection;
    if(!mysqlconn->Init("127.0.0.1", 3306, "root", "123", "game"))
    {
        LOG_ERROR("init mysql connection handler failed.");
    }
    
    /* 干我们该干的活了 */
    if (!mysqlconn->Execute(pPlayerInfoTable))
    {
        LOG_ERROR("create pPlayerInfo table failed");
    }

    /* 增删改查 */
    
    return 0;
}


