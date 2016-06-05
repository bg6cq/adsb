/* send2servers v1.0 by  james@ustc.edu.cn 2016.05.11
   ./send2servers xxxx a.a.a.a:port b.b.b.b:port c.c.c.c:port
   accept connection on tcp port xxxx
	send all data received to
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
        socklen_t optlen = sizeof(optval);
        optval = 120;
        Setsockopt(c_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
        optval = 10;
        Setsockopt(c_fd, SOL_TCP, TCP_KEEPCNT, &optval, optlen);
        optval = 120;
        Setsockopt(c_fd, SOL_TCP, TCP_KEEPIDLE, &optval, optlen);
        optval = 10;
        Setsockopt(c_fd, SOL_TCP, TCP_KEEPINTVL, &optval, optlen);

	for(i=0;i<total_server;i++) {
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
		for(i=0;i<total_server;i++) {
			Write(server_fd[i],buf,n);
		}
        }
}

void usage()
{
	printf("\nsend2servers v1.0 by james@ustc.edu.cn\n");
	printf("%s",
   	"./send2servers xxxx a.a.a.a:port b.b.b.b:port c.c.c.c:port"
   	"accept connection on tcp port xxxx"
   	"send all data received"
	"	a.a.a.a:port"
	"	b.b.b.b:port"
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

	if((argc<3)|(argc-2>MAXS)) 
		usage();

	port = argv[1];
	printf("accepting on port %s\n", port);
	total_server = argc-2;
	printf("sending to %d servers\n",total_server);
	for(i=0;i<total_server;i++) {
		strncpy(server_port[i],argv[2+i],STRLEN);
		printf("    %s\n",server_port[i]);
	}
		
	signal(SIGCHLD,SIG_IGN);

#ifndef DEBUG
	daemon_init("send2servers",LOG_DAEMON);
#endif

	listen_fd = Tcp_listen("0.0.0.0",port,(socklen_t *)&llen);

	while (1) {
		struct sockaddr sa; int slen;
		slen = sizeof(sa);
		c_fd = Accept(listen_fd, &sa, (socklen_t *)&slen);
#ifdef DEBUG
		fprintf(stderr,"get connection:\n");
		Process(c_fd);
#else
		if( Fork()==0 ) {
			Close(listen_fd);
			Process(c_fd);
		}
#endif
		Close(c_fd);
	}
}
