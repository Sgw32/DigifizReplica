/**
 * @file STLED316S.cpp
 * @brief Arduino Library for STLED316S LED controller with keyscan
 * @author David Leval
 * @version 1.1.0
 * @date 16/09/2021
 * 
 * STLED316S library is free software: you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License as
 * published by the Free Software Foundation, either version 3 of the
 * License, or (at your option) any later version.
 * <http://www.gnu.org/licenses/>.
 */
 
/******************************************************************************/
/* Include                                                                    */
/******************************************************************************/
#include "STLED316S.h"

/******************************************************************************/
/* Constructor Functions                                                      */
/******************************************************************************/
STLED316S_Common::STLED316S_Common(uint8_t nbrOfDigit)
{
    _nbrOfDigit = nbrOfDigit;
}

STLED316S_SPI::STLED316S_SPI(uint8_t nbrOfDigit) : STLED316S_Common(nbrOfDigit)
{
    pinMode(_STBpin, OUTPUT);

    //Set chip select to default state
    digitalWrite(_STBpin, HIGH);
    //Initialize the Arduino SPI library
    SPI.begin();
}

STLED316S_SPI::STLED316S_SPI(uint8_t nbrOfDigit, uint8_t STBpin) : STLED316S_Common(nbrOfDigit)
{
    pinMode(STBpin, OUTPUT);
    _STBpin = STBpin;

    //Set chip select to default state
    digitalWrite(_STBpin, HIGH);
    //Initialize the Arduino SPI library
    SPI.begin();
}

STLED316S::STLED316S(uint8_t nbrOfDigit, uint8_t STBpin, uint8_t CLKpin, uint8_t DATApin) : STLED316S_Common(nbrOfDigit)
{
    _STBpin = STBpin;
    _CLKpin = CLKpin;
    _DATApin = DATApin;

    pinMode(_STBpin, OUTPUT);
    pinMode(_CLKpin, OUTPUT);
    pinMode(_DATApin, OUTPUT);
    //Set pin to default state
    digitalWrite(_STBpin, HIGH);
    digitalWrite(_CLKpin, HIGH);
    digitalWrite(_DATApin, LOW);
}

/******************************************************************************/
/* Class STLED316S_SPI Public Functions                                       */
/******************************************************************************/
/**
 * @brief Sending data to the STLED316S with SPI
 * 
 * @param *data : Address of the first byte to send
 * @param lenght : Number of data to send
 */
void STLED316S_SPI::writeData(uint8_t *data, uint8_t lenght)
{
    uint8_t i;
    uint8_t* p_data = data;
    uint8_t SendByte;
    
    SPI.beginTransaction(SPISettings(100000, LSBFIRST, SPI_MODE0)); //Send LSB first
    digitalWrite(_STBpin, LOW);//STB_L();
    delayMicroseconds(2);   
    for(i=0;i<lenght;i++) {
        SendByte = *p_data; 
        //SendByte = SwapBit(*p_data); //if SPI send MSB first
        SPI.transfer(SendByte);
        p_data++;
    } 
    delayMicroseconds(2);
    digitalWrite(_STBpin, HIGH);//STB_H();
    SPI.endTransaction();
}

/******************************************************************************/
/* Class STLED316S_SPI Private Functions                                      */
/******************************************************************************/
/**
 * @brief Endian converter\n
 * Swap every bit in 8-bit data\n
 * b7 b6 b5 b4 b3 b2 b1 b0 -> b0 b1 b2 b3 b4 b5 b6 b7
 * 
 * @param byte : Data to convert
 * @return : Result
 */
uint8_t STLED316S_SPI::SwapBit(uint8_t byte)
{
    return
            (byte & 0x80) >>7 | (byte & 0x40) >>5 | (byte & 0x20) >>3 | (byte & 0x10) >>1 |
            (byte & 0x08) <<1 | (byte & 0x04) <<3 | (byte & 0x02) <<5 | (byte & 0x01) <<7;
}

