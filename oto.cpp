#include "mbed.h"
#include "oto.h"

extern PwmOut Buzz;

void doremi(){
    Buzz = 0.5;
    Buzz.period_us( DO_1 );thread_sleep_for(200);
    Buzz.period_us( RE_1 );thread_sleep_for(200);
    Buzz.period_us( MI_1 );thread_sleep_for(200);
    Buzz.period_us( FA_1 );thread_sleep_for(200);
    Buzz.period_us( SO_1 );thread_sleep_for(200);
    Buzz.period_us( RA_1 );thread_sleep_for(200);
#if 0
    Buzz.period_us( SI_1 );thread_sleep_for(200);
    Buzz.period_us( DO_2 );thread_sleep_for(200);
    Buzz.period_us( RE_2 );thread_sleep_for(200);
    Buzz.period_us( MI_2 );thread_sleep_for(200);
    Buzz.period_us( FA_2 );thread_sleep_for(200);
    Buzz.period_us( SO_2 );thread_sleep_for(200);
    Buzz.period_us( RA_2 );thread_sleep_for(200);
#endif
    Buzz = 0;
}

void greensleeves(){
//#define UT 0.5
#define UT 500  // 0.5=500ms
    Buzz=0.5;
    Buzz.period_us( RA_1 );thread_sleep_for(UT*0.5);
    Buzz.period_us( DO_2 );thread_sleep_for(UT    );
    Buzz.period_us( RE_2 );thread_sleep_for(UT*0.5);
    Buzz.period_us( MI_2 );thread_sleep_for(UT*0.75);
    Buzz.period_us( FA_2 );thread_sleep_for(UT*0.25);
    Buzz.period_us( MI_2 );thread_sleep_for(UT*0.5);
    Buzz.period_us( RE_2 );thread_sleep_for(UT);
    Buzz.period_us( SI_1 );thread_sleep_for(UT*0.5);

    Buzz.period_us( SO_1 );thread_sleep_for(UT*1.5);
    Buzz.period_us( RA_1 );thread_sleep_for(UT*0.5);
    Buzz.period_us( SI_1 );thread_sleep_for(UT*0.5);
    Buzz.period_us( DO_2 );thread_sleep_for(UT);
    Buzz.period_us( RA_1 );thread_sleep_for(UT*0.5);
    Buzz.period_us( RA_1 );thread_sleep_for(UT*0.75);
    Buzz.period_us( SO_1S);thread_sleep_for(UT*0.25);
    Buzz.period_us( RA_1 );thread_sleep_for(UT*0.5);
    Buzz.period_us( SI_1 );thread_sleep_for(UT);
    Buzz.period_us( SO_1S);thread_sleep_for(UT*0.5);
    Buzz.period_us( MI_1 );thread_sleep_for(UT);

//    Buzz.period_us(  );thread_sleep_for(UT);
//    Buzz.period_us(  );thread_sleep_for(UT);
//    Buzz.period_us(  );thread_sleep_for(UT);
//    Buzz.period_us(  );thread_sleep_for(UT);
   Buzz = 0;
}

void Buzzer_1(){
    Buzz = 0.5;
    for(int i=6;i>0;i--){
        Buzz.period_ms(i);
        thread_sleep_for(50);
    }
    Buzz = 0;
}

void Buzzer_pipi(int num){
#if 1
    Buzz.period_ms(1);
    for(int i=0;i<num;i++){
        Buzz = 0.5;thread_sleep_for(50);
        Buzz = 0.0;thread_sleep_for(20);
    }
#endif
}

void Buzzer_bubu(int num){
#if 1
    Buzz.period_ms(6);
    for(int i=0;i<num;i++){
        Buzz = 0.5;thread_sleep_for(50);
        Buzz = 0.0;thread_sleep_for(20);
    }
#endif
}
