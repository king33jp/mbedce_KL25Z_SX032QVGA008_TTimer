/** AT93C46
 * AT93C46A x16 mode(7pin OC mean NC ,maybe)
 * AT93C46E x16 mode(7pin NC)
 * AT93C46D x16 mode(7pin ORG,H=x16,L=x8)
 * Cap=1Kbits 64words @ x16
 * SB(start bit)=1bit OP-code=2bits Addr=6bits(64word)
 * controll use gpib port(Outoutx3,Inputx1)
 * 
 */
#ifndef MBED_EEPROM_AT93C46_H
#define MBED_EEPROM_AT93C46_H
 
#include "mbed.h"

/** define
 * Command code define
 * 
 */
#define EE_READ  0x80   // 10 xxxxxx(A5-A0)
#define EE_WRITE 0x40   // 01 xxxxxx(A5-A0)
#define EE_EWEN  0x3F   // 00 11XXXX(X is DONT CARE)
#define EE_EWDS  0x00   // 00 00XXXX(X is DONT CARE)
#define EE_ERASE 0xC0   // 11 xxxxxx(A5-A0)

/** EEPROM controll class
 *
 * Example:
 * @code
 * // AT93C46 GPIO use
 
   #include "mbed.h"
   #include "EEPROM_AT93C46.h"
 
   EEPROM_AT93C46 eeprom(PTE20,PTE21,PTE22,PTE23);

   int main() {
 
    eeprom.write_enable();
    eeprom.erase(0);
    eeprom.write(0,0xABCD);
    eeprom.write_disable();
    for(char jj=0;jj<64++){
        printf("%04X(%d,",eeprom.read(jj),jj);
    }
   }
 * @endcode
 */
class EEPROM_AT93C46 {
    public:

  /** Create a EEPROM_AT93C46 object connected to four pins
   *
   * @param eccs pin connected to CS of AT93C46
   * @param eeck pin connected to CLK of AT93C46
   * @param eedi pin connected to DI of AT93C46 
   * @param eedo pin connected to DO of AT93C46
   */ 
    EEPROM_AT93C46(PinName eecs,PinName eeck,PinName eedi,PinName eedo);

/** data=8bit
 * I send to AT93C46 with 9bit data(start bit added )
 * bit send with wait_us(1),so 
 * @param addr Sending data to AT93C46
 */
    void send(char data);

/** Data read ftom AT93C46
 * send( OP-code|addr )
 * @param addr address(6bit)
 */
    unsigned short read(char addr);

/** Data read ftom AT93C46
 * send( OP-code|addr )
 * @param addr address(6bit)
 * @return data( unsigned short 1 word )
 */
    void write_enable();
/** Do write-enable to AT93C46
 * send( OP-code )
 * @param addr address(6bit)
 */
    void write_disable();
/** Do write-disable to AT93C46
 * send( OP-code )
 * @param addr address(6bit)
 */
    void erase(char addr);

/** Data erase to AT93C46
 * send( OP-code|addr )
 * @param addr address(6bit)
 * @param data write data(16bit=short)
 */
    void write(char addr,unsigned short data);
    
    private:
    DigitalOut _eecs; //AT93C46 CS ,active H
    DigitalOut _eeck; //AT93C46 CLK,rise-edge is write,fall-edge is read
    DigitalOut _eedi; //AT93C46 DataIn (mbed is output)
    DigitalIn  _eedo; //AT93C46 DataOut(mbed is input)
};
#endif