/******************************************************************************/
/* Class STLED316S Public Functions                                           */
/******************************************************************************/
/**
 * @brief Sending data to the STLED316S without SPI
 * 
 * @param *data : Address of the first byte to send
 * @param lenght : Number of data to send
 */
void STLED316S::writeData(uint8_t *data, uint8_t lenght)
{
    uint8_t i;
    uint8_t* p_data = data;
    uint8_t SendByte;

    digitalWrite(_CLKpin, HIGH);
    digitalWrite(_STBpin, LOW);//STB_L();
    delayMicroseconds(2); 
    for(i=0;i<lenght;i++) {
        SendByte = *p_data; 
        sendSW_SPI(SendByte);
        p_data++;
    }
    delayMicroseconds(2);
    digitalWrite(_STBpin, HIGH);//STB_H();
}

/**
 * @brief Reading data byte from STLED316S
 * 
 * @param address : Address of the data
 * @return data : Data read
 */
uint8_t STLED316S::readData(uint8_t address)
{
    uint8_t j;
    uint8_t SendByte = STLED316S_DATA_RD|STLED316S_READ_PAGE|address;
    uint8_t ReadByte = 0;

    digitalWrite(_CLKpin, HIGH);
    digitalWrite(_STBpin, LOW);//STB_L();
    delayMicroseconds(2); 

    sendSW_SPI(SendByte);

    pinMode(_DATApin, INPUT_PULLUP);
    for(j=0;j<8;j++) {
        digitalWrite(_CLKpin, LOW);
        delayMicroseconds(1);
        digitalWrite(_CLKpin, HIGH);
        if(digitalRead(_DATApin)) ReadByte |= (0x01 << j);
        delayMicroseconds(1);
    }

    delayMicroseconds(2);
    digitalWrite(_STBpin, HIGH);//STB_H();
    pinMode(_DATApin, OUTPUT);

    return ReadByte;
}

/******************************************************************************/
/* Class STLED316S Private Functions                                          */
/******************************************************************************/
/**
 * @brief Simulate SPI transfer with delay
 * 		  (freq. = 500kHz)
 * @param data : byte to send
 */
void STLED316S::sendSW_SPI(uint8_t data)
{
    uint8_t j;
    
    for(j=0;j<8;j++) {
        if(data & (0x01 << j)) digitalWrite(_DATApin, HIGH);
        else digitalWrite(_DATApin, LOW);
        digitalWrite(_CLKpin, LOW);
        delayMicroseconds(1);
        digitalWrite(_CLKpin, HIGH);
        delayMicroseconds(1);
    }
}

/******************************************************************************/
/* Class STLED316S_Common Private Functions                                   */
/******************************************************************************/

/**
 * @brief Calculation of a number to the 10th power : 10^x
 * 
 * @param factor factor
 * @return uint32_t result
 */
uint32_t STLED316S_Common::pow10(uint8_t factor)
{
    uint32_t nbrRet = 1;
    for(uint8_t i=0; i<factor; i++) nbrRet *= 10;
    return nbrRet;
}

/**
 * @brief Displays the minus sign just before the number
 * 
 * @param nbr Display number
 * @param minPosition Minus sign Minimal position
 * @return uint8_t digit pointer
 */
uint8_t STLED316S_Common::dispNumberMinusSign(uint32_t nbr, uint8_t minPosition)
{
    uint8_t minusPtr = _nbrOfDigit;
    uint8_t retPtr = _nbrOfDigit;

    while(minusPtr > 1) {
        if(nbr >= pow10(minusPtr - 1)) {
            minusPtr += 1;
            break;
        }
        minusPtr--;
    }

    if(minusPtr < minPosition) minusPtr = minPosition;
    if(minusPtr >= _nbrOfDigit) { //Overflow or max position
        minusPtr = _nbrOfDigit;
        retPtr--;
    }
    _dispDataBuffer[minusPtr] = _digitTable[CHAR_MINUS];
    return retPtr;
}

/**
 * @brief Display number
 * 
 * @param digitPtr Digit starting position
 * @param nbr Number to display
 * @param minNbrOfDigit Minimum number of digits of the number
 */
