#ifndef OTO_H
#define OTO_H
#include "mbed.h"

void doremi(void);
void greensleeves(void);
void Buzzer_1(void);
void Buzzer_pipi(int num);
void Buzzer_bubu(int num);

#define	DO_0 	3822	//ド 	261.63,0.003822
#define	DO_0S	3608	//ド#	277.18,0.003608
#define	RE_0 	3405	//レ 	293.66,0.003405
#define	RE_0S	3214	//レ#	311.13,0.003214
#define	MI_0 	3034	//ミ 	329.63,0.003034
#define	FA_0 	2863	//ファ	349.23,0.002863
#define	FA_0S	2703	//ファ#	369.99,0.002703
#define	SO_0 	2551	//ソ	392,0.002551
#define	SO_0S	2408	//ソ#	415.3,0.002408
#define	RA_0 	2273	//ラ 	440,0.002273
#define	RA_0S	2145	//ラ#	466.16,0.002145
#define	SI_0 	2025	//シ 	493.88,0.002025
#define	DO_1 	1911	//ド 	523.25,0.001911
#define	DO_1S	1804	//ド#	554.37,0.001804
#define	RE_1 	1703	//レ 	587.33,0.001703
#define	RE_1S	1607	//レ#	622.25,0.001607
#define	MI_1 	1517	//ミ 	659.25,0.001517
#define	FA_1 	1432	//ファ	698.46,0.001432
#define	FA_1S	1351	//ファ#	739.99,0.001351
#define	SO_1 	1276	//ソ 	783.99,0.001276
#define	SO_1S	1204	//ソ#	830.61,0.001204
#define	RA_1 	1136	//ラ 	880,0.001136
#define	RA_1S	1073	//ラ#	932.33,0.001073
#define	SI_1 	1012	//シ 	987.77,0.001012
#define	DO_2 	956	//ド 	1046.5,0.000956
#define	RE_2 	902	//レ 	1108.73,0.000902
#define	RE_2S	851	//レ#	1174.66,0.000851
#define	MI_2 	804	//ミ 	1244.51,0.000804
#define	FA_2 	758	//ファ	1318.51,0.000758
#define	FA_2S	716	//ファ#	1396.91,0.000716
#define	SO_2 	676	//ソ 	1479.97,0.000676
#define	SO_2S	638	//ソ#	1567.98,0.000638
#define	RA_2 	602	//ラ 	1660,0.000602

#endif