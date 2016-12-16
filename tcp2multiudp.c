/* tcp2multiudp v1.0 by  james@ustc.edu.cn 2016.05.11
   ./tcp2multiudp tcp_port udp_port
   accept connection on tcp port tcp_port
	send all data received to
		udp port if one IP send keepalive packets to udp_port in 60 sec 
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

// max udp_clients
#define MAXC 5000

#define MAXLEN 16384
#define STRLEN 100


int server_tcp_fd;
int server_udp_fd;
int total_clients = 0;
struct sockaddr client_addr[MAXC];
int client_addr_len[MAXC];
time_t client_lasttm[MAXC];


void fromudp(struct sockaddr *clientaddr, int addrlen){
	time_t tm;
	time(&tm);
	int i=0;

	for(i=0;i<total_clients;i++) {
		if(memcmp(&client_addr[i], clientaddr, addrlen)==0) {  // found 
			client_lasttm [i] = tm;
#ifdef DEBUG
			fprintf(stderr,"clients %d update tm\n",i);
#endif
			return;
		}
	}
	for(i=0;i<total_clients;i++) {
		if( tm - client_lasttm [i] > 90 )  {  // overwirte
			memcpy(&client_addr[i], clientaddr, addrlen);
			client_addr_len[i]=addrlen;
			client_lasttm [i] = tm;
#ifdef DEBUG
			fprintf(stderr,"overwrite clients %d\n",i);
#endif
		return;
		}
	}
	if(total_clients<MAXC-1) {
		i = total_clients;
		memcpy(&client_addr[i], clientaddr, addrlen);
		client_addr_len[i]=addrlen;
		client_lasttm [i] = tm;
		total_clients ++;
#ifdef DEBUG
		fprintf(stderr,"add clients %d\n",i);
#endif
		return;
	}
}
void sendtoudp( char *buf,int n)
{
	time_t tm;
	time(&tm);
	int i=0;
	for(i=0;i<total_clients;i++) {
		if( tm - client_lasttm [i] < 65 )  {
#ifdef DEBUG
		fprintf(stderr,"send to udp clients %d\n",i);
#endif
			sendto (server_udp_fd, buf, n, 0, &client_addr[i], client_addr_len[i]);
			}
	}
}

void Process(int server_tcp_fd, char *udp_port) 
{
        unsigned char buf[MAXLEN];
        int i,n;
        int optval;
        socklen_t optlen = sizeof(optval);
        optval = 120;
        Setsockopt(server_tcp_fd, SOL_SOCKET, SO_KEEPALIVE, &optval, optlen);
        optval = 10;
        Setsockopt(server_tcp_fd, SOL_TCP, TCP_KEEPCNT, &optval, optlen);

	int llen;	
	server_udp_fd =  Udp_server("0.0.0.0",udp_port,(socklen_t *)&llen);

	while(1) {
		fd_set rset;
		int max_fd,m;
		struct timeval tv;

		tv.tv_sec = 300;
                tv.tv_usec = 0;
		FD_ZERO(&rset);
           	FD_SET(server_tcp_fd, &rset);
           	FD_SET(server_udp_fd, &rset);
                max_fd = max(server_tcp_fd,server_udp_fd);

                m = select (max_fd + 1, &rset, NULL, NULL, &tv);

                if (m == 0)
                        continue;

                if (FD_ISSET(server_tcp_fd, &rset)) {
                        n = recv (server_tcp_fd, buf, MAXLEN,0);
                        if(n<=0)   {
				close(server_udp_fd);
                                return ;
                        }
			buf[n]=0;
#ifdef DEBUG
			fprintf(stderr,"from tcp %s\n",buf);
#endif
			sendtoudp(buf,n);
		}
                if (FD_ISSET(server_udp_fd, &rset)) {
				struct sockaddr clientaddr;
				int addrlen;
			 n = recvfrom(server_udp_fd, buf, MAXLEN, 0, 
                     		(struct sockaddr *)&clientaddr,
                     		&addrlen);
        		if (n <= 0)
            			continue;
			buf[n]=0;
#ifdef DEBUG
			fprintf(stderr,"from udp %s\n",buf);
#endif
			fromudp(&clientaddr, addrlen);
		}
        }
}

void usage()
{
	printf("tcp2multiudp v1.0 by james@ustc.edu.cn\n");
	printf("%s",
   	"./tcp2multiudp tcp_port udp_port\n"
   	"accept connection on tcp port tcp_port\n"
	"send all data received to\n"
	"	udp port if one IP send keepalive packets to udp_port in 60 sec \n");
	printf("max clients: %d\n",MAXC);
	exit(0);
}

int main(int argc, char *argv[])
{
	int listen_fd;
	int server_tcp_fd;
	int llen;
	char *tcp_port, *udp_port;
	int i;

	if(argc!=3) 
		usage();

	tcp_port = argv[1];
	udp_port = argv[2];
	printf("accepting on tcp port %s\n", tcp_port);
	printf("accepting on udp port %s\n", udp_port);
		
	signal(SIGCHLD,SIG_IGN);

#ifndef DEBUG
	daemon_init("tcp2multiudp",LOG_DAEMON);
#endif

	listen_fd = Tcp_listen("0.0.0.0",tcp_port,(socklen_t *)&llen);

	while (1) {
		struct sockaddr sa; int slen;
		slen = sizeof(sa);
		server_tcp_fd = Accept(listen_fd, &sa, (socklen_t *)&slen);
#ifdef DEBUG
		fprintf(stderr,"get connection:\n");
#endif
		Process(server_tcp_fd, udp_port);
		Close(server_tcp_fd);
	}
}