void STLED316S_Common::dispNumber(uint8_t digitPtr, uint32_t nbr, uint8_t minNbrOfDigit)
{
    while(digitPtr > 0) {
        uint32_t nbrSel = pow10(digitPtr - 1);

        if(nbr < nbrSel) {
            if(digitPtr <= minNbrOfDigit) {
                _dispDataBuffer[digitPtr] = _digitTable[0];
            } 
        } else {
            _dispDataBuffer[digitPtr] = (_numberMask&(1<<digitPtr)) ? _digitTable[(nbr/nbrSel)%10] : _digitTable[17]; 
        }
        digitPtr--;
    }
}

/**
 * @brief Display number reverse order
 * 
 * @param digitPtr Digit starting position
 * @param nbr Number to display
 * @param minNbrOfDigit Minimum number of digits of the number
 */
void STLED316S_Common::dispNumberRev(uint8_t digitPtr, uint32_t nbr, uint8_t minNbrOfDigit)
{
    while(digitPtr > 0) {
        uint32_t nbrSel = pow10(digitPtr - 1);

        if(nbr < nbrSel) {
            if(digitPtr <= minNbrOfDigit) {
                _dispDataBuffer[4-digitPtr] = _digitTable[0];
            } 
        } else {
            _dispDataBuffer[4-digitPtr] = _digitTable[(nbr/nbrSel)%10]; 
        }
        digitPtr--;
    }
}

void STLED316S_Common::setNumberMask(uint8_t mask)
{
  _numberMask = mask;
}

/******************************************************************************/
/* Class STLED316S_Common Public Functions                                    */
/******************************************************************************/
/**
 * @brief Initializes the STLED316S driver
 * 
 */
void STLED316S_Common::begin(void)
{
    //Wait until everything is initialized
    delayMicroseconds(20);
    //Initialize the first data of display buffer
    _dispDataBuffer[0] = STLED316S_DATA_WR | STLED316S_DIG_PAGE | STLED316S_ADDR_INC;
    //Puts STLED316S in default state (default brightness, clear, ON)
    setBrightness(DIGITall, STLED316S_DEFAULT_BRIGHTNESS);
    setBrightnessLED(LEDall, STLED316S_DEFAULT_BRIGHTNESS);
    clearDisplay();
    displayON();
}

/**
 * @brief Initializes the STLED316S driver with segments parameters
 * 
 * @param digA : Output pin of STLED316S for segment a
 * @param digB : Output pin of STLED316S for segment b
 * @param digC : Output pin of STLED316S for segment c
 * @param digD : Output pin of STLED316S for segment d
 * @param digE : Output pin of STLED316S for segment e
 * @param digF : Output pin of STLED316S for segment f
 * @param digG : Output pin of STLED316S for segment g
 * @param digDP : Output pin of STLED316S for segment DP
 */
void STLED316S_Common::begin(uint8_t digA, uint8_t digB, uint8_t digC, uint8_t digD, uint8_t digE, uint8_t digF, uint8_t digG, uint8_t digDP)
{
    _digDP = digDP;

    //b7->SEG8 / b6->SEG7 / b5->SEG6 / b4->SEG5 / b3->SEG4 / b2->SEG3 / b1->SEG2 / b0->SEG1
    _digitTable[0x0] = digA | digB | digC | digD | digE | digF;         //0
    _digitTable[0x1] = digB | digC;                                     //1
    _digitTable[0x2] = digA | digB | digD | digE | digG;                //2
    _digitTable[0x3] = digA | digB | digC | digD | digG;                //3
    _digitTable[0x4] = digB | digC | digF | digG;                       //4
    _digitTable[0x5] = digA | digC | digD | digF | digG;                //5
    _digitTable[0x6] = digA | digC | digD | digE | digF | digG;         //6
    _digitTable[0x7] = digA | digB | digC;                              //7
    _digitTable[0x8] = digA | digB | digC | digD | digE | digF | digG;  //8
    _digitTable[0x9] = digA | digB | digC | digD | digF | digG;         //9
    _digitTable[0xA] = digA | digB | digC | digE | digF | digG;         //A
    _digitTable[0xB] = digC | digD | digE | digF | digG;                //b
    _digitTable[0xC] = digA | digD | digE | digF;                       //C
    _digitTable[0xD] = digB | digC | digD | digE | digG;                //d
    _digitTable[0xE] = digA | digD | digE | digF | digG;                //E
    _digitTable[0xF] = digA | digE | digF | digG;                       //F
    _digitTable[CHAR_MINUS] = digG;                                     //-
    
    begin();
}

