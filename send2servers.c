/* send2servers v1.0 by  james@ustc.edu.cn 2016.05.11
   ./send2servers a.a.a.a:port b.b.b.b:port c.c.c.c:port
	send all data readed from stdin to:
		a.a.a.a:port
		b.b.b.b:port
		c.c.c.c:port
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <linux/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include "sock.h"
#include <ctype.h>
#include <math.h>

// #define DEBUG 1

// max servers
#define MAXS 50

#define MAXLEN 16384
#define STRLEN 100

int total_server=0;
int server_fd[MAXS];
char server_port[MAXS][STRLEN];

void Process(int c_fd) 
{
        unsigned char buf[MAXLEN];
        int i,n;
        int optval;

	for(i=0; i<total_server; i++) {
		char *p;
		p = strchr(server_port[i],':');
		if(p==NULL) exit(0);	
		*p=0; p++;
		server_fd[i] =  Tcp_connect(server_port[i], p);
		int r_fd = server_fd[i];
        	socklen_t optlen = sizeof(optval);
        	optval = 120;
        	Setsockopt(r_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
        	optval = 10;
        	Setsockopt(r_fd, SOL_TCP, TCP_KEEPCNT, &optval, optlen);
        	optval = 120;
        	Setsockopt(r_fd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen);
        	optval = 10;
        	Setsockopt(r_fd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen);
	}
        while (1) {
                n = Read(c_fd, buf, MAXLEN);
                if(n<=0)   {
                        exit(0);
                }
		buf[n]=0;
#ifdef DEBUG
		dump_pkt(buf,n);
#endif
		for(i=0; i<total_server; i++) {
			Write(server_fd[i],buf,n);
		}
        }
}

void usage()
{
	printf("send2servers v1.0 by james@ustc.edu.cn\n");
	printf("%s",
	"./send2servers a.a.a.a:port [ b.b.b.b:port c.c.c.c:port ... ] \n"
	"read from stdin, send all data to\n"
	"	a.a.a.a:port\n"
	"	b.b.b.b:port\n"
	"	c.c.c.c:port\n");
	printf("max servers: %d\n",MAXS);
	exit(0);
}

int main(int argc, char *argv[])
{
	int listen_fd;
	int c_fd;
	int llen;
	char *port;
	int i;

	if((argc<2)|(argc-1>MAXS))
		usage();

	total_server = argc-1;
	printf("sending stdin to %d servers:\n",total_server);
	for(i=0;i<total_server;i++) {
		strncpy(server_port[i],argv[1+i],STRLEN);
		printf("    %s\n",server_port[i]);
	}
		
	Process(0);  // 0 is stdin
}
