/**
  main.cpp
  for aitendo SX032QVGA008 , 320*240 size TFT with resistive Touch Panel.
  TFT driver is ILI9341
  Controlled by SPI interface

  Other function is original on FRDM_function_SoldierII by imAkichi(c).
  
  This software Original is SpeedStudioTFT libraries by "2014 Copyright (c) Seeed Technology Inc". 
  I made by it to the reference.
*/
#define FLIPFLIP 0
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "mbed.h"
#include "SX032QVGA008.h"

#include "Consolas10.h"
//#include "Consolas12.h"
#include "Prototype33x38.h"
#include "Prototype29x28.h"
#include "Prototype17x20.h"
#include "Prototype24x34_T.h"
#include "Prototype20x25_T.h"

#include "RTC_S35190.h"
#include "EEPROM_AT93C46.h"

#include "RCS620S.h"
#if FLIPFLIP
#include "MMA8451Q.h"
#endif

#include "oto.h"
#include "cardreader.h"


#define mode_NUM 1 

// 雇用更新、退職日から4年引く
// 55歳の誕生日の1年終わったのちだったと思う
#define KOUSHIN 4
#define CHANGE_COUNTUP 60

#define PIN_XP          PTB2
#define PIN_XM          PTB0
#define PIN_YP          PTB3
#define PIN_YM          PTB1

#define PIN_MOSI        PTD2
#define PIN_MISO        PTD3
#define PIN_SCLK        PTD1
#define PIN_CS_TFT      PTD0
#define PIN_DC_TFT      PTA13
#define PIN_BL_TFT      PTE31
#define PIN_RST_TFT     PTD5
#define PIN_CS_SD       PTB8

//#define MAX_MEMORY 20   // <128/6 calendar.h
//ST7735_TFT TFT(PTD2, PTD3, PTD1, PTA13, PTD0, PTD5,"TFT"); // mosi, miso, sclk, cs, rs, reset
RTC_S35190 rtc(PTB10,PTB9,PTB8);   // PinName rtsio,PinName rtclk,PinName rtcs
#if FLIPFLIP
MMA8451Q acc(PTE25, PTE24);
#endif
//InterruptIn Sw1(PTD7);
//DigitalIn Sw1(PTD7);
//DigitalIn Sw2(PTD6);
//DigitalIn Sw3(PTA17);
//Serial pc(USBTX, USBRX); // tx, rx
Ticker tt;
PwmOut Buzz(PTE20);

EEPROM_AT93C46 eeprom(PTC17,PTC16,PTC13,PTC12); // cs,clk,di,do
RCS620S felica(PTE0, PTE1);   // RCS620S(PinName p_tx, PinName p_rx)

SX032QVGA008 TFT(PIN_XP, PIN_XM, PIN_YP, PIN_YM, PIN_MOSI, PIN_MISO, PIN_SCLK, PIN_CS_TFT, PIN_DC_TFT, PIN_RST_TFT, PIN_BL_TFT, PIN_CS_SD);
point pp;

char sel_num(int x,int y,char mins,char maxs,unsigned char * font);
void rtc_init(char *rtc_today);
void rtc_check(char *rtc_today);
void rtc_initialmode_check(char *rtc_today,int num);
void set_rtc_retire( char *rtc_retire , int initdef);
void start_display(void);
void start_menu(char *rtc_today);
void data_operation(char *rtc_retire);
int sw_2menu(int dx,int dy,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4);
int sw_1button(int dx,int dy,int x1,int y1,char* moji,unsigned char * f);
int sw_2button(int dx,int dy,int x1,int y1,int x2,int y2,char* moji1,char* moji2);
//void DrawMode(void);

int flag_t_base=0 ;
int Sw1=1,Sw2=1,Sw3=1;       // Swはタッチで判定
int mode_changer_count=0;                 // 時間が来たらモードを切り替える
int card_sense_countdown=0;               // 途中割り込みでカードセンスする場合は、時間制限で抜けてくる
uint32_t balance;
uint8_t buf[RCS620S_MAX_CARD_RESPONSE_LEN];
uint32_t namecode_data[MAX_MEMORY];  // MAX_MEMORY 人分の氏名コード 0をデフォルト表示にする
                                // 氏名コードは整数、記憶は3byteの整数とする。最大でも9999999(7桁=0x98967F)
uint32_t retidate_data[MAX_MEMORY];  // 退職日 YY/MM/DDの3byte RTCデータで保存2024/12/25は0x241225=2363941
int16_t accdata[3];
int tft_orient=3;    // TFT向きデフォルト3。SW左側。1で逆にする。
int curr_data_offset = 0;   // namecode/retidateの現在の選択を示す、デフォルトは0
int felica_ret;
float bkld=0.5;


void t_base(){
    flag_t_base++;
    mode_changer_count++;
}

int main(){
    char rtc_today[7] , rtc_retire[7] ;
    char datestr[10];
    char timestr[8];
//    long timeserial_start,timeserial_now;
    long days_remain, times_remain;
//    int times_ss=0;     // 読み込んだものが1sec進んでたら処理する
    int mode_change=0;  // mode変えたら最初だけ画面初期化する
    int mode=0;
    char month[12][4]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };
//    char week[7]={133,134,128,129,130,131,132 };   // 0/1/1=Saturday  Kanji weekday = moji code set

    Buzz.period(0.001);
    Buzz = 0;

    Buzzer_pipi(3);    
//    Buzz = 0;  // duty=0%? off?
    TFT.setBacklight(bkld);

//    Buzz.period(0);
    felica.timeout = COMMAND_TIMEOUT;
    tt.attach( &t_base , 500ms ); // 1秒ごとに割り込む
//    Sw1.fall( &sw1_int );
    
    TFT.set_orientation( tft_orient );
//    centerx = TFT.width() >> 1;
//    centery = TFT.height() >> 1; 
//    TFT.claim(stdout);      // send stdout to the TFT display 
    //TFT.claim(stderr);      // send stderr to the TFT display
    TFT.background(Black);    // set background to black
    TFT.foreground(White);    // set chars to white
    TFT.cls();
    TFT.locate(0,0);
//    TFT.set_font((unsigned char*) Consolas9x16);  // select the font
//    TFT.set_font((unsigned char*) Consolas7x13);  // select the font
    //TFTreset = 0;
    //Configure the display driver
//    TFT.setBacklight(0);
    TFT.setBacklight(0.8);

    start_display();    //Print a welcome message
    thread_sleep_for(3000);            //Wait for 5 seconds

    rtc.portinit();
    rtc_check(rtc_today);
    rtc.rd_today(rtc_today);
    rtc.getDateStr(datestr,rtc_today);
    rtc.getTimeStr(timestr,rtc_today);

    start_menu(rtc_today);
    
    felica_ret = 0;
    int ret_loop=0;
    while (felica_ret == 0) {
        thread_sleep_for(1000);
        felica_ret = felica.initDevice();
        ret_loop++;
        if(ret_loop>10)break;
    }
    if(felica_ret==0){
        printBalanceLCD("felica Err",0,0,0);
        thread_sleep_for(5000);
    }
//    rtc_initialmode_check(rtc_today,5);// 起動時、Sw1,Sw2が同時押下時、初期化モードに入る。
//                                       // 25x0.2=5sec 検出を待つ時間
    set_rtc_retire( rtc_retire , 1 );      // 退職日をセットする rtc_today全く同じデータ形式 

/*********************
 * ここからメインループ
 */
    mode_changer_count=0;
    mode_change=1;
    while(1){
        //if(Sw3==0){
        //    data_operation(rtc_retire);   // eeprom dataを調べる、なおす。
        //    mode=0;
        //    mode_change=1;
        //}
        rtc.rd_today(rtc_today);

        rtc.getTimeStr(timestr,rtc_today);
    //for(int ii=0;ii<7;ii++)TFT.printf(":%2X",rtc_today[ii]);
        char month_num = rtc.RtcData2Dec(rtc_today[1]);
        days_remain  = rtc.getDaySerial2( rtc_retire ) - rtc.getDaySerial2( rtc_today ) - 1;
        // 当日を引かないと1日多くなる
        times_remain = rtc.getTimeSerial( rtc_retire ) - rtc.getTimeSerial( rtc_today );

        int yy=0;
        int sensed_id=0;
        int swa=0;
        switch( mode ){
          case 0:
            TFT.background(Black);
            if(mode_change==1){
                TFT.cls();
                TFT.foreground(Cyan);
                TFT.set_font( (unsigned char *)Prototype29x28 );
                TFT.locate(0,0);TFT.printf("User:");
                TFT.set_font( (unsigned char *)Prototype20x25 );
                TFT.foreground(Yellow);
                TFT.printf("%07d",(int)namecode_data[ curr_data_offset ]);
//                TFT.Bitmap(210,120,110,120,(unsigned char*)asuna_110x120);
                TFT.line(  0, 30,319, 30,White);
                TFT.set_font((unsigned char*) Prototype17x20);  // select the font
                TFT.foreground(Yellow);
                TFT.locate( 10, 40);TFT.printf("Retire");
                TFT.foreground(GreenYellow);
                TFT.locate(100, 40);TFT.printf("20%02d/%s/%d",rtc.RtcData2Dec(rtc_retire[0]),month[rtc.RtcData2Dec(rtc_retire[1])-1],rtc.RtcData2Dec(rtc_retire[2]));
                TFT.foreground(Orange);
                TFT.locate( 10, 67);TFT.printf("Today");
                TFT.foreground(Cyan);
                TFT.locate(100, 67);TFT.printf("20%02d/%s/%d",rtc.RtcData2Dec(rtc_today[0]),month[month_num-1],rtc.RtcData2Dec(rtc_today[2]));
                TFT.foreground(Blue);
                TFT.locate(  3,162);TFT.printf("Works(");
                TFT.locate(220,164);TFT.printf(")");
                
                TFT.set_font((unsigned char*)Prototype33x38);
                TFT.foreground(White);
                TFT.locate( 60,125); TFT._putc(67);// 残 142
                TFT.locate(220,123); TFT._putc(59);// 日 134
                TFT.line( 60,120,255,120,Blue);
                TFT.line( 60,122,255,122,Navy);
                mode_change=0;
                
                TFT.rect( 30 ,190, 30+110,190+40,Yellow);
                TFT.rect( 180,190,180+110,190+40,Yellow);
                
            }
            TFT.set_font((unsigned char*)Prototype20x25);  // select the font
            TFT.foreground(LightGrey);
            TFT.locate(100, 93);   TFT.printf("%02d:%02d",rtc.RtcData2Dec(rtc_today[4]&0xBF),rtc.RtcData2Dec(rtc_today[5]));
            TFT.foreground(Red);
            TFT.set_font((unsigned char*)Prototype24x34);
            TFT.locate(100,125);    TFT.printf("%5d",days_remain);
//            TFT.foreground(Orange);
            TFT.set_font((unsigned char*)Prototype20x25);
//            TFT.locate(118,159);    TFT.printf("%5d",days_remain-(365*4));
            TFT.locate(118,159);    TFT.printf("%5d",( 240 * days_remain ) / 365 );
            swa = sw_2button(110,40,30,190,180,190,"Next","User");
            if( swa==1 ){
                mode=1;
                mode_change=1;
            } else if(swa==2){
                data_operation(rtc_retire);   // eeprom dataを調べる、なおす。
                mode=0;
                mode_change=1;
            } else if(swa==10){
                start_menu(rtc_today);
                mode=0;
                mode_change=1;
            }
          break;
          case 1:
            if(mode_change==1){
                TFT.background(Black);
                TFT.foreground(White);
                TFT.cls();
                TFT.set_font( (unsigned char *)Prototype33x38 );
                TFT.locate(0,0);
                // 定年たいまぁ 148 143 149 150 151 152
                TFT._putc(73);TFT._putc(68);TFT._putc(74);TFT._putc(75);TFT._putc(76);TFT._putc(77);
                TFT.line(  0, 38,319, 38,Yellow);
                TFT.set_font((unsigned char*) Prototype17x20);  // select the font
                TFT.foreground(Yellow);
                TFT.locate( 10, 40);TFT.printf("User:");
                TFT.foreground(GreenYellow);
                TFT.locate( 90, 40);TFT.printf("%07d",namecode_data[ curr_data_offset ]);

                TFT.foreground(Yellow);
                TFT.set_font((unsigned char*)Prototype33x38);
                TFT.locate(125, 63);    TFT._putc(68); // 年 143
                TFT.locate(215, 63);    TFT._putc(69);TFT._putc(53);// ヶ月 144 128
                TFT.locate(215, 96);    TFT._putc(59); // 日 134
                TFT.locate(248, 95);    TFT._putc(80); // 間 155
                TFT.locate(215,127);    TFT._putc(78); // 時 153
                TFT.locate(248,127);    TFT._putc(80); // 間 155
                TFT.locate(215,163);    TFT._putc(79); // 分 154
                TFT.locate(248,162);    TFT._putc(80); // 間 155

                mode_change=0;
            }
            TFT.set_font((unsigned char*)Prototype24x34);  // select the font
            TFT.background(Black);
            TFT.foreground(Cyan);
            yy=days_remain/365;
            TFT.locate( 75, 65);    TFT.printf("%2d",yy );
            TFT.foreground(Yellow);
            TFT.set_font((unsigned char*)Prototype24x34);  // select the font
            TFT.locate(165, 65);    TFT.printf("%2d",(days_remain-(yy*365))/30 );

            TFT.set_font((unsigned char*)Prototype20x25);  // select the font
            TFT.locate( 10,102);    TFT.printf("%10d",days_remain );

            TFT.set_font((unsigned char*)Prototype20x25);  // select the font
            TFT.foreground(Orange);
            TFT.locate( 10,132);  TFT.printf("%10d",days_remain*24+(times_remain/3600) );
            
            TFT.set_font((unsigned char*)Prototype20x25);
            TFT.foreground( RGB(200,180,0) );
            TFT.locate( 10,165);  TFT.printf("%10d",(days_remain*24*60)+(times_remain/60) );
            if( sw_1button(110,40,30,195,"Next",(unsigned char*)Prototype20x25) == 1 ){
                mode=0;
                mode_change=1;
            }

          break;
          case 2:
            TFT.fillrect( 50, 60,270,180,Black);
            TFT.rect( 52, 62,268,178,Yellow);
            sensed_id = cardreading_mode(card_sense_countdown);     // 抜けるまでここで。
            if(sensed_id !=0){ // COMPANY_IDが読めた時だけ0以外が返っている
              for(int i=0;i<MAX_MEMORY;i++){
                if(namecode_data[i]==sensed_id){
                    curr_data_offset = i;
                    break;
                }
              }
            }
            set_rtc_retire(rtc_retire,0);  // 退職日を再セット 
            mode_change=1;
            mode=0;
            mode_changer_count=0;
          break;
          default:
          break;
        } // for switch()
        if(felica_ret!=0){
            if(felica.polling( 0xFFFF )){
                 mode=2;    // Card有ったら読みモードへ移行
                 card_sense_countdown=5;
                 Buzzer_pipi(1);
            } else {
                felica.rfOff();
                while( flag_t_base < 2 )thread_sleep_for(100);// 連続で読もうとすると止まる(volatile宣言?)
            }
        }
        //if(flag_t_base>2)
        flag_t_base=0;
#if FLIPFLIP
        acc.getAccAllAxis(accdata);
        int tft_orient2;
        tft_orient2 = tft_orient;
        if(accdata[1]>2000){
            tft_orient2=3;
        } else if(accdata[1]<-2000){
            tft_orient2=1;
        }
        if(tft_orient2 != tft_orient){
            tft_orient=tft_orient2;
            TFT.set_orientation( tft_orient );
            mode_change=1;
        }
#endif
    } // for while()
}

