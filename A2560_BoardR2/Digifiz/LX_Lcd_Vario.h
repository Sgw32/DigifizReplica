#ifndef LX_Lcd_Vario_h
#define LX_Lcd_Vario_h

#include "Arduino.h"

//Continue and Last Bit
#define CONTINUE 128			// Command continuation Bit
#define LAST 0					// Last Command

//MODE SET
#define MODESET 64				// Mode Set Command

#define MODE_NORMAL 0			// Display Mode
#define MODE_POWERSAVING 16

#define DISPLAY_DISABLED 0		// Display On/Off
#define DISPLAY_ENABLED 8

#define BIAS_THIRD 0			// BIAS Mode
#define BIAS_HALF 4

#define DRIVE_STATIC 1			// Drive Mode
#define DRIVE_2 2
#define DRIVE_3 3
#define DRIVE_4 0


//BLINK
#define BLINK  112				// Blink Command

#define BLINKING_NORMAL 0		// Blink Mode
#define BLINKING_ALTERNATION 4

#define BLINK_FREQUENCY_OFF 0	// Blink Frequency
#define BLINK_FREQUENCY2 1
#define BLINK_FREQUENCY1 2
#define BLINK_FREQUENCY05 3


//LOADDATAPOINTER
#define LOADDATAPOINTER  0x00	// Data pointer start address


//BANK SELECT
#define BANKSELECT 120			// Bank Select Command

#define BANKSELECT_O1_RAM0 0
#define BANKSELECT_O1_RAM2 2

#define BANKSELECT_O2_RAM0 0
#define BANKSELECT_O2_RAM2 1


// Device Select, not really sure how to implement best, 
//because you can combine these commands to address 8 PCF
#define DEVICE_SELECT 96		// Device Select Command
#define DEVICE_SELECT_A2 4
#define DEVICE_SELECT_A1 2
#define DEVICE_SELECT_A0 1		// A2,A1,A0 = B111

class LX_Lcd_Vario
{
    public:
        LX_Lcd_Vario();
        void begin();
        void addPCF(uint8_t pcfAddr, uint8_t modeSet, uint8_t devSel, uint8_t blink, uint8_t bankSel);
        void init();
        void clear();
        void fire();
	void fireDigifiz();
        void addInd(float val);
        void show();
        void showOLD();
        void addScr(uint8_t val);
        void addSym(uint8_t val);
        void upperNum(int16_t val);
        void addNumber(uint8_t pos, uint8_t val);
        void setDigifizBuffer(uint8_t bank, uint8_t val);
        void setDigifizBufferMasked(uint8_t bank, uint8_t val, uint8_t mask);
        void setDigifizBufferSegment(uint8_t bank, uint8_t segment, uint8_t val);
    private:
        uint8_t settings[16][5];
        uint8_t PCFcount = 0;
        uint8_t buffer[19][3];  //max 19 needed, course its only 19 different Pointer
	uint8_t digifizBuffer[40];
        uint8_t oldBuffer[19][3];
        uint8_t bufferCount = 0;
        uint8_t oldBufferCount = 0;
        void addToBuffer(uint8_t *val);
};

#endif