/**
 * @brief Turn ON the STLED316S display
 * 
 */
void STLED316S_Common::displayON(void)
{
    uint8_t data = STLED316S_DISP_ON_CMD;
    writeData(&data,1);
}

/**
 * @brief Turn OFF the STLED316S display
 * 
 */
void STLED316S_Common::displayOFF(void)
{
    uint8_t data = STLED316S_DISP_OFF_CMD; 
    writeData(&data,1);
}

/**
 * @brief Set the LED display brightness.
 * 
 * @param DIGITnum : DIGITall->All or Digit number (DIGITn1..DIGITn6)
 * @param brightness : (3 significant bits, valid range 0..7 (1/16 .. 14/16 dutycycle) 
 */
void STLED316S_Common::setBrightness(DIGITnum_t DIGITnum, uint8_t brightness)
{
    uint8_t data[5];
    
    data[0] = STLED316S_DATA_WR | STLED316S_CONF1_PAGE | STLED316S_ADDR_INC;
    //data[1] = (brightness<<5)|STLED316S_CONF_BRT_CONSTANT|STLED316S_CONF_DIGIT;
    data[1] = STLED316S_CONF_BRT_VARIABLE|(_nbrOfDigit-1);
    switch(DIGITnum)
    {
        case DIGITall:
            _digitBrightness[0] = (brightness<<4) | brightness;;
            _digitBrightness[1] = _digitBrightness[0];
            _digitBrightness[2] = _digitBrightness[0];
            break;
        case DIGITn1:
            _digitBrightness[0] = (_digitBrightness[0] & 0xF0) | brightness;
            break;
        case DIGITn2:
            _digitBrightness[0] = (brightness<<4) | (_digitBrightness[0] & 0x0F);
            break;
        case DIGITn3:
            _digitBrightness[1] = (_digitBrightness[1] & 0xF0) | brightness;
            break;
        case DIGITn4:
            _digitBrightness[1] = (brightness<<4) | (_digitBrightness[1] & 0x0F);
            break;
        case DIGITn5:
            _digitBrightness[2] = (_digitBrightness[2] & 0xF0) | brightness;
            break;
        case DIGITn6:
            _digitBrightness[2] = (brightness<<4) | (_digitBrightness[2] & 0x0F);
            break;
    }
    data[2] = _digitBrightness[0];
    data[3] = _digitBrightness[1];
    data[4] = _digitBrightness[2];
    
    writeData(&data[0],5);
}

/**
 * @brief Clear display
 * 
 */
void STLED316S_Common::clearDisplay(void)
{
    //_dispDataBuffer[0] = STLED316S_DATA_WR | STLED316S_DIG_PAGE | STLED316S_ADDR_INC;
    for(uint8_t i=0;i<_nbrOfDigit;i++) _dispDataBuffer[i+1] = 0x00;
    
    writeData(&_dispDataBuffer[0],_nbrOfDigit+1);
}

/**
 * @brief Controls the LEDs of a digit with raw data
 * 
 * @param DIGITnum : DIGITall->All or Digit number (DIGITn1..DIGITn6)
 * @param raw : b7->SEG8 / b6->SEG7 / b5->SEG6 / b4->SEG5 / b3->SEG4 / b2->SEG3 / b1->SEG2 / b0->SEG1
 */