void start_display(){
    TFT.cls();
    TFT.set_font((unsigned char*) Prototype33x38);
    int w=TFT.width();
    int h=TFT.height();
    TFT.locate( w/2-(33*7)/2, h/2-50);
    //for(char i=135;i<142;i++)TFT._putc(i); // TAKA BO C CHI TAN KEN TAI
    for(char i=60;i<67;i++)TFT._putc(i); // TAKA BO C CHI TAN KEN TAI
    TFT.fillrect(w/2-100,h/2-2,w/2+100,h/2+2,Orange);
    TFT.locate( w/2-120, h/2+50-38);
    //TFT._putc(52);TFT.printf("2014");// 127
    TFT.set_font((unsigned char*) Prototype29x28);
    TFT.printf("mbed-ce ver.");
}

// お絵かきモード
#if 0   // ROM領域足りず・・・
void DrawMode(){
    TFT.set_font( (unsigned char *)Consolas7x13 );
    TFT.rect(280,210,320,239,Green);
    TFT.rect(280,  0,320, 30,Blue );
    for(int ix=80;ix<320;ix+=80) TFT.line(ix,0,ix,239,Yellow);
    for(int iy=60;iy<240;iy+=60) TFT.line(0,iy,319,iy,Yellow);
    TFT.locate(285,220);TFT.printf("CLR");
    TFT.locate(285, 10);TFT.printf("TOP");
    while(1){
        TFT.getPixel(pp);
        TFT.locate(0,0);
        TFT.printf("[%3d,%3d]",pp.x,pp.y);
        if( (pp.x>280)&&(pp.y>210) ){
            TFT.cls();
            TFT.rect(280,210,320,239,Green);
            TFT.rect(280,  0,320, 30,Blue );
            TFT.locate(285,220);TFT.printf("CLR");
            TFT.locate(285, 10);TFT.printf("TOP");
            for(int ix=80;ix<320;ix+=80) TFT.line(ix,0,ix,239,Yellow);
            for(int iy=60;iy<240;iy+=60) TFT.line(0,iy,319,iy,Yellow);
        } else if( (pp.x>280)&&(pp.y<30)&&(pp.y>0) ){
            return;
        } else {
//        TFT.pixel(pp.x,pp.y,White);
            if( (pp.x==320)&&(pp.y==0) ){
                ;
            } else {
                TFT.fillcircle(pp.x,pp.y,1,White);
            }
        }
        wait(0.01);
    }
}
#endif

