#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>


#define LOG printf
#define MAXLEN 16384


/* convert 2 byte hex to int8, such as
   "FF" -> (int) 255
*/
uint8_t hex2int(char *buf)
{	uint8_t t;
	if( (buf[0]>='0') && (buf[0]<='9') )
		t = buf[0]-'0';
	else if( (buf[0]>='A') && (buf[0]<='F') )
		t = buf[0]-'A'+10;
	else return 0;
	t = t<<4;
	if( (buf[1]>='0') && (buf[1]<='9') )
		t += buf[1]-'0';
	else if( (buf[1]>='A') && (buf[1]<='F') )
		t += buf[1]-'A'+10;
	else return 0;
	return t;
}

/* 
http://adsb-decode-guide.readthedocs.io/en/latest/introduction.html
   decode hex "8D4840D6202CC371C32CE0576098" 28 byte message
   to DF,CA,ICAO24,DATA,PC 
*/
   
int decode_adsb_outer_layer(uint8_t *buf, uint8_t *DF, uint8_t *CA, uint8_t *ICAO24,
	uint8_t *DATA, uint8_t *TC, uint32_t *PC)
{
	int i;
	if(strlen(buf)!=28) {
		LOG("msg %s len!=28\n",buf);
		return 0;
	}
	for(i=0;i<28;i++) {
		if( isxdigit(buf[i])) continue;
		LOG("msg %s:%d %c is not a hex\n",buf,i,buf[i]);
		return 0;
	}
	uint8_t t;
	t = hex2int(buf);
	*DF = t >> 3;
	*CA = t & 0x7;
	for(i=0;i<6;i++)
		ICAO24[i]=*(buf+i+2);
	ICAO24[6]=0;
/*	
	*ICAO24 = hex2int(buf+2);
	*ICAO24 = ( *ICAO24 << 8) +hex2int(buf+4);
	*ICAO24 = ( *ICAO24 << 8) +hex2int(buf+6);
*/
	*DATA = hex2int(buf+8);
	*(DATA+1) = hex2int(buf+10);
	*(DATA+2) = hex2int(buf+12);
	*(DATA+3) = hex2int(buf+14);
	*(DATA+4) = hex2int(buf+16);
	*(DATA+5) = hex2int(buf+18);
	*(DATA+6) = hex2int(buf+20);
	*(DATA+7) = 0;
	*TC = (*DATA) >> 3;
	*PC = hex2int(buf+22);
	*PC = ( *PC << 8) + hex2int(buf+24);
	*PC = ( *PC << 8) + hex2int(buf+26);
	return 1;
}

char aidcharset[]="#ABCDEFGHIJKLMNOPQRSTUVWXYZ#####_###############0123456789######";

