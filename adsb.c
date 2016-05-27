/* ADS-B decoder by james@ustc.edu.cn

thanks to:
http://adsb-decode-guide.readthedocs.io
https://github.com/etabbone/01.ADSB_BSBv6_UPS

*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>

//#define MOREDEBUG 1

#define LOG printf

#define MAXLEN 16384
// max number of airbornes to track
#define MAXID 	100

// 最近100秒接收到的ICAO和AID对应
char icaos[MAXID][7];
char aid[MAXID][9];
time_t aidtm[MAXID];

// 最近10秒的CPR位置信息包
char cpricaos[MAXID*2][7];
time_t utm[MAXID*2];
uint32_t LAT_CPR_EVEN[MAXID*2];
uint32_t LON_CPR_EVEN[MAXID*2];
int Fbit[MAXID*2];

// 保存icao对应的aid,最多保留100秒
void save_aid(char *ICAO, char *AID)
{	int i;
	time_t ctm = time(NULL);
	for(i=0; i<MAXID; i++) {
		if(icaos[i][0] == 0) // i位置为空，可以存
			break;
		if(ctm-aidtm[i] > 100)  // i 位置的数据太老，可以覆盖
			break;
		if(strcmp(icaos[i],ICAO) == 0) // i 位置存放的是之前的信息，可以覆盖
			break;
	}
	if(i == MAXID) { // 满了，不存
		LOG("WARN: Too Many Aircraft to track\n");
		return;
	}
	strcpy(icaos[i], ICAO);
	strcpy(aid[i], AID);
	aidtm[i] = ctm;
#ifdef	MOREDEBUG
	LOG("save %s:%s in %d\n", ICAO, AID, i);
#endif
}

// 根据ICAO 找出AID索引
int find_aid(char *ICAO)
{	int found = 0, i;
	time_t ctm = time(NULL);
	for(i=0; i<MAXID; i++) {
		if(icaos[i][0] == 0) // i位置为空，没找到
			break;
		if(strcmp(icaos[i], ICAO) == 0) { // 找到了
			found = 1;
			break;
		}
	}
	if(!found) 
		return -1;
#ifdef	MOREDEBUG
	LOG("found %s in %d, aid:%s\n", ICAO, i, aid[i]);
#endif
	return i;
}

// 保存最新收到的CPR，以备后续CPR计算位置
// 信息保存10秒
void save_cpr(char *ICAO, int F, uint32_t lat, uint32_t lon)
{	int i;
	time_t ctm = time(NULL);
	for(i=0; i<MAXID*2; i++) {
		if(cpricaos[i][0] == 0) // i位置为空，可以存
			break;
		if(ctm-utm[i] > 10)  // i 位置的数据太老，可以覆盖
			break;
		if((Fbit[i] == F) && (strcmp(cpricaos[i], ICAO) == 0)) // i 位置存放的是之前的信息，可以覆盖
			break;
	}
	if(i == MAXID*2) { // 满了，不存
#ifdef	MOREDEBUG
		LOG("Too Many Aircraft to track\n");
#endif
		return;
	}
	strcpy(cpricaos[i], ICAO);
	LAT_CPR_EVEN[i] = lat;
	LON_CPR_EVEN[i] = lon;
	utm[i] = ctm;
	Fbit[i] = F;
#ifdef	MOREDEBUG
	LOG("save %s in %d\n", ICAO, i);
#endif
}

// 取回最近的CPR信息, 找不到返回0
int find_cpr(char *ICAO, int F, uint32_t *lat, uint32_t *lon) 
{	int found = 0, i;
	time_t ctm = time(NULL);
	for(i=0; i<MAXID*2; i++ ) {
		if(cpricaos[i][0] == 0) // i位置为空，没找到
			break;
		if((Fbit[i] == F)&& (strcmp(cpricaos[i], ICAO) == 0 )) { // 找到了
			found = 1;
			break;
		}
	}
	if(!found) 
		return 0;
	if(ctm-utm[i] > 10)  // i 位置的数据太老, 相当于没找到
		return 0;
	*lat = LAT_CPR_EVEN[i];
	*lon = LON_CPR_EVEN[i];
#ifdef	MOREDEBUG
	LOG("found CPR_EVEN %s in %d\n", ICAO, i);
#endif
	return 1;
}

// From https://github.com/etabbone/01.ADSB_BSBv6_UPS
//
//=========================================================================
//
// Always positive MOD operation, used for CPR decoding.
//
int cprModFunction(int a, int b) {
    int res = a % b;
    if (res < 0) res += b;
    return res;
}
//
//=========================================================================
//
// The NL function uses the precomputed table from 1090-WP-9-14
//
int cprNLFunction(double lat) {
    if (lat < 0) lat = -lat; // Table is simmetric about the equator
    if (lat < 10.47047130) return 59;
    if (lat < 14.82817437) return 58;
    if (lat < 18.18626357) return 57;
    if (lat < 21.02939493) return 56;
    if (lat < 23.54504487) return 55;
    if (lat < 25.82924707) return 54;
    if (lat < 27.93898710) return 53;
    if (lat < 29.91135686) return 52;
    if (lat < 31.77209708) return 51;
    if (lat < 33.53993436) return 50;
    if (lat < 35.22899598) return 49;
    if (lat < 36.85025108) return 48;
    if (lat < 38.41241892) return 47;
    if (lat < 39.92256684) return 46;
    if (lat < 41.38651832) return 45;
    if (lat < 42.80914012) return 44;
    if (lat < 44.19454951) return 43;
    if (lat < 45.54626723) return 42;
    if (lat < 46.86733252) return 41;
    if (lat < 48.16039128) return 40;
    if (lat < 49.42776439) return 39;
    if (lat < 50.67150166) return 38;
    if (lat < 51.89342469) return 37;
    if (lat < 53.09516153) return 36;
    if (lat < 54.27817472) return 35;
    if (lat < 55.44378444) return 34;
    if (lat < 56.59318756) return 33;
    if (lat < 57.72747354) return 32;
    if (lat < 58.84763776) return 31;
    if (lat < 59.95459277) return 30;
    if (lat < 61.04917774) return 29;
    if (lat < 62.13216659) return 28;
    if (lat < 63.20427479) return 27;
    if (lat < 64.26616523) return 26;
    if (lat < 65.31845310) return 25;
    if (lat < 66.36171008) return 24;
    if (lat < 67.39646774) return 23;
    if (lat < 68.42322022) return 22;
    if (lat < 69.44242631) return 21;
    if (lat < 70.45451075) return 20;
    if (lat < 71.45986473) return 19;
    if (lat < 72.45884545) return 18;
    if (lat < 73.45177442) return 17;
    if (lat < 74.43893416) return 16;
    if (lat < 75.42056257) return 15;
    if (lat < 76.39684391) return 14;
    if (lat < 77.36789461) return 13;
    if (lat < 78.33374083) return 12;
    if (lat < 79.29428225) return 11;
    if (lat < 80.24923213) return 10;
    if (lat < 81.19801349) return 9;
    if (lat < 82.13956981) return 8;
    if (lat < 83.07199445) return 7;
    if (lat < 83.99173563) return 6;
    if (lat < 84.89166191) return 5;
    if (lat < 85.75541621) return 4;
    if (lat < 86.53536998) return 3;
    if (lat < 87.00000000) return 2;
    else return 1;
}
//
//=========================================================================
//
int cprNFunction(double lat, int fflag) {
    int nl = cprNLFunction(lat) - (fflag ? 1 : 0);
    if (nl < 1) nl = 1;
    return nl;
}
//
//=========================================================================
//
double cprDlonFunction(double lat, int fflag) {
    return 360.0 / cprNFunction(lat, fflag);
}
//
//=========================================================================
//
// This algorithm comes from:
// http://www.lll.lu/~edward/edward/adsb/DecodingADSBposition.html.
//
// A few remarks:
// 1) 131072 is 2^17 since CPR latitude and longitude are encoded in 17 bits.
// 2) We assume that we always received the odd packet as last packet for
//    simplicity. This may provide a position that is less fresh of a few
//    seconds.
//
void DecodeCPR(char *ICAO, uint32_t LAT_CPR_E, uint32_t LON_CPR_E, uint32_t LAT_CPR_O, uint32_t LON_CPR_O, uint16_t ALT, int aidindex, int fflag)
{
	int qbit = (ALT>>4)&1;
	ALT = ((ALT>>5)<<4) +  (ALT&0xf);
	uint32_t alt = ALT * ( qbit ? 25: 100) -1000;
    
    	double AirDlat0 = 360.0 / 60.0;
    	double AirDlat1 = 360.0 / 59.0;

    	double lat0 = LAT_CPR_E;
    	double lat1 = LAT_CPR_O;
    	double lon0 = LON_CPR_E;
    	double lon1 = LON_CPR_O;
	
	double lat, lon;

    // Compute the Latitude Index "j"
    	int    j     = (int) floor(((59*lat0 - 60*lat1) / 131072) + 0.5);
	
#ifdef	MOREDEBUG
LOG("j=%d ",j);
#endif
    	double rlat0 = AirDlat0 * (cprModFunction(j,60) + lat0 / 131072);
    	double rlat1 = AirDlat1 * (cprModFunction(j,59) + lat1 / 131072);

    	if (rlat0 >= 270) rlat0 -= 360;
    	if (rlat1 >= 270) rlat1 -= 360;

#ifdef	MOREDEBUG
LOG("Lat_EVEN = %f Lat_ODD = %f ", rlat0, rlat1);
#endif

    	// Check that both are in the same latitude zone, or abort.
    	if (cprNLFunction(rlat0) != cprNLFunction(rlat1)) return;

    	// Compute ni and the Longitude Index "m"
    	if (fflag) { // Use odd packet.
        	int ni = cprNFunction(rlat1,1);
        	int m = (int) floor((((lon0 * (cprNLFunction(rlat1)-1)) -
                              (lon1 * cprNLFunction(rlat1))) / 131072.0) + 0.5);
        	lon = cprDlonFunction(rlat1, 1) * (cprModFunction(m, ni)+lon1/131072);
        	lat = rlat1;
    	} else {     // Use even packet.
        	int ni = cprNFunction(rlat0,0);
        	int m = (int) floor((((lon0 * (cprNLFunction(rlat0)-1)) -
                              (lon1 * cprNLFunction(rlat0))) / 131072) + 0.5);
        	lon = cprDlonFunction(rlat0, 0) * (cprModFunction(m, ni)+lon0/131072);
        	lat = rlat0;
    }

    if (lon >= 180) {
        	lon -= 360;
    }
LOG("%s Lat:%f Lon:%f Alt:%d(ft)\n", aid[aidindex], lat, lon, alt);
}

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

/* V_we V_sn speed to head_deg */
float head_deg(float V_we, float V_sn)
{	float h;
	h = atan2(V_we, V_sn)*180.0/3.1415926;
	if(h<0) h+=360;
	return h;
}

