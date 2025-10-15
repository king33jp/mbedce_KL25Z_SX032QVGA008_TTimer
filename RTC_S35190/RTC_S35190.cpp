#include "RTC_S35190.h"
#include "mbed.h"

RTC_S35190::RTC_S35190(PinName rtsio,PinName rtclk,PinName rtcs): _rtsio(rtsio),_rtclk(rtclk),_rtcs(rtcs){
    _rtcs = 0;
    _rtclk = 1;
}
    
void RTC_S35190::portinit(){
    _rtcs=0;             // RTCS = 0;
    _rtclk=1;            // RTCLK = 1;
}

void RTC_S35190::send_cmd(char comm){
    signed char c;
    _rtsio.output();              // RTSIO=出力に変更
    _rtcs = 1;              // RTCS=1 CS Active データ入出力後に L にする。
                                // 必ずコマンドの後にデータがあるので。
    for(c=7;c>=0;c--){
        _rtclk = 0;        // RTCLK = 0;
        _rtsio = (comm>>c)&0x01;
        wait_us(10);
        _rtclk = 1;         // RTCLK = 1;
        wait_us(10);
    }
}

//  Statusレジスタ1でPOC|BLDの値を返す
char RTC_S35190::get_stat(){
    char data,tmp;
    signed char c;
    data = 0;
    send_cmd( RtcStat1 );

    _rtsio.input();;            // 入力に変更
    for(c=0;c<8;c++){
        _rtclk = 0;        // RTCLK = 0;
        wait_us(10);
        _rtclk = 1;         // RTCLK = 1;
        wait_us(10);
        tmp = _rtsio;        // RTSIOはbit1データなので1回シフトしておく
        data |= tmp<<c;
    }
    _rtcs = 0;             // RTCS = 0;
    wait_us(10);
    data &= 0xC0;   // 上位2bitのみ抽出
    return data;
}

void RTC_S35190::reset(){
    char data;
    signed char c;

    send_cmd( RtcStat1 & Reg_W );
    
    data = 0x03;                // 24時間,RESET=1
    _rtsio.output();             // 出力に変更
    for(c=0;c<8;c++){
        _rtclk = 0;        // RTCLK = 0;
        _rtsio = (data>>c)&0x01;
        wait_us(10);
        _rtclk = 1;         // RTCLK = 1;
        wait_us(10);
    }
    _rtcs = 0;             // RTCS = 0;
    wait_us(10);
    _rtsio.input();            // 入力に変更
}
    
void RTC_S35190::wr_freereg(char data){
    signed char c;

    send_cmd( Free_Reg & Reg_W );
    
    _rtsio.output();             // 出力に変更
    for(c=0;c<8;c++){
        _rtclk = 0;        // RTCLK = 0;
        _rtsio = (data>>c)&0x01;
        wait_us(10);
        _rtclk = 1;         // RTCLK = 1;
        wait_us(10);
    }
    _rtcs = 0;             // RTCS = 0;
    wait_us(10);
    _rtsio.input();            // 入力に変更
}
    
char RTC_S35190::rd_freereg(){
    char data;
    signed char c;
    data = 0;
    
    send_cmd( Free_Reg );
    
    _rtsio.input();            // 入力に変更
    for(c=0;c<8;c++){
        _rtclk = 0;        // RTCLK = 0;
        wait_us(10);
        _rtclk = 1;         // RTCLK = 1;
        wait_us(10);
        data |= (_rtsio<<c);
    }
    _rtcs = 0;             // RTCS = 0;
    wait_us(10);
    return data;
}

void RTC_S35190::rd_today(char *rtc_today){
    char a;
    signed char c;

    send_cmd( RtcRegAcc1 );

    _rtsio.input();            // 入力に変更
    for(a=0;a<7;a++){
        rtc_today[a] = 0;       // initialize
        for(c=0;c<8;c++){
            _rtclk = 0;        // RTCLK = 0;
            wait_us(10);
            _rtclk = 1;         // RTCLK = 1;
            wait_us(10);
            rtc_today[a] |= _rtsio<<c;
        }
    }
    _rtcs = 0;             // RTCS = 0;
    wait_us(10);
}