int sw_2button(int dx,int dy,int x1,int y1,int x2,int y2,char* moji1,char* moji2){
    struct button {
        int xu;
        int yu;
        int xd;
        int yd;
    };
    button sw1 = {  x1, y1, x1+dx, y1+dy};
    button sw2 = {  x2, y2, x2+dx, y2+dy};
    button sw10= { 270,  0,320, 50 };
    TFT.set_font((unsigned char *)Prototype29x28);
    TFT.foreground(White);
    TFT.background(Black);
    TFT.locate( x1+5, y1+5);TFT.printf("%s",moji1);
    TFT.locate( x2+5, y2+5);TFT.printf("%s",moji2);

    TFT.getPixel(pp);
    if( (pp.x>sw1.xu)&&(pp.x<sw1.xd)&&(pp.y>sw1.yu)&&(pp.y<sw1.yd) ){
        TFT.foreground(Black);
        TFT.background(White);
        TFT.rect(sw1.xu , sw1.yu , sw1.xd , sw1.yd , Yellow);
        TFT.locate( x1+5, y1+5);TFT.printf("%s",moji1);
        return 1;
    } else if( (pp.x>sw2.xu)&&(pp.x<sw2.xd)&&(pp.y>sw2.yu)&&(pp.y<sw2.yd) ){
        TFT.foreground(Black);
        TFT.background(White);
        TFT.rect(sw2.xu , sw2.yu , sw2.xd , sw2.yd , Yellow);
        TFT.locate( x2+5, y2+5);TFT.printf("%s",moji2);
        return 2;
    } else if( (pp.x>sw10.xu)&&(pp.x<sw10.xd)&&(pp.y>sw10.yu)&&(pp.y<sw10.yd) ){
        return 10;
    } else return 0;
}

int sw_1button(int dx,int dy,int x1,int y1,char* moji,unsigned char * f){
    struct button {
        int xu;
        int yu;
        int xd;
        int yd;
    };
    button sw1 = { x1, y1, x1+dx, y1+dy};
    TFT.set_font((unsigned char *)Prototype29x28);
    TFT.foreground(White);
    TFT.background(Black);
    TFT.rect(sw1.xu , sw1.yu , sw1.xd , sw1.yd , Yellow);
    TFT.locate( x1+5, y1+5);TFT.printf("%s",moji);
    
    TFT.getPixel(pp);
    if( (pp.x>sw1.xu)&&(pp.x<sw1.xd)&&(pp.y>sw1.yu)&&(pp.y<sw1.yd) ){
        TFT.foreground(Black);
        TFT.background(White);
        TFT.rect(sw1.xu , sw1.yu , sw1.xd , sw1.yd , Yellow);
        TFT.locate( x1+5, y1+5);TFT.printf("%s",moji);
        TFT.set_font( f );  // fontを元に戻す
        TFT.foreground(White);
        TFT.background(Black);
        return 1;
    }else {
        TFT.set_font( f );  // fontを元に戻す
        return 0;
    }
}