void decode_adsb(uint8_t *buf)
{
	uint8_t DF, CA, TC;
	uint32_t PC;
	uint8_t ICAO24[7];
	uint8_t DATA[8];
	decode_adsb_outer_layer(buf, &DF, &CA, ICAO24, DATA, &TC, &PC);
LOG("msg: %s\n",buf);
LOG("DF=%d CA=%d ICAO=%s TC=%d\n",DF,CA,ICAO24,TC);
	if(DF!=17) {
		LOG("I only know DF=17\n");
		return;
	}
	if((TC>=1) && (TC<=4)) {		// Aircraft Identification
		char aid[9];
		uint32_t t;
		t = *(DATA+1) << 16;
		t += (*(DATA+2) << 8);	
		t += (*(DATA+3));	
		aid[0] = aidcharset[t>>18];
		aid[1] = aidcharset[(t>>12)&0x3f];
		aid[2] = aidcharset[(t>>6)&0x3f];
		aid[3] = aidcharset[(t)&0x3f];
		t = *(DATA+4) << 16;
		t += (*(DATA+5) << 8);	
		t += (*(DATA+6));	
		aid[4] = aidcharset[t>>18];
		aid[5] = aidcharset[(t>>12)&0x3f];
		aid[6] = aidcharset[(t>>6)&0x3f];
		aid[7] = aidcharset[(t)&0x3f];
		aid[8]=0;	
		LOG("aid=%s\n\n",aid);
	} else if((TC>=9) && (TC<=18)) {		// Airborne Positions
		uint8_t F;
		F = (*(DATA+2) >> 2) & 1;
		LOG("F=%d\n\n",F);
	} else if(TC==19) {		// Airborne Velocity
/*
                                                                                                                                                                      
|  DATA       *   +1                          * +2       * +3             *+4                       *+5            * +6                   | CRC                      |
|-------|-----||----|--------|-----|------|---||---------||------|--------||----|-------|------|----||-------|---||-----|-------|---------|--------------------------|
|  TC   | ST  || IC | RESV_A | NAC | S-EW | V-||EW       || S-NS | V-NS   ||    | VrSrc | S-Vr | Vr ||       | RE||SV_B | S-Dif | Dif     | CRC                      |
|-------|-----||----|--------|-----|------|---||---------||------|--------||----|-------|------|----||-------|---||-----|-------|---------|--------------------------|
| 10011 | 001 || 0  | 1      | 000 | 1    | 00||00001001 || 1    | 0010100||000 | 0     | 1    | 000||001110 | 00||     | 0     | 0010111 | 010110110010100001001111 |
*/

		uint8_t ST;
		ST = *DATA & 0x7;
		LOG("ST=%d\n",ST);
		if(ST==1 || ST==2) { // subtype 1 or 2
			uint8_t S_EW,S_NS,VrSrc,S_Vr,S_Dif, Dif;
			uint16_t V_EW, V_NS, Vr;
			float V_we, V_sn ,V,h;
			S_EW = (*(DATA+1)>>2)&1;
			V_EW = ((*(DATA+1)&0x3)<<8) + *(DATA+2);
			S_NS = (*(DATA+3)>>7);
			V_NS = ((*(DATA+3)&0x7F)<<3) + (*(DATA+4)>>5);
			VrSrc = (*(DATA+4)>>4) & 1;
			S_Vr = (*(DATA+4)>>3) & 1;
			Vr = ((*(DATA+4)&0x7)<<6) + (*(DATA+5)>>2);
			S_Dif = (*(DATA+6)>>7) & 1;
			Dif = *(DATA+6) & 0x7F;
			if(S_EW) 
				V_we=1.0-V_EW;
			else V_we = V_EW-1.0;
			if(S_NS) 
				V_sn = 1.0-V_NS;
			else V_sn = V_NS-1.0;
			V = sqrt(V_sn*V_sn + V_we*V_we);
			if(abs(V_sn)<0.01) {
				if(S_EW) h = 270;
				else h = 90;
			} else if(V_sn>0) {
				h = atan(V_we/V_sn)*360/2/3.1415926;
				if(h<=0) h+=360;
			} else {
				h = atan(V_we/V_sn)*360/2/3.1415926;
				if(h<=0) h+=360;
			}

			LOG("S_EW:%d V_we=%f S_NS:%d V_sn=%f V=%.2f(kn) h=%.0f\n\n",S_EW,V_we,S_NS,V_sn,V,h);
		}
	}
}
main(int argc, char *argv[])
{	FILE *fp;
	char buf[MAXLEN];
	int i;
	decode_adsb("8D4840D6202CC371C32CE0576098");
	decode_adsb("8D40621D58C382D690C8AC2863A7");
	decode_adsb("8D40621D58C386435CC412692AD6");
	decode_adsb("8D485020994409940838175B284F");
	decode_adsb("8DA05F219B06B6AF189400CBC33F");
	fp = fopen("adsb.txt","r");
	i=0;
	while(fgets(buf,MAXLEN,fp)) {
		i++;
		if(i==1) exit;
		if(buf[0]!='*') 
			continue;
		if(strlen(buf+1)<28)
			continue;
		if(buf[29]==';') {
			buf[29]=0;
			decode_adsb(buf+1);
		}
	}
	
}