/* http://adsb-decode-guide.readthedocs.io/en/latest/introduction.html
   decode hex "8D4840D6202CC371C32CE0576098" 28 byte message
   to DF,CA,ICAO24,DATA,PC 
*/
   
int decode_adsb_outer_layer(uint8_t *buf, uint8_t *DF, uint8_t *CA, uint8_t *ICAO24,
	uint8_t *DATA, uint8_t *TC, uint32_t *PC)
{
	int i;
	uint8_t t;
	if(strlen(buf) != 28) {
		LOG("WARN: msg %s len!=28\n",buf);
		return 0;
	}
	for(i=0; i<28; i++) {
		if( isxdigit(buf[i])) continue;
		LOG("ERROR: msg %s:%d %c is not a hex\n",buf,i,buf[i]);
		return 0;
	}
	t = hex2int(buf);
	*DF = t >> 3;
	*CA = t & 0x7;
	for(i=0;i<6;i++)
		ICAO24[i]=*(buf+i+2);
	ICAO24[6]=0;
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
	uint8_t DF, CA, TC, ICAO24[7], DATA[8];
	uint32_t PC;
	decode_adsb_outer_layer(buf, &DF, &CA, ICAO24, DATA, &TC, &PC);
#ifdef	MOREDEBUG
LOG("msg: %s\n",buf);
LOG("DF=%d CA=%d ICAO=%s TC=%d ",DF,CA,ICAO24,TC);
#endif
	if(DF!=17) {
		LOG("ERROR: I only know DF=17\n");
		return;
	}
	if((TC>=1) && (TC<=4)) {		// Aircraft Identification
		char aid[9];
		uint32_t t;
		t = (*(DATA+1)<<16) + (*(DATA+2)<<8) + (*(DATA+3));	
		aid[0] = aidcharset[t>>18];
		aid[1] = aidcharset[(t>>12)&0x3f];
		aid[2] = aidcharset[(t>>6)&0x3f];
		aid[3] = aidcharset[(t)&0x3f];
		t = (*(DATA+4)<<16) + (*(DATA+5)<<8) + (*(DATA+6));	
		aid[4] = aidcharset[t>>18];
		aid[5] = aidcharset[(t>>12)&0x3f];
		aid[6] = aidcharset[(t>>6)&0x3f];
		aid[7] = aidcharset[(t)&0x3f];
		aid[8]=0;	
		save_aid(ICAO24, aid);
#ifdef MOREDEBUG
		LOG("aid=%s\n",aid);
#endif
	} else if((TC>=9) && (TC<=18)) {	// Airborne Positions
/*
| DATA               *+1         *+2                *+3       *+4          *+5       *+6       |
| TC    | SS | NICsb || ALT     ||     | T | F | LA||T-CPR   ||        | L||ON-CPR  ||         |
|-------|----|-------||---------||-----|---|---|---||--------||--------|--||--------||---------|
| 01011 | 00 | 0     || 11000011||1000 | 0 | 0 | 10||11010110||1001000 | 0||11001000||10101100 |
| 01011 | 00 | 0     || 11000011||1000 | 0 | 1 | 10||01000011||0101110 | 0||11000100||00010010 |
*/
		uint8_t F;
		static char lastICAO[7];
		static lastF;
		uint16_t ALT;
		uint32_t LAT_CPR_E, LAT_CPR_O, LON_CPR_E, LON_CPR_O;
		int aidindex = find_aid(ICAO24);
		if(aidindex==-1)  // 没找到aid，不继续处理
			return ;
		F = (*(DATA+2)>>2) & 1;
#ifdef	MOREDEBUG
		LOG("F=%d ",F);
#endif
		if(F==0) { // F=0 时，保存 CPR信息 
			LAT_CPR_E = ((*(DATA+2)&0x3)<<15) + (*(DATA+3)<<7) + (*(DATA+4)>>1);
			LON_CPR_E = ((*(DATA+4)&1)<<16) + (*(DATA+5)<<8) + *(DATA+6);
			save_cpr(ICAO24, F, LAT_CPR_E, LON_CPR_E);
			if(find_cpr(ICAO24, 1, &LAT_CPR_O, &LON_CPR_O)) {
				DecodeCPR(ICAO24, LAT_CPR_E, LON_CPR_E, LAT_CPR_O, LON_CPR_O, ALT, aidindex, 0);
			}
		} else { // F=1 时，取得保存的CPR信息一起计算位置	
			ALT = (*(DATA+1)<<4) + (*(DATA+2)>>4);
			LAT_CPR_O = ((*(DATA+2)&0x3)<<15) + (*(DATA+3)<<7) + (*(DATA+4)>>1);
			LON_CPR_O = ((*(DATA+4)&1)<<16) + (*(DATA+5)<<8) + *(DATA+6);
			save_cpr(ICAO24, F, LAT_CPR_O, LON_CPR_O);
			if(find_cpr(ICAO24, 0, &LAT_CPR_E, &LON_CPR_E)) {
				DecodeCPR(ICAO24, LAT_CPR_E, LON_CPR_E, LAT_CPR_O, LON_CPR_O, ALT, aidindex, 1);
			}
		}
	} else if(TC==19) {			// Airborne Velocity
		uint8_t ST;
		int aidindex = find_aid(ICAO24);
		if(aidindex==-1)  // 没找到aid，不继续处理
			return ;
		ST = *DATA & 0x7;
#ifdef	MOREDEBUG
		LOG("ST=%d ",ST);
#endif
		if(ST==1 || ST==2) { // subtype 1 or 2
/*
|  DATA        *+1                             *+2        *+3              *+4                       *+5          *+6                     |
|-------|-----||----|--------|-----|------|---||---------||------|--------||----|-------|------|----||-------|---||-----|-------|---------|
|  TC   | ST  || IC | RESV_A | NAC | S-EW | V-||EW       || S-NS | V-NS   ||    | VrSrc | S-Vr | Vr ||       | RE||SV_B | S-Dif | Dif     |
|-------|-----||----|--------|-----|------|---||---------||------|--------||----|-------|------|----||-------|---||-----|-------|---------|
| 10011 | 001 || 0  | 1      | 000 | 1    | 00||00001001 || 1    | 0010100||000 | 0     | 1    | 000||001110 | 00||     | 0     | 0010111 |
*/
			uint8_t S_EW, S_NS, VrSrc, S_Vr, S_Dif, Dif;
			uint16_t V_EW, V_NS, Vr;
			float V_we, V_sn , V, h;
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
			h = head_deg(V_we, V_sn);
			
		//	LOG("S_EW:%d V_we=%f S_NS:%d V_sn=%f V=%.2f(kn) h=%.02f VR=%c%d(ft/min)\n",S_EW,V_we,S_NS,V_sn,V,h, S_Vr==0?'+':'-',Vr);
			LOG("%s V=%.2f(kn) h=%.02f VR=%c%d(ft/min)\n" ,aid[aidindex], V, h, S_Vr==0?'+':'-', Vr);
		} else if(ST==3 || ST==4) { // subtype 3 or 4
/*
|  DATA        *+1                             *+2        *+3              *+4                       *+5              *+6                |
|-------|-----||----|--------|-----|------|---||---------||------|--------||----|-------|------|----||-------|--------||-------|---------|
|  TC   | ST  || IC | RESV_A | NAC | H-s  | Hdg          || AS-t | AS     ||    | VrSrc | S-Vr | Vr          | RESV_B || S-Dif | Dif     |
|-------|-----||----|--------|-----|------|---||---------||------|--------||----|-------|------|----||-------|--------||-------|---------|
| 10011 | 011 || 0  | 0      | 000 | 1    | 10||10110110 || 1    | 0101111||000 | 1     | 1    | 000||100101 | 00     || 0     | 0000000 |
*/
			uint8_t H_S, AS_t, VrSrc, S_Vr, S_Dif, Dif;
			uint16_t Hdg, AS, Vr;
			float V_we, V_sn ,V,h;
			H_S = (*(DATA+1)>>2)&1;
			Hdg = ((*(DATA+1)&0x3)<<8) + *(DATA+2);
			AS_t = (*(DATA+3)>>7);
			AS = ((*(DATA+3)&0x7F)<<3) + (*(DATA+4)>>5);
			VrSrc = (*(DATA+4)>>4) & 1;
			S_Vr = (*(DATA+4)>>3) & 1;
			Vr = ((*(DATA+4)&0x7)<<6) + (*(DATA+5)>>2);
			S_Dif = (*(DATA+6)>>7) & 1;
			Dif = *(DATA+6) & 0x7F;
			if(H_S)
				h = Hdg/1024.0*360.0;
			LOG("%s V=%d(kn,%s) h=%.02f VR=%c%d(ft/min)\n", aid[aidindex], AS, AS_t==0?"IAS":"TAS", h, S_Vr==0?'+':'-', Vr);
		}
	}
}



