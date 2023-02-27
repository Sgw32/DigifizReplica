/******************************************************************************
 * @file STLED316S.h
 * @brief Arduino Library for STLED316S LED controller with keyscan (Header)
 * @author David Leval
 * @version 1.1.0
 * @date 16/09/2021
 * 
 * Resources:
 * Uses SPI.h for SPI operation
 *
 * The STLED316S is a LED controller and driver that interface microprocessors 
 * to 7-segments LED displays through a serial 3-wire interface (compatible SPI)\n
 * Features :\n
 * - LED driver with 14 outputs (8 segments/6 digits common-anode)
 * - A single LED digit output (DIG1_LED) can be used to drive up to 8 discrete LEDs
 * - 8-step dimming circuit to control brightness of individual LEDs for LED digit
 * - Power 5VDC / can operate with 3.3 V interface voltages
 * \n
 * 
 * Release :
 *      - v1.0.0 (08/04/2020) : Initial version
 *      - v1.0.1 (02/06/2021) : Addition of a private variable to save the state of the LEDs 
 *                              (Contribution of Giovani Luis Franco)
 *      - v1.0.2 (17/06/2021) : Fix vtable linker error (default implementation of virtual functions )
 *      - v1.0.3 (13/09/2021) : Modify default correspondence between the driver outputs and the display segments
 *                              (STLED316S Display Board compatibility)
 *      - v1.1.0 (16/09/2021) : Add Float display value and signed decimal value
 *                              Addition of a keyscan state reading function
 * 
 * STLED316S library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * <http://www.gnu.org/licenses/>.
 * 
 ******************************************************************************/

#ifndef __STLED316S_h
#define __STLED316S_h

/******************************************************************************/
/* Include                                                                    */
/******************************************************************************/

#ifdef ARDUINO
  #if (ARDUINO >= 100)
  #include <Arduino.h>
  #else
  #include <WProgram.h>
  #include <pins_arduino.h>
#endif

#endif
#include <stdint.h>
#include <SPI.h>  // Arduino SPI library

/******************************************************************************/
/* Platform specificity                                                       */
/******************************************************************************/
/*#if defined(__AVR_ATmega328P__) || defined(__AVR_ATmega168__) //Arduino Uno
    //
#elif defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega16U4__) //Arduino Leonardo
    //
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__) //Arduino Mega
    //
#elif defined(ARDUINO_SAM_DUE) || defined(ARDUINO_SAMD_ZERO) //Arduino Due or Zero
    //
#elif defined(ARDUINO_ARCH_STM32F0) || defined(ARDUINO_ARCH_STM32F1) || defined(ARDUINO_ARCH_STM32F3) || defined(ARDUINO_ARCH_STM32F4) || defined(ARDUINO_ARCH_STM32L4) //STM32
    //
#elif defined(ARDUINO_ARCH_ESP32)
    //
#elif defined(ARDUINO_ARCH_ESP8266)
    //
#elif defined(ARDUINO_ARCH_RP2040)
    //
#else
  #error Unsupported Hardware.
#endif*/


/******************************************************************************/
/* Configuration                                                              */
/******************************************************************************/
#define STLED316S_DEFAULT_BRIGHTNESS 	2      //!< Brightness Default (0 to 7)

/******************************************************************************/
/* Display segment definition                                                 */
/******************************************************************************/
#define SEG1	0x01
#define SEG2	0x02
#define SEG3	0x04
#define SEG4	0x08
#define SEG5	0x10
#define SEG6	0x20
#define SEG7	0x40
#define SEG8	0x80


/******************************************************************************/
/* Typedef                                                                    */
/******************************************************************************/
typedef enum {
    DIGITall = 0,
    DIGITn1 = 1,
    DIGITn2 = 2,
    DIGITn3 = 3,
    DIGITn4 = 4,
    DIGITn5 = 5,
    DIGITn6 = 6
} DIGITnum_t;

typedef enum {
    LEDall = 0x00,
    LEDn1 = 0x01,
    LEDn2 = 0x02,
    LEDn3 = 0x04,
    LEDn4 = 0x08,
    LEDn5 = 0x10,
    LEDn6 = 0x20,
    LEDn7 = 0x40,
    LEDn8 = 0x80
} LEDnum_t;


