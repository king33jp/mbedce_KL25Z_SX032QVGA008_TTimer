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

#include "MMA8451Q.h"

#define ACCEL_I2C_ADDRESS 0x1D<<1
#define REG_STATUS        0x00
#define REG_XYZ_DATA_CFG  0x0E
#define REG_WHO_AM_I      0x0D
#define REG_CTRL_REG1     0x2A
#define REG_CTRL_REG2     0x2B
#define REG_CTRL_REG3     0x2C
#define REG_CTRL_REG4     0x2D
#define REG_CTRL_REG5     0x2E
#define REG_OUT_X_MSB     0x01
#define REG_OUT_Y_MSB     0x03
#define REG_OUT_Z_MSB     0x05

MMA8451Q::MMA8451Q(PinName sda, PinName scl) : m_i2c(sda, scl) {
    uint8_t data[2] = {REG_CTRL_REG1, 0x00}; // Puts acc in standby for configuring
    writeRegs(data, 2);
    data[0] = REG_XYZ_DATA_CFG; // Writing 00 turns off high-pass filter and sets full scale range to 2g
    // data[1] = 0x01; for 4g
    // data[1] = 0x02; for 8g
    writeRegs(data, 2);
    data[0] = REG_CTRL_REG2;
    data[1] = 0x00; // Disable self-test, software reset and auto-sleep; operates in normal mode
    writeRegs(data, 2);
    data[0] = REG_CTRL_REG3; // Interrupt polarity low, push-pull output
    writeRegs(data, 2);
    data[0] = REG_CTRL_REG4;
    data[1] = 0x01; // Enables interrupt for data Ready
    writeRegs(data, 2);
    data[0] = REG_CTRL_REG5;
    writeRegs(data, 2); // Routes Data Ready interrupt to INT1
    data[0] = REG_CTRL_REG1; 
    data[1] = 0x09; // Data rate is 800Hz
    writeRegs(data, 2);
}

MMA8451Q::~MMA8451Q() { }

void MMA8451Q::getAccAllAxis(int16_t * res) {
    uint8_t temp[6];
    readRegs(REG_OUT_X_MSB, temp, 6);
    res[0] = (temp[0] * 256) + temp[1];
    res[1] = (temp[2] * 256) + temp[3];
    res[2] = (temp[4] * 256) + temp[5];
}

void MMA8451Q::readRegs(int addr, uint8_t * data, int len) {
    char t[1] = {addr};
    m_i2c.write(ACCEL_I2C_ADDRESS, t, 1, true);
    m_i2c.read(ACCEL_I2C_ADDRESS, (char *)data, len);
}

void MMA8451Q::writeRegs(uint8_t * data, int len) {
    m_i2c.write(ACCEL_I2C_ADDRESS, (char *)data, len);
}