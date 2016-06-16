#define DBUSER "root"
#define DBPASSWD ""
#define DB     "adsb"
#define DBHOST "localhost"
#define DBPORT 3306
#define DBSOCKPATH "/var/lib/mysql/mysql.sock"

#include <mysql/mysql.h>
MYSQL *mysql;

MYSQL * connectdb(void)
{
        MYSQL *mysql;
	int read_time_out = 5;
        if ((mysql=mysql_init(NULL))==NULL) 
                err_quit("mysql_init error\n");
	mysql_options(mysql, MYSQL_OPT_READ_TIMEOUT, &read_time_out);
        if( mysql_real_connect(mysql, DBHOST, DBUSER, DBPASSWD,
        DB, DBPORT, DBSOCKPATH, 0)== NULL)
                err_quit("mysql_init error\n");
        return mysql;
}
