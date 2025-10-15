/* mbed library for aitendo SX032QVGA08 TFT module with resistive touch panel.
 * 
 * Original library is "SpeedStudioTFT" by 2014 Copyright (c) Seeed Technology Inc.
 * I made by it to the reference.
 *
 * (1) Because there was some mistake in the source, I was corrected.
 * (2) Also, I changed to the source you have to assume FRDM
 * (3) I changed BackLight is PWM controlled.
 */

#ifndef SX032QVGA008_H
#define SX032QVGA008_H

#include "mbed.h"
#include "SPI_TFT_ILI9341ext.h"
#ifdef USE_SDCARD
#include "SDFileSystem.h" // import the SDFileSystem library 
#endif

struct point {
    int x;
    int y;
};

class SX032QVGA008 : public  
#ifdef USE_SDCARD
    SDFileSystem,
#endif
    SPI_TFT_ILI9341ext
{
public:
    /** create a TFT with touch object connected to the pins:
     *
     * @param pin xp resistiv touch x+
     * @param pin xm resistiv touch x-
     * @param pin yp resistiv touch y+
     * @param pin ym resistiv touch y-
     * @param mosi,miso,sclk SPI connection to TFT
     * @param cs pin connected to CS of display
     * @param rs pin connected to RS(DC) of display
     * @param reset pin connected to RESET of display
     * @param backlight pin connected to bkl of display 
     *  ( You must drive N-MOSFET.pin is connected to NMOS gate terminal.)
     * @param sdcard_cs pin connected to CS of sdcard
     * based on my SPI_TFT lib
     */
    SX032QVGA008(PinName xp, PinName xm, PinName yp, PinName ym,
                     PinName mosi, PinName miso, PinName sclk,
                     PinName csTft, PinName dcTft, PinName resTft, PinName blTft,
                     PinName csSd);

    /** Backlight PWM controll
     *
     * @param duty is PWM duty (0 - 1.0)
     * period is 0.01 fixed.
     */
    void setBacklight(float duty);
    
    /** calibrate the touch display
     *
     * User is asked to touch on two points on the screen
     */
    void calibrate(void);

    /** read x and y coord on screen
     *
     * @returns point(x,y)
     */
    bool
    getPixel(point& p);

    /** calculate coord on screen
    *
    * @param a_point point(analog x, analog y)
    * @returns point(pixel x, pixel y)
    *
    */
    point toPixel(point p);

protected:
    PinName _xm;
    PinName _ym;
    PinName _xp;
    PinName _yp;
    PwmOut bl;

    typedef enum { YES, MAYBE, NO } TOUCH;
    TOUCH getTouch(point& p);
    int readTouch(PinName p, PinName m, PinName a, PinName i);

    int x_off,y_off;
    int pp_tx,pp_ty;
};

#endif