int sw_2menu(int dx,int dy,int x1,int y1,int x2,int y2,int x3,int y3,int x4,int y4){
    int loop_num=0;
//    float bkld=0.6;
    struct button {
        int xu;
        int yu;
        int xd;
        int yd;
    };
    button sw1 = { x1, y1, x1+dx, y1+dy};
    button sw2 = { x2, y2, x2+dx, y2+dy};
    button sw3 = { x3, y3, x3+dx, y3+dy};
    button sw4 = { x4, y4, x4+dx, y4+dy};
    button sw5 = { 160-60,190,160   ,239};
    button sw6 = { 160   ,190,160+60,239};
    TFT.foreground(White);
    TFT.background(Black);
    TFT.locate( 220,195 );TFT.set_font((unsigned char *)Prototype20x25);
    TFT.printf("%4.2f",bkld);
    TFT.setBacklight(bkld);

    TFT.set_font((unsigned char *)Prototype29x28);
    TFT.rect(sw1.xu , sw1.yu , sw1.xd , sw1.yd , Yellow);
    TFT.rect(sw2.xu , sw2.yu , sw2.xd , sw2.yd , Yellow);
    TFT.rect(sw3.xu , sw3.yu , sw3.xd , sw3.yd , Yellow);
    TFT.rect(sw4.xu , sw4.yu , sw4.xd , sw4.yd , Green);
//    TFT.rect(sw5.xu , sw5.yu , sw5.xd , sw5.yd , Blue);
//    TFT.rect(sw6.xu , sw6.yu , sw6.xd , sw6.yd , Blue);
    TFT.line(sw5.xu , sw5.yu , sw5.xd , sw5.yu , Blue);
    TFT.line(sw5.xu , sw5.yu , sw5.xu+(sw5.xd-sw5.xu)/2 , sw5.yd , Blue);
    TFT.line(sw5.xd , sw5.yu , sw5.xu+(sw5.xd-sw5.xu)/2 , sw5.yd , Blue);
    TFT.line(sw6.xu , sw6.yd , sw6.xd , sw6.yd , Blue);
    TFT.line(sw6.xu , sw6.yd , sw6.xu+(sw6.xd-sw6.xu)/2 , sw6.yu , Blue);
    TFT.line(sw6.xd , sw6.yd , sw6.xu+(sw6.xd-sw6.xu)/2 , sw6.yu , Blue);
 

//    TFT.locate( x1+5, y1+5);TFT.printf("Draw");
    TFT.locate( x2+5, y2+5);TFT.printf("Timer");
//    TFT.locate( x3+5, y3+5);TFT.printf("Calib");
    TFT.locate( x4+5, y4+5);TFT.printf("SetTD");
    TFT.locate(   10, 195 );TFT.printf("BKL:");
    while(1){
        TFT.getPixel(pp);
//        if( (pp.x>sw1.xu)&&(pp.x<sw1.xd)&&(pp.y>sw1.yu)&&(pp.y<sw1.yd) ){
//            TFT.foreground(Black);
//            TFT.background(White);
//            TFT.rect(sw1.xu , sw1.yu , sw1.xd , sw1.yd , Yellow);
//            TFT.locate( x1+5, y1+5);TFT.printf("Draws");
//            return 1;
//        }
        if( (pp.x>sw2.xu)&&(pp.x<sw2.xd)&&(pp.y>sw2.yu)&&(pp.y<sw2.yd) ){
            TFT.foreground(Black);
            TFT.background(White);
            TFT.rect(sw2.xu , sw2.yu , sw2.xd , sw2.yd , Yellow);
            TFT.locate( x2+5, y2+5);TFT.printf("Timer");
            return 2;
        }
        if( (pp.x>sw3.xu)&&(pp.x<sw3.xd)&&(pp.y>sw3.yu)&&(pp.y<sw3.yd) ){
            TFT.foreground(Black);
            TFT.background(White);
            TFT.rect(sw3.xu , sw3.yu , sw3.xd , sw3.yd , Yellow);
            TFT.locate( x3+5, y3+5);TFT.printf("Calib");
            return 3;
        }
        if( (pp.x>sw4.xu)&&(pp.x<sw4.xd)&&(pp.y>sw4.yu)&&(pp.y<sw4.yd) ){
            TFT.foreground(Black);
            TFT.background(White);
            TFT.rect(sw4.xu , sw4.yu , sw4.xd , sw4.yd , Yellow);
            TFT.locate( x4+5, y4+5);TFT.printf("SetTD");
            return 4;
        }
        if( (pp.x>sw5.xu)&&(pp.x<sw5.xd)&&(pp.y>sw5.yu)&&(pp.y<sw5.yd) ){
            bkld -=0.05;
            if(bkld<0.05)bkld=0.05;
            TFT.locate( 220,195 );TFT.set_font((unsigned char *)Prototype20x25);
            TFT.printf("%4.2f",bkld);
            TFT.set_font((unsigned char *)Prototype29x28);
            TFT.setBacklight(bkld);
            loop_num=0;
        }
        if( (pp.x>sw6.xu)&&(pp.x<sw6.xd)&&(pp.y>sw6.yu)&&(pp.y<sw6.yd) ){
            bkld +=0.05;
            if(bkld>1.0)bkld=1.0;
            TFT.locate( 220,195 );TFT.set_font((unsigned char *)Prototype20x25);
            TFT.printf("%4.2f",bkld);
            TFT.set_font((unsigned char *)Prototype29x28);
            TFT.setBacklight(bkld);
            loop_num=0;
        }
        thread_sleep_for(50);
        loop_num++;
        if(loop_num>100) return 2;
    }
    return 0;
}

void start_menu(char *rtc_today){
    int loop_startmenu=1;
    while( loop_startmenu ){
        TFT.foreground(White);
        TFT.background(Black);
        TFT.set_font( (unsigned char *)Prototype29x28 );
        TFT.cls();
        int openingsw = sw_2menu(115,40,20,50,180,50,20,120,180,120);
        switch (openingsw){
        case 1:
            TFT.foreground(White);
            TFT.background(Black);
            TFT.cls();
            loop_startmenu=1;
//            DrawMode();
            break;
        case 2:
            loop_startmenu=0;
            break;
        case 3:
            TFT.set_font( (unsigned char *)Prototype17x20 );
            //TFT.calibrate();
            break;
        case 4:
            TFT.set_font( (unsigned char *)Prototype17x20 );
            rtc_init(rtc_today);
            break;
        default:
            break;
        }
    }
}

char sel_num(int keta,int line,char mins,char maxs,unsigned char * font){   // maxss =23 or 59
    signed char dec;
    int sx=220,sy=30,dx,dy;
    dx=90;
    dy=40;
    dec=mins;
    while(1){

        TFT.locate(keta,line);
        TFT.printf("%2d",dec);
        //if( Sw1 == 0 ){  // increment
        if( sw_1button(dx,dy,sx,sy,"UP",font) == 1 ){
            thread_sleep_for(100);
            while(Sw1==0);
            dec++;
            if(dec>maxs)dec=mins;
        }
        //if( Sw2 == 0 ){  // decrement
        if( sw_1button(dx,dy,sx,sy+60,"DN",font) == 1 ){
            thread_sleep_for(100);
            while(Sw2==0);
            dec--;
            if(dec<mins)dec=maxs;
        }
        
        //if( Sw3 == 0 ){ // determin.
        if( sw_1button(dx,dy,sx,sy+120,"ENT",font) == 1 ){
            thread_sleep_for(100);
            while(Sw3==0);
            break;
        }
        thread_sleep_for(50);
    }
    while( TFT.getPixel(pp) );  // getPixelの返り値は(TOUCH==YES)の判定結果。手を放すまで待つ。
    return (char)dec;
}

/* ***********
 * RTC設定モード
 */
