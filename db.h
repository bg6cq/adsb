#define DBUSER "root"
#define DBPASSWD ""
#define DB     "adsb"
#define DBHOST "localhost"
#define DBPORT 3306
#define DBSOCKPATH "/var/lib/mysql/mysql.sock"

#include <mysql/mysql.h>
MYSQL *mysql;

void err_quit(const char*s)
{
	fprintf(stderr,s);
	exit(0);
}

MYSQL * connectdb(void)
{
        MYSQL *mysql;

        if ((mysql=mysql_init(NULL))==NULL) 
                err_quit("mysql_init error\n");
        if( mysql_real_connect(mysql, DBHOST, DBUSER, DBPASSWD,
        DB, DBPORT, DBSOCKPATH, 0)== NULL)
                err_quit("mysql_init error\n");
        return mysql;
}