int main(int argc, char *argv[])
{	char buf[MAXLEN];

//	tast data ICAO been changed, CRC not corrected
//	test data aid from http://adsb-decode-guide.readthedocs.io/en/latest/identification.html
	decode_adsb("8D4840D6202CC371C32CE0576098");
// 	test data postion from http://adsb-decode-guide.readthedocs.io/en/latest/position.html
	decode_adsb("8D4840D658C386435CC412692AD6");
	decode_adsb("8D4840D658C382D690C8AC2863A7");
//	test data Velocity from http://adsb-decode-guide.readthedocs.io/en/latest/position.html
	decode_adsb("8D4840D6994409940838175B284F");
	decode_adsb("8D4840D69B06B6AF189400CBC33F");
//	test data postion from http://www.lll.lu/~edward/edward/adsb/DecodingADSBposition.html
	decode_adsb("8D4840D7202CC381C32CE0576098");
	decode_adsb("8D4840D7580FF2CF7E9BA6F701D0");
	decode_adsb("8D4840D7580FF6B283EB7A157117");

	while(fgets(buf,MAXLEN,stdin)) {
		if(buf[0]!='*') 
			continue;
		if(strlen(buf+1)<28)
			continue;
		if(buf[29]==';') {
			buf[29]=0;
			decode_adsb(buf+1);
		}
	}
	return 0;
}
