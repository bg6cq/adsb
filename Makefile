all: adsb

adsb: adsb.c
	gcc -g -o adsb adsb.c -lm