void rtc_init(char *rtc_today){
    int sx=220,sy=30,dx=90,dy=40;
    int keta=0,ans=0,gyo=0;
    TFT.set_font((unsigned char *)Prototype17x20);
    TFT.background(Black);
    TFT.foreground(White);
    TFT.cls();
    TFT.locate(0,0);
    TFT.printf("Initial mode!!");// line,colum,strの順番
    thread_sleep_for(500);
    TFT.line(0,20,320,20,Orange);
    TFT.foreground(Green);
    TFT.locate(0,25);TFT.printf("Date&Time Set?");
    TFT.foreground(White);
    TFT.locate(40,48);TFT.printf("YES or NO");
    TFT.rect(sx,sy,sx+dx,sy+dy,Yellow);
    sy += 60;
    TFT.rect(sx,sy,sx+dx,sy+dy,Yellow);
    sy += 60;
    TFT.rect(sx,sy,sx+dx,sy+dy,Yellow);
    ans=0;
    while(1){
        sy=30; // YES
        if( sw_1button(dx,dy,sx,sy,"YES",(unsigned char*)Prototype20x25) == 1 ){
            thread_sleep_for(100);
            ans=1;
            break;
        }  // NO
        if( sw_1button(dx,dy,sx,sy+60,"NO",(unsigned char*)Prototype20x25) == 1 ){
            thread_sleep_for(100);
            ans=0;
            break;
        }
        thread_sleep_for(50);
    }
    if(ans==1){
        thread_sleep_for(200);
        rtc.reset();
        rtc_today[0] = 0x14;
        rtc_today[1] = 0x05;
        rtc_today[2] = 0x02;
        rtc_today[3] = 0x00;
        rtc_today[4] = 0x00;
        rtc_today[5] = 0x00;
        rtc_today[6] = 0x00;
        TFT.set_font((unsigned char *)Prototype20x25);
        gyo=100;
        TFT.locate(0,gyo);TFT.printf("20");
        keta  =40;rtc_today[0] = rtc.Dec2RtcData(sel_num(keta,gyo,15,30,(unsigned char *)Prototype20x25));TFT.printf("/");
        keta +=60;rtc_today[1] = rtc.Dec2RtcData(sel_num(keta,gyo, 1,12,(unsigned char *)Prototype20x25));TFT.printf("/");
        keta +=60;rtc_today[2] = rtc.Dec2RtcData(sel_num(keta,gyo, 1,31,(unsigned char *)Prototype20x25));
        keta=0;gyo=160;
        TFT.locate(keta,gyo);
        keta  = 0;rtc_today[4] = rtc.Dec2RtcData(sel_num(keta,gyo, 0,23,(unsigned char *)Prototype20x25));TFT.printf(":");
        keta +=60;rtc_today[5] = rtc.Dec2RtcData(sel_num(keta,gyo, 0,59,(unsigned char *)Prototype20x25));TFT.printf(":");
        keta +=60;rtc_today[6] = rtc.Dec2RtcData(sel_num(keta,gyo, 0,59,(unsigned char *)Prototype20x25));
        rtc.wr_today(rtc_today);
    } else {
        return ;
    }
}

void rtc_check(char *rtc_today){
    char stat_data = rtc.get_stat();
//    TFT.printf("stat=%02x",stat_data);
    if(stat_data > 0 ){
        rtc_init(rtc_today); // 異常フラグありで初期化
    }
}

void rtc_initialmode_check(char *rtc_today,int num){
    int j=0;
    while(1){
        if( (Sw1==0) && (Sw2==0) ){
            rtc_init(rtc_today);
            break;
        }
        j++;if(j>num)break;
        thread_sleep_for(200);
    }
}

void set_rtc_retire( char *rtc_retire ,int initdef){
    int addr=0;
#if 0
    eeprom.write_enable();
    for(int addr=0;addr<128;addr++)eeprom.erase(addr);  // 消去すると1になる(0xFF)
    int namecode=99206;
    eeprom.write(addr  ,(unsigned short)((namecode>>16) & 0xff));
    eeprom.write(addr+1,(unsigned short)((namecode>> 8) & 0xff));
    eeprom.write(addr+2,(unsigned short)( namecode      & 0xff));
    eeprom.write(addr+3,(unsigned short)( 0x36 ));
    eeprom.write(addr+4,(unsigned short)( 0x09 ));
    eeprom.write(addr+5,(unsigned short)( 0x07 ));
    eeprom.write_disable();
#endif
    if(initdef==1){ // initdef=1のときは、eepromからの読みだしを入れる
        for(int i=0;i<MAX_MEMORY;i++){ // 全データ読み出し
            addr=i*6;
            namecode_data[i]= (eeprom.read(addr  )<<16)+(eeprom.read(addr+1)<<8)+eeprom.read(addr+2);
            retidate_data[i]= (eeprom.read(addr+3)<<16)+(eeprom.read(addr+4)<<8)+eeprom.read(addr+5);
        }
    }
    if(initdef==2){ // initdef=2のときは、Defaultを入れる
        namecode_data[0]= 99206;
        retidate_data[0]= 0x360907;
    }
//    namecode_data[0]= (eeprom.read(0)<<16)+(eeprom.read(1)<<8)+eeprom.read(2);
//    retidate_data[0]= (eeprom.read(3)<<16)+(eeprom.read(4)<<8)+eeprom.read(5);
    rtc_retire[0] = (char)(retidate_data[ curr_data_offset ]>>16)&0xFF;
    rtc_retire[1] = (char)(retidate_data[ curr_data_offset ]>> 8)&0xFF;
    rtc_retire[2] = (char)(retidate_data[ curr_data_offset ]    )&0xFF;
#if 0   // 多年数チェック用
    rtc_retire[0] = rtc.Dec2RtcData( 37 );  // Year=2037
    rtc_retire[1] = rtc.Dec2RtcData(  2 );  // Month=2
    rtc_retire[2] = rtc.Dec2RtcData( 25 );  // Day=25
#endif
    rtc_retire[4] = rtc.Dec2RtcData( 0 );
    rtc_retire[5] = rtc.Dec2RtcData( 0 );
    rtc_retire[6] = rtc.Dec2RtcData( 0 );
}

#if 0
void start_display(){
    TFT.cls();
    TFT.set_font((unsigned char*) Prototype33x38);
    TFT.locate( 15,  0);
    for(char i=135;i<139;i++)TFT._putc(i); // TAKA BO C CHI TAN KEN TAI
    TFT.locate( 30, 38);
    for(char i=139;i<142;i++)TFT._putc(i); // TAKA BO C CHI TAN KEN TAI
    TFT.fillrect(5,78,155,80,Orange);
    TFT.locate( 20, 85);
    TFT._putc(127);TFT.printf("2014");
    Buzzer_1();
}
#endif