/******************************************************************************/
/* Class declaration                                                          */
/******************************************************************************/
class STLED316S_Common
{
    private:
        uint8_t _dispDataBuffer[7]; //!< Memory buffer used for display
        uint8_t _digitTable[17] = {0x3F,0x06,0x5B,0x4F,0x66,0x6D,0x7D,0x07,0x7F,0x6F,0x77,0x7C,0x39,0x5E,0x79,0x71,0x40};
        uint8_t _digitBrightness[3];
        uint8_t _LEDbrightness[4];
        uint8_t _digDP;
        uint8_t _LEDstate;  //!< Memory of LEDs state
        uint8_t _nbrOfDigit;
        const uint8_t CHAR_MINUS = 0x10;
        uint32_t pow10(uint8_t factor);
        uint8_t dispNumberMinusSign(uint32_t nbr, uint8_t minPosition);
        void dispNumber(uint8_t digitPtr, uint32_t nbr, uint8_t minNbrOfDigit);

    public:
        //STLED316S Register 
        static const uint8_t STLED316S_DISP_ON_CMD = 0x0D;
        static const uint8_t STLED316S_DISP_OFF_CMD = 0x0E;
        static const uint8_t STLED316S_DATA_WR = 0x00;
        static const uint8_t STLED316S_DATA_RD = 0x40;
        static const uint8_t STLED316S_ADDR_INC = 0x00;
        static const uint8_t STLED316S_ADDR_FIXED = 0x20;
        static const uint8_t STLED316S_CONF1_PAGE = 0x10;
        static const uint8_t STLED316S_DIG_PAGE = 0x00;
        static const uint8_t STLED316S_LED_PAGE = 0x08;
        static const uint8_t STLED316S_BRT_LED_PAGE = 0x18;
        static const uint8_t STLED316S_READ_PAGE = 0x08;
        static const uint8_t STLED316S_ADDR_LED_DATA = 0x00;
        static const uint8_t STLED316S_ADDR_KEY_DATA1 = 0x01;
        static const uint8_t STLED316S_ADDR_KEY_DATA2 = 0x02;
        static const uint8_t STLED316S_CONF_BRT_CONSTANT = 0x18;
        static const uint8_t STLED316S_CONF_BRT_VARIABLE = 0x00;

        STLED316S_Common(uint8_t nbrOfDigit);
        void begin(void);
        void begin(uint8_t digA, uint8_t digB, uint8_t digC, uint8_t digD, uint8_t digE, uint8_t digF, uint8_t digG, uint8_t digDP);
        void displayON(void);
        void displayOFF(void);
        void setBrightness(DIGITnum_t DIGITnum, uint8_t brightness);
        void clearDisplay(void);
        void dispRAW(DIGITnum_t DIGITnum, uint8_t raw);
        void dispRAW(uint8_t *raw);
        void dispUdec(uint32_t nbr);
        void dispHex(uint32_t data);
        void dispFloat(float nbr, uint8_t decimal);
        void dispDec(int32_t nbr);
        void setDP(DIGITnum_t DIGITnum, uint8_t state);
        void setBrightnessLED(LEDnum_t LEDnum, uint8_t brightness);
        void setLED(LEDnum_t LEDnum, bool state);
        uint16_t readKeyScan(void);

        virtual void writeData(uint8_t *data, uint8_t lenght);
        virtual uint8_t readData(uint8_t address);
};

class STLED316S_SPI : public STLED316S_Common
{
    private :
        uint8_t _STBpin = 10; 		//!< Chip Select Pin (STB) : Default 10
        uint8_t SwapBit(uint8_t byte);

    public:
        STLED316S_SPI(uint8_t nbrOfDigit);
        STLED316S_SPI(uint8_t nbrOfDigit, uint8_t STBpin);
        void writeData(uint8_t *data, uint8_t lenght) override;
};

class STLED316S : public STLED316S_Common
{
    private :
        uint8_t _STBpin;
        uint8_t _CLKpin;
        uint8_t _DATApin;
        void sendSW_SPI(uint8_t data);

    public:
        STLED316S(uint8_t nbrOfDigit, uint8_t STBpin, uint8_t CLKpin, uint8_t DATApin);
        void writeData(uint8_t *data, uint8_t lenght) override;
        uint8_t readData(uint8_t address) override;
};

#endif //__STLED316S_h
