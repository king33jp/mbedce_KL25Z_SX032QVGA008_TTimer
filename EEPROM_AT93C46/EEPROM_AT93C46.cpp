#include "EEPROM_AT93C46.h"
#include "mbed.h"

EEPROM_AT93C46::EEPROM_AT93C46(PinName eecs,PinName eeck,PinName eedi,PinName eedo): _eecs(eecs),_eeck(eeck),_eedi(eedi),_eedo(eedo){
    _eecs=0;     // active is Hi
    _eeck=0;
};

void EEPROM_AT93C46::send(char data){
    signed char i=7;
    _eedi=1;
    _eecs=1;     // fall is in function
    wait_us(1);
    _eeck=1;
    wait_us(1);
    _eeck=0;
    while(i>=0){
        _eedi = (data>>i)&0x01;
        i--;
        wait_us(1);
        _eeck=1;
        wait_us(1);
        _eeck=0;
    }
}

unsigned short EEPROM_AT93C46::read(char addr){
    unsigned short data=0;
    signed char i=15;
   send(EE_READ|addr);
    wait_us(1);
    for(i=15;i>=0;i--){
        _eeck=1;
        wait_us(1);
        _eeck=0;
        data = data | (_eedo<<i);
        wait_us(1);
    }
    _eecs=0;
    return data;
}

void EEPROM_AT93C46::write_enable(){
    send(EE_EWEN);
    wait_us(1);
    _eecs=0;
}

void EEPROM_AT93C46::write_disable(){
    send(EE_EWDS);
    wait_us(1);
    _eecs=0;
}
void EEPROM_AT93C46::erase(char addr){
    send(EE_ERASE|addr);
    wait_us(1);
    _eecs=0;
/** wait busy flag clear */
    wait_us(1);     // tcs > 250ns @2.7V
    _eecs=1;
    wait_us(1);     // tsv < 250ns @2.7V
    while(_eedo==0); // 0.1ms < twp < 10ms 
    _eecs=0;
}
void EEPROM_AT93C46::write(char addr,unsigned short data){
    signed char i=15;
    send(EE_WRITE|addr);
    for(i=15;i>=0;i--){
        _eedi = (int)( (data>>i)&0x0001 );
        wait_us(1);
        _eeck=1;
        wait_us(1);
        _eeck=0;
    }
    _eecs=0;
/** wait busy flag clear */
    wait_us(1);     // tcs > 250ns @2.7V
    _eecs=1;
    wait_us(1);     // tsv < 250ns @2.7V
    while(_eedo==0); // 0.1ms < twp < 10ms 
    _eecs=0;
}