void ope_dispcode(int id,int colf,int colb,int rev){
    int dx,dy,dy0=75;
    if(rev==1){
        TFT.background(colf);TFT.foreground(colb);
    } else {
        TFT.background(colb);TFT.foreground(colf);
    }
    if(id<=6){
        dx=0;
        dy=dy0+id*14;
    } else if(id<=13) {
        dx=53;
        dy=dy0+(id-7)*14;
    } else {
        dx=106;
        dy=dy0+(id-14)*14;
    }
    TFT.locate(dx,dy);
    if(namecode_data[id]<10000000){
        TFT.printf("%07d",namecode_data[id]);
    } else {
        TFT.printf("-------",namecode_data[id]);
    }
}

int ope_changecode(int idnum){
    int ret=2;  // 変更したら1を返す。キャンセルは0、キャンセル時有効IDの場合に3を返し表示IDを変更する。
    int sx=220,sy=30,dx,dy;
    dx=90;
    dy=40;
    //while(Sw3 == 0);    // 離すまでまつ。
    thread_sleep_for(100);
    TFT.background(Black);
    TFT.foreground(White);
//    TFT.cls();
    TFT.set_font( (unsigned char *)Prototype17x20 );
    TFT.locate(0,0);
    TFT.printf("FeliCa read Card!");
    TFT.set_font( (unsigned char *)Consolas7x13 );

    int sensed_id=0,org_id;
    org_id=namecode_data[idnum];
    int loop=0;
    while(1){
        if(namecode_data[idnum]<10000000){
        } else {
            TFT.locate(0,20);TFT.printf("%02d:-------",idnum);
            TFT.set_font( (unsigned char *)Prototype17x20 );
            TFT.locate(0,45);TFT.printf("[%06X]",retidate_data[idnum]);
        }


        TFT.set_font( (unsigned char *)Prototype20x25 );
        if( (loop%2)==0 ){
            TFT.foreground(Magenta);
            TFT.locate(0,20);TFT.printf("%02d:*******",idnum);
            TFT.set_font( (unsigned char *)Prototype17x20 );
            TFT.locate(0,45);TFT.printf("[%06X]",retidate_data[idnum]);
        } else {
            TFT.foreground(Magenta);
            TFT.locate(0,20);TFT.printf("%02d:-------",idnum);
            TFT.set_font( (unsigned char *)Prototype17x20 );
            TFT.locate(0,45);TFT.printf("[%06X]",retidate_data[idnum]);
        }
        TFT.set_font( (unsigned char *)Consolas7x13 );

        if( felica.polling(COMPANY_ID_SYSTEM_CODE) ){
            if(requestService(COMPANY_ID_SERVICE_CODE)){
                if(readEncryption(COMPANY_ID_SERVICE_CODE, 0, buf)){
                    Buzzer_pipi(2);
                    sensed_id=(buf[15]-0x30);
                    for(int i=16;i<22;i++){
                        sensed_id = sensed_id*10+(buf[i]-0x30);
                    }
                    namecode_data[idnum]=sensed_id;
                    //felica.rfOff();
                }
            }
        }
        felica.rfOff();
        loop++;
        //if(Sw1==0){ // キャンセル
        if( sw_1button(dx,dy,sx,sy,"CAN",(unsigned char*)Consolas7x13) == 1 ){
           thread_sleep_for(100);;//while(Sw1==0);
            if(namecode_data[idnum]<10000000)ret=3;
            break;
        }
        if(sensed_id != 0){
            while( 1 ){
                TFT.set_font( (unsigned char *)Prototype20x25 );
                TFT.locate(0,20);
                TFT.foreground(Yellow);
                TFT.printf("%02d:%07d",idnum,namecode_data[idnum]);
                TFT.set_font( (unsigned char *)Prototype17x20 );
                TFT.locate(0,45);TFT.printf("[%06X]",retidate_data[idnum]);
                thread_sleep_for(100);
                //if(Sw1==0){ // やめる
                if( sw_1button(dx,dy,sx,sy,"CAN",(unsigned char*)Consolas7x13) == 1 ){
                    thread_sleep_for(100);
                    ret=0;
                    namecode_data[idnum]=org_id;    // 値を戻す
                    break;
                }
                //if(Sw2==0){ // もういちど
                if( sw_1button(dx,dy,sx,sy+60,"RET",(unsigned char*)Consolas7x13) == 1 ){
                    thread_sleep_for(100);
                    sensed_id=0;
                    ret=2;
                    break;
                }
                //if(Sw3==0){ // 変更決定
                if( sw_1button(dx,dy,sx,sy+120,"ENT",(unsigned char*)Consolas7x13) == 1 ){
                    thread_sleep_for(100);
                    ret=1;
                    namecode_data[idnum]=sensed_id;
                    break;
                }
                thread_sleep_for(50);
            }
            while( TFT.getPixel(pp) );  // getPixelの返り値は(TOUCH==YES)の判定結果。手を放すまで待つ。
            //while( (Sw1==0)||(Sw2==0)||(Sw3==0) );
        }
        if(ret!=2)break;
        thread_sleep_for(500);
    }
    if(ret==1){ // 誕生日の設定
        TFT.fillrect(sx,sy    ,sx+dx,sy+dy    ,Black);
        TFT.fillrect(sx,sy+60 ,sx+dx,sy+dy+60 ,Black);
        TFT.fillrect(sx,sy+120,sx+dx,sy+dy+120,Black);
        TFT.rect(sx,sy    ,sx+dx,sy+dy    ,Yellow);
        TFT.rect(sx,sy+60 ,sx+dx,sy+dy+60 ,Yellow);
        TFT.rect(sx,sy+120,sx+dx,sy+dy+120,Yellow);
 
        TFT.background(Black);
        TFT.foreground(Green);
        TFT.set_font( (unsigned char *)Prototype17x20 );
        TFT.locate(0,0);
        TFT.fillrect(0,0,319,20,Black);     // 1行分消す
        TFT.printf("Set Birthday!");

        TFT.set_font( (unsigned char *)Prototype20x25 );
        TFT.locate(0,20);TFT.printf("%02d:%07d",idnum,namecode_data[idnum]);
        TFT.set_font( (unsigned char *)Prototype20x25 );
        TFT.foreground(Cyan);
        TFT.locate(0,45);TFT.printf("%02X/%02X/%02X",(retidate_data[idnum]>>16)&0xFF,(retidate_data[idnum]>>8)&0xFF,(retidate_data[idnum]&0xFF));

        int reti_yy;
        TFT.foreground(White);
        reti_yy = sel_num(0,45,0,99,(unsigned char *)Prototype20x25) + 60;//60歳定年だから60を足す。
        if(reti_yy>99)reti_yy -= 100;
        retidate_data[idnum]  = rtc.Dec2RtcData( reti_yy ) << 16;
        thread_sleep_for(200);
        retidate_data[idnum] += rtc.Dec2RtcData( sel_num(20*3,45,1,12,(unsigned char *)Prototype20x25) ) << 8;
        thread_sleep_for(200);
        retidate_data[idnum] += rtc.Dec2RtcData( sel_num(20*6,45,1,31,(unsigned char *)Prototype20x25) );
        thread_sleep_for(200);
    }
    while( (Sw1==0)||(Sw2==0)||(Sw3==0) );  // すべてのボタンが離されるまで待つ
    return ret;
}