void RTC_S35190::wr_today(char *rtc_today){
    char a;
    signed char c;
    
    send_cmd( RtcRegAcc1 & Reg_W );

    _rtsio.output();             // 出力に変更
    for(a=0;a<7;a++){
        for(c=0;c<8;c++){
            _rtclk = 0;        // RTCLK = 0;
            _rtsio = (rtc_today[a]>>c)&0x01;
            wait_us(10);
            _rtclk = 1;         // RTCLK = 1;
            wait_us(10);
        }
    }
    _rtcs = 0;             // RTCS = 0;
    wait_us(10);
    _rtsio.input();            // 入力に変更
}


void RTC_S35190::rd_now(char *rtc_today){
    char a,data;
    signed char c;

    data = 0;
    
    send_cmd( RtcRegAcc2 );
        
    _rtsio.input();            // 入力に変更
    for(a=4;a<7;a++){
        data = 0;

        for(c=0;c<8;c++){
            _rtclk = 0;        // RTCLK = 0;
            wait_us(10);
            _rtclk = 1;         // RTCLK = 1;
            wait_us(10);
            data |= _rtsio<<c;
        }
        rtc_today[a] = data;
    }
    _rtcs = 0;             // RTCS = 0;
    wait_us(10);
}

char RTC_S35190::Dec2RtcData(char tmp){
    return ((tmp/10)<<4) | ((tmp%10)&0x0F);
}

char RTC_S35190::RtcData2Dec(char tmp){
    return (tmp&0x0F) + (tmp>>4)*10;
}

void RTC_S35190::getDateStr(char *data,char *rtc_today){
    sprintf(data,"20%02d/%2d/%2d",RtcData2Dec(rtc_today[0]),RtcData2Dec(rtc_today[1]),RtcData2Dec(rtc_today[2]));
}

void RTC_S35190::getTimeStr(char *data,char *rtc_today){
    sprintf(data,"%02d:%02d:%02d",(RtcData2Dec(rtc_today[4]&0xBF)),RtcData2Dec(rtc_today[5]),RtcData2Dec(rtc_today[6]));
}

long RTC_S35190::getDaySerial(char *rtc_today){
    long ddd,yyy,dd,mm;
    yyy  = 2000+RtcData2Dec(rtc_today[0]);
    mm = RtcData2Dec(rtc_today[1]);
    dd = RtcData2Dec(rtc_today[2]);
    ddd =  (yyy) * 365;
    ddd += (yyy/4)-(yyy/100)+(yyy/400);
    ddd += 30*mm + (mm+1)*3/5 + dd - 33;
    return ddd;
}

long RTC_S35190::getDaySerial2(char *rtc_today){
    long ddd,yyy,dd,mm;
    int mday[13] = {0,31,28,31,30,31,30,31,31,30,31,30,31};
    yyy  = 2000+RtcData2Dec(rtc_today[0]);
    mm = RtcData2Dec(rtc_today[1]);
    dd = RtcData2Dec(rtc_today[2]);
    ddd =  (yyy) * 365;
    ddd += (yyy/4)-(yyy/100)+(yyy/400);
//    for(int i=1;i<(mm-1);i++){ 2020/1/22 change
    for(int i=1;i<mm;i++){
        ddd += mday[i];
    }
    ddd = ddd + dd;
    return ddd;
}

long RTC_S35190::getTimeSerial(char *rtc_today){
    long ss;
    ss  = RtcData2Dec(rtc_today[4]&0xBF)*60*60;
    ss += RtcData2Dec(rtc_today[5])*60;
    ss += RtcData2Dec(rtc_today[6]);
    return ss;
}