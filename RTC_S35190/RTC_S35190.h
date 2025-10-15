#ifndef MBED_RTC_S35190_H
#define MBED_RTC_S35190_H
 
#include "mbed.h"

#define Free_Reg    0x6F        // 0b01101111
#define RtcRegAcc1  0x65        // 0b01100101
#define RtcRegAcc2  0x67        // 0b01100111
#define RtcStat1    0x61        // 0b01100001
#define RtcStat2    0x63        // 0b01100011
#define Reg_W       0xFE

class RTC_S35190 {
    public:
    RTC_S35190(PinName rtsio,PinName rtclk,PinName rtcs);
    void portinit(void);
    void send_cmd(char comm);
    char get_stat(void);
    void reset(void);
    void wr_freereg(char data);
    char rd_freereg(void);
    void rd_today(char *rtc_today);
    void wr_today(char *rtc_today);
    void rd_now(char *rtc_today);
    char Dec2RtcData(char tmp);
    char RtcData2Dec(char tmp);
    void getDateStr(char *data,char *rtc_today);
    void getTimeStr(char *data,char *rtc_today);
    long getDaySerial(char *rtc_today);
    long getDaySerial2(char *rtc_today);
    long getTimeSerial(char *rtc_today);

private:
    DigitalInOut    _rtsio;
    DigitalOut      _rtclk;
    DigitalOut      _rtcs;
};
#endif