void ope_datawrite(int idnum){
    int addr;
    addr=idnum*6;
    eeprom.write_enable();
    for(int i=0;i<6;i++)eeprom.erase(addr+i);
    eeprom.write(addr  ,(unsigned short)((namecode_data[idnum]>>16) & 0xff));
    eeprom.write(addr+1,(unsigned short)((namecode_data[idnum]>> 8) & 0xff));
    eeprom.write(addr+2,(unsigned short)( namecode_data[idnum]      & 0xff));
    eeprom.write(addr+3,(unsigned short)((retidate_data[idnum]>>16) & 0xff));
    eeprom.write(addr+4,(unsigned short)((retidate_data[idnum]>> 8) & 0xff));
    eeprom.write(addr+5,(unsigned short)( retidate_data[idnum]      & 0xff));
    eeprom.write_disable();
}
    
void data_operation(char *rtc_retire){
//    while(Sw3 == 0);    // 離すまでまつ。
    int sx=220,sy=30,dx,dy;
    

    thread_sleep_for(100);
    TFT.background(Black);
    TFT.foreground(White);
    TFT.cls();
    TFT.set_font( (unsigned char *)Prototype17x20 );
    TFT.locate(0,0);
    TFT.printf("Operation..");
    TFT.set_font( (unsigned char *)Consolas7x13 );
    for(int i=0;i<MAX_MEMORY;i++){
        ope_dispcode(i,White,Black,0);
    }

    // ボタン作成(右サイド)
    dx=90;
    dy=40;
    TFT.rect(sx,sy,sx+dx,sy+dy,Yellow);
    sy += 60;
    TFT.rect(sx,sy,sx+dx,sy+dy,Yellow);
    sy += 60;
    TFT.rect(sx,sy,sx+dx,sy+dy,Yellow);
    

    int idnum=0;
    ope_dispcode(idnum,White,Black,1);
    while(1){
        sy=30;
        //if(Sw1==0){ // ↑
        if( sw_1button(dx,dy,sx,sy,"UP",(unsigned char*)Consolas7x13) == 1 ){
            //while(Sw1==0);
            thread_sleep_for(100);
            idnum--;if(idnum<0)idnum=0;
            ope_dispcode(idnum+1,White,Black,0);
            ope_dispcode(idnum  ,White,Black,1);
        }
        //if(Sw2==0){ // ↓
        if( sw_1button(dx,dy,sx,sy+60,"DN",(unsigned char*)Consolas7x13) == 1 ){
            //while(Sw2==0);
            thread_sleep_for(100);
            idnum++;if(idnum>(MAX_MEMORY-1))idnum=MAX_MEMORY-1;
            ope_dispcode(idnum-1,White,Black,0);
            ope_dispcode(idnum  ,White,Black,1);
        }
        //if(Sw3==0){ // 決定
        if( sw_1button(dx,dy,sx,sy+120,"ENT",(unsigned char*)Consolas7x13) == 1 ){
            //while(Sw3==0);
            thread_sleep_for(100);
            int chgmde1=ope_changecode(idnum);
            if( chgmde1 == 1){
                ope_datawrite(idnum);
            } else if(chgmde1==3){
                curr_data_offset=idnum;
                set_rtc_retire(rtc_retire,0);  // 退職日を再セット 
            }
            break;
        }
        TFT.foreground(Yellow);
        TFT.background(Black);
        TFT.set_font( (unsigned char *)Prototype20x25 );
        if(namecode_data[idnum]<10000000){
            TFT.locate(0,20);TFT.printf("%02d",idnum);
            TFT.foreground(Green);TFT.printf(":");
            TFT.foreground(Cyan);TFT.printf("%07d",namecode_data[idnum]);
            TFT.set_font( (unsigned char *)Prototype17x20 );
            TFT.locate(0,45);
            TFT.foreground(Green);TFT.printf("[");
            TFT.foreground(Yellow);TFT.printf("%06X",retidate_data[idnum]);
            TFT.foreground(Green);TFT.printf("]");
        } else {
            TFT.locate(0,20);TFT.printf("%02d",idnum);
            TFT.foreground(Green);TFT.printf(":");
            TFT.foreground(Cyan);TFT.printf("-------");
            TFT.set_font( (unsigned char *)Prototype17x20 );
            TFT.locate(0,45);
            TFT.foreground(Green);TFT.printf("[");
            TFT.foreground(Yellow);TFT.printf("%06X",retidate_data[idnum]);
            TFT.foreground(Green);TFT.printf("]");

        }
        TFT.set_font( (unsigned char *)Consolas7x13 );
        thread_sleep_for(50);
    }
    //while( (Sw3==0)||(Sw2==0)||(Sw1==0) );
}

