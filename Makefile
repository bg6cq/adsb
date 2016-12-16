all: adsb send2servers tcp2servers udp2tcp tcp2multiudp

adsb: adsb.c db.h
	gcc -g -o adsb adsb.c -lm  -lmysqlclient -L/usr/lib64/mysql/

send2servers: send2servers.c
	gcc -g -o send2servers send2servers.c

tcp2servers: tcp2servers.c
	gcc -g -o tcp2servers tcp2servers.c
tcp2multiudp: tcp2multiudp.c
	gcc -g -o tcp2multiudp tcp2multiudp.c

udp2tcp: udp2tcp.c
