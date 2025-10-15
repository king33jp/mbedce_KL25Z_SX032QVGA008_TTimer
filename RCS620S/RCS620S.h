/*
 * RC-S620/S sample library for Arduino
 *
 * Copyright 2010 Sony Corporation
 *
 * Rewrite for mbed
 *
 * modified by SWITCHSCIENCE
 *   https://github.com/SWITCHSCIENCE/mbed-FeliCa
 * 
 * modified by Suga
 */

/** @file
 * @brief Sony FeliCa reader/writer (RC-S620/S) library for mbed
 */

#include "mbed.h"
#include <inttypes.h>

#ifndef RCS620S_H_
#define RCS620S_H_

/* --------------------------------
 * Constant
 * -------------------------------- */

#define RCS620S_MAX_CARD_RESPONSE_LEN    254
#define RCS620S_MAX_RW_RESPONSE_LEN      265

/* --------------------------------
 * Class Declaration
 * -------------------------------- */

class RCS620S
{
public:
    RCS620S(PinName p_tx, PinName p_rx);

    int initDevice(void);
    int polling(uint16_t systemCode = 0xffff);
    int cardCommand(
        const uint8_t* command,
        uint8_t commandLen,
        uint8_t response[RCS620S_MAX_CARD_RESPONSE_LEN],
        uint8_t* responseLen);
    int rfOff(void);

    int push(
        const uint8_t* data,
        uint8_t dataLen);

private:
    int rwCommand(
        const uint8_t* command,
        uint16_t commandLen,
        uint8_t response[RCS620S_MAX_RW_RESPONSE_LEN],
        uint16_t* responseLen);
    void cancel(void);
    uint8_t calcDCS(
        const uint8_t* data,
        uint16_t len);

    void writeSerial(
        const uint8_t* data,
        uint16_t len);
    int readSerial(
        uint8_t* data,
        uint16_t len);
    void flushSerial(void);

    int checkTimeout(time_t t0);

public:
    time_t timeout;
    uint8_t idm[8];
    uint8_t pmm[8];
    
private:
    BufferedSerial serial;
};

#endif /* !RCS620S_H_ */
