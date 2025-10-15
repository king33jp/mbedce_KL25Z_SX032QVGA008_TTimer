/* Copyright (c) 2010-2011 mbed.org, MIT License
* Copyright (c) 2014 Antonio Quevedo, UNICAMP
*
* Permission is hereby granted, free of charge, to any person obtaining a copy of this software
* and associated documentation files (the "Software"), to deal in the Software without
* restriction, including without limitation the rights to use, copy, modify, merge, publish,
* distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the
* Software is furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all copies or
* substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING
* BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
* NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
* DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#ifndef MMA8451Q_H
#define MMA8451Q_H

#include "mbed.h"

/**
* MMA8451Q accelerometer example, 8-bit samples
*
* @code
* #include "mbed.h"
* #include "MMA8451Q.h"
* 
* int main(void) {
* 
* MMA8451Q acc(PTE25, PTE24);
* PwmOut rled(LED_RED);
* PwmOut gled(LED_GREEN);
* PwmOut bled(LED_BLUE);
* uint8_t data[3];
*
*     while (true) {
          acc.getAccAllAxis(data);       
*         rled = 1.0 - abs(data[0]/128);
*         gled = 1.0 - abs(data[0]/128);
*         bled = 1.0 - abs(data[0]/128);
*         wait(0.4);
*     }
* }
* @endcode
*/
class MMA8451Q
{
public:
  /**
  * MMA8451Q constructor
  *
  * @param sda SDA pin
  * @param sdl SCL pin
  */
  MMA8451Q(PinName sda, PinName scl);

  /**
  * MMA8451Q destructor
  */
  ~MMA8451Q();

  /**
   * Get XYZ axis acceleration, 8-bits
   *
   * @param res array where acceleration data will be stored
   */
  void getAccAllAxis(int16_t * res);

private:
  I2C m_i2c;
  void readRegs(int addr, uint8_t * data, int len);
  void writeRegs(uint8_t * data, int len);

};

#endif