void STLED316S_Common::dispRAW(DIGITnum_t DIGITnum, uint8_t raw)
{
    uint8_t i;

    if(DIGITnum > _nbrOfDigit) return;

    if(DIGITnum==DIGITall) for(i=0;i<_nbrOfDigit;i++) _dispDataBuffer[i+1] = raw;
    else _dispDataBuffer[DIGITnum] = raw;
    
    writeData(&_dispDataBuffer[0],_nbrOfDigit+1);
}

/**
 * @brief Controls the LEDs of all digit with raw data buffer
 * 
 * @param *raw : Address of the first byte
 * b7->SEG8 / b6->SEG7 / b5->SEG6 / b4->SEG5 / b3->SEG4 / b2->SEG3 / b1->SEG2 / b0->SEG1
 */
void STLED316S_Common::dispRAW(uint8_t *raw)
{
    uint8_t i;
    uint8_t* raw_data = raw;
    
    for(i=0;i<_nbrOfDigit;i++) {
        _dispDataBuffer[i+1] = *raw_data;
        raw_data++;
    }
    writeData(&_dispDataBuffer[0],_nbrOfDigit+1);
}

/**
 * @brief Display a unsigned decimal number
 * 
 * @param nbr : Unsigned Decimal Number
 */
void STLED316S_Common::dispUdec(uint32_t nbr)
{
    dispNumberRev(_nbrOfDigit,nbr,1);
    
    writeData(&_dispDataBuffer[0],_nbrOfDigit+1);
}

/**
 * @brief Display a hexadecimal data
 * 
 * @param data : Hex
 */
void STLED316S_Common::dispHex(uint32_t data)
{
    _dispDataBuffer[1] = _digitTable[data&0x0000000F];
    _dispDataBuffer[2] = _digitTable[(data&0x000000F0)>>4];
    
    writeData(&_dispDataBuffer[0],_nbrOfDigit+1);
}

/**
 * @brief Display a float number
 * 
 * @param nbr Float number
 * @param decimal Number of decimals
 */
void STLED316S_Common::dispFloat(float nbr, uint8_t decimal)
{
    uint8_t digitPtr = _nbrOfDigit;
    float nbrAbs = abs(nbr);

    //clear all digit
    for(uint8_t i=1; i<=_nbrOfDigit; i++) _dispDataBuffer[i] = 0;

    //Converting from floating point to integer
    uint32_t intNbr = (uint32_t)(nbrAbs * (float)pow10(decimal));

    //Check and display sign
    if(nbr < 0) {
        digitPtr = dispNumberMinusSign(intNbr,(decimal + 2));
    }

    //Display number
    dispNumber(digitPtr, intNbr, (decimal + 1));

    //Display dot
    _dispDataBuffer[decimal + 1] |= _digDP;

    //Send to driver
    writeData(&_dispDataBuffer[0],_nbrOfDigit+1);
}

/**
 * @brief Display a signed decimal number
 * 
 * @param nbr 
 */
void STLED316S_Common::dispDec(int32_t nbr)
{
    uint8_t digitPtr = _nbrOfDigit;
    uint32_t nbrAbs = abs(nbr);

    //clear all digit
    for(uint8_t i=1; i<=_nbrOfDigit; i++) _dispDataBuffer[i] = 0;

    //Check and display sign
    if(nbr < 0) {
        digitPtr = dispNumberMinusSign(nbrAbs,2);
    }

    //Display number
    dispNumber(digitPtr, nbrAbs, _nbrOfDigit);

    //Send to driver
    writeData(&_dispDataBuffer[0],_nbrOfDigit+1);
}

/**
 * @brief Activation or deactivation of the DP of a digit
 * 
 * @param DIGITnum : DIGITall->All or Digit number (DIGITn1..DIGITn6)
 * @param state : 0->OFF, 1->ON
 */
