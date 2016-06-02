all: adsb

adsb: adsb.c db.h
	gcc -g -o adsb adsb.c -lm  -lmysqlclient -L/usr/lib64/mysql/