void STLED316S_Common::setDP(DIGITnum_t DIGITnum, uint8_t state)
{
    uint8_t i;

    if(DIGITnum > _nbrOfDigit) return;

    if(state) {
        if(DIGITnum==DIGITall) for(i=0;i<_nbrOfDigit;i++) _dispDataBuffer[i+1] |= _digDP;
        else _dispDataBuffer[DIGITnum] |= _digDP;
    } else {
        if(DIGITnum==DIGITall) for(i=0;i<_nbrOfDigit;i++) _dispDataBuffer[i+1] &= ~_digDP;
        else _dispDataBuffer[DIGITnum] &= ~_digDP;
    }
    
    writeData(&_dispDataBuffer[0],_nbrOfDigit+1);
}


/**
 * @brief Set the LED brightness.
 * 
 * @param LEDnum : LEDall->All or LED number (LEDn1..LEDn8)
 * @param brightness : (3 significant bits, valid range 0..7 (1/16 .. 14/16 dutycycle) 
 */
void STLED316S_Common::setBrightnessLED(LEDnum_t LEDnum, uint8_t brightness)
{
    uint8_t data[5];
    switch(LEDnum)
    {
        case LEDall:
            _LEDbrightness[0] = (brightness<<4) | brightness;
            _LEDbrightness[1] = _LEDbrightness[0];
            _LEDbrightness[2] = _LEDbrightness[0];
            _LEDbrightness[3] = _LEDbrightness[0];
            break;
        case LEDn1:
            _LEDbrightness[0] = (_LEDbrightness[0] & 0xF0) | brightness;
            break;
        case LEDn2:
            _LEDbrightness[0] = (brightness<<4) | (_LEDbrightness[0] & 0x0F);
            break;
        case LEDn3:
            _LEDbrightness[1] = (_LEDbrightness[1] & 0xF0) | brightness;
            break;
        case LEDn4:
            _LEDbrightness[1] = (brightness<<4) | (_LEDbrightness[1] & 0x0F);
            break;
        case LEDn5:
            _LEDbrightness[2] = (_LEDbrightness[2] & 0xF0) | brightness;
            break;
        case LEDn6:
            _LEDbrightness[2] = (brightness<<4) | (_LEDbrightness[2] & 0x0F);
            break;
        case LEDn7:
            _LEDbrightness[3] = (_LEDbrightness[3] & 0xF0) | brightness;
            break;
        case LEDn8:
            _LEDbrightness[3] = (brightness<<4) | (_LEDbrightness[3] & 0x0F);
            break;
    } 
    
    data[0] = STLED316S_DATA_WR | STLED316S_BRT_LED_PAGE | STLED316S_ADDR_INC;
    data[1] = _LEDbrightness[0];
    data[2] = _LEDbrightness[1];
    data[3] = _LEDbrightness[2];
    data[4] = _LEDbrightness[3];
    
    writeData(&data[0],5);
}

/**
 * @brief Set the LED state.
 * 
 * @param LEDnum : LEDall->All or LED number (LEDn1..LEDn8)
 * @param state : true or false
 */
void STLED316S_Common::setLED(LEDnum_t LEDnum, bool state)
{
    uint8_t data[2];
    
    if(LEDnum == LEDall) {
        if(state) _LEDstate = 0xFF;
        else _LEDstate = 0x00;
    } else {
        if(state) _LEDstate |= LEDnum;
        else _LEDstate &= ~LEDnum;
    }
    
    data[0] = STLED316S_DATA_WR | STLED316S_LED_PAGE;
    data[1] = _LEDstate;
    writeData(&data[0],2);
}

/**
 * @brief Read Key scan matrix state
 * 
 * @return uint16_t State of Key scan matrix
 */
uint16_t STLED316S_Common::readKeyScan(void)
{
    uint16_t keyState = 0;
    keyState = readData(STLED316S_ADDR_KEY_DATA2) << 8;
    keyState |= readData(STLED316S_ADDR_KEY_DATA1);
    return keyState;
}

/**
 * @brief default implementation of the writing function 
 */
void STLED316S_Common::writeData(uint8_t *data, uint8_t lenght)
{

}

/**
 * @brief default implementation of the reading function 
 */
uint8_t STLED316S_Common::readData(uint8_t address)
{
    return 0;
}
