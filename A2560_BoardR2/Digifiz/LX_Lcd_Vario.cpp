#include "Arduino.h"
#include "LX_Lcd_Vario.h"
#include <Wire.h>
#include "LCD_VARIO_mapping.h"


//Constructor, initialize Wire lib
LX_Lcd_Vario::LX_Lcd_Vario()
{
    Wire.begin();
}

//Begin method, not used so far
void LX_Lcd_Vario::begin(){
}

void LX_Lcd_Vario::addPCF(uint8_t pcfAddr, uint8_t modeSet, uint8_t devSel, uint8_t blink, uint8_t bankSel){
    settings[PCFcount][0] = pcfAddr;
    settings[PCFcount][1] = modeSet;
    settings[PCFcount][2] = devSel;
    settings[PCFcount][3] = blink;
    settings[PCFcount][4] = bankSel;
    PCFcount++;
}

void LX_Lcd_Vario::init(){
    //init all added PCF's, don't know exactly if you have to init all
    for (uint8_t pcf = 0; pcf < PCFcount; pcf++){
        Wire.beginTransmission(settings[pcf][0]);
        Wire.write(CONTINUE | settings[pcf][1]); //modeset
        Wire.write(CONTINUE | settings[pcf][3]); //blink
        Wire.write(LAST | settings[pcf][2]); //devSel
        //Wire.write(LAST | settings[pcf][4]); //banksel
        Wire.endTransmission();
    }
    for (int i=0;i!=40;i++)
    {
        digifizBuffer[i]=0x00;
    }
}

void LX_Lcd_Vario::clear(){
    //works only for one Hardware Adress for now, have to implement the logic for two
    Wire.beginTransmission(settings[0][0]);
	Wire.write(CONTINUE | DEVICE_SELECT); //select the first PCF, assume that the first is the device with 000, maybe implement another logic
	Wire.write(LOADDATAPOINTER);
	for (int i=0; i < PCFcount * 20; i++) {
		Wire.write(0x00);
	}
	Wire.endTransmission();
}

void LX_Lcd_Vario::fire(){
    //same as in lcd_clear()
    Wire.beginTransmission(settings[0][0]);
	Wire.write(CONTINUE | DEVICE_SELECT); //select the first PCF, assume that the first is the device with 000, maybe implement another logic
	Wire.write(LOADDATAPOINTER);
	for (int i=0; i < PCFcount * 20; i++) {
		Wire.write(0xFF);
	}
	Wire.endTransmission();
}

void LX_Lcd_Vario::fireDigifiz(){
    //same as in lcd_clear()
    Wire.beginTransmission(settings[0][0]);
	Wire.write(CONTINUE | DEVICE_SELECT); //select the first PCF, assume that the first is the device with 000, maybe implement another logic
	Wire.write(LOADDATAPOINTER);
	for (int i=0; i < 40; i++) {
		Wire.write(digifizBuffer[i]);
	}
	Wire.endTransmission();
}


void LX_Lcd_Vario::setDigifizBuffer(uint8_t bank, uint8_t val)
{
	digifizBuffer[bank]=val;	
}

void LX_Lcd_Vario::setDigifizBufferMasked(uint8_t bank, uint8_t val, uint8_t mask)
{
	digifizBuffer[bank]=(val&mask)|(digifizBuffer[bank]&(~mask));	
}

void LX_Lcd_Vario::setDigifizBufferSegment(uint8_t bank, uint8_t segment, uint8_t val)
{
	if (val)
		digifizBuffer[bank]|=segment;
	else
		digifizBuffer[bank]&=~segment;
}

void LX_Lcd_Vario::show(){
    //first set all data from the old buffer to zero
    for (uint8_t i = 0; i < oldBufferCount; i++){
        oldBuffer[i][2] = 0;
    }
    //compare if pointer already exist and set data from new buffer
    //if not add line to oldBuffer
    int counter = oldBufferCount;
    for (uint8_t i = 0; i < bufferCount; i++){
        bool found = false;
        for (uint8_t j = 0; j < counter; j++){
            if (buffer[i][1] == oldBuffer[j][1] && buffer[i][0] == oldBuffer[j][0]){
                oldBuffer[j][2] = buffer[i][2];
                found = true;
                break;
            }
        }
        if(!found){
            Serial.println("not found");
                oldBuffer[oldBufferCount][0] = buffer[i][0];
                oldBuffer[oldBufferCount][1] = buffer[i][1];
                oldBuffer[oldBufferCount][2] = buffer[i][2];
                found = false;
                oldBufferCount++;
        }
    }

    //actually write the data
    for (uint8_t i = 0; i < oldBufferCount; i++){
        Wire.beginTransmission(settings[0][0]);
        Wire.write(CONTINUE | settings[oldBuffer[i][0]][2]); //devsel
        Wire.write(oldBuffer[i][1]); //pointer
        Wire.write(oldBuffer[i][2]); //data
        Wire.endTransmission();
    }


    //copy Array for next interval
    for (uint8_t i = 0; i < bufferCount; i++){
        for (uint8_t j = 0; j < 3; j++){
            oldBuffer[i][j] = buffer[i][j];
        }
    }
    oldBufferCount = bufferCount;
    bufferCount = 0;
}

void LX_Lcd_Vario::showOLD(){
    //clear the display, let it flicker
    clear();
    //Write the data to the display
    for (uint8_t i = 0; i < bufferCount; i++){
        Wire.beginTransmission(settings[0][0]);
        Wire.write(CONTINUE | settings[buffer[i][0]][2]); //devsel
        Wire.write(buffer[i][1]); //pointer
        Wire.write(buffer[i][2]); //data
        Wire.endTransmission();
    }
    //reset the buffer counter, no need to clear the array
    bufferCount = 0;
}

void LX_Lcd_Vario::addToBuffer(uint8_t *val){
    //check if section already set and do bitwise or
    bool found = false;
    for (uint8_t i = 0; i < bufferCount; i++){
        if (buffer[i][0] == val[0] && buffer[i][1] == val[1]){
            buffer[i][2] = buffer[i][2] | val[2];
            found = true;
            break;
        }
    }
    if(!found){
        for (uint8_t i = 0; i < 3; i++){
            buffer[bufferCount][i] = val[i];
        }
        bufferCount++;
    }
    
}

void LX_Lcd_Vario::addInd(float val){
    if (val >= 5){
        addToBuffer((uint8_t *)IND[0]);
    }else if (val <= -5){
        addToBuffer((uint8_t *)IND[50]);
    }else{
        val = (uint8_t)round((val - (-5)) * (0 - 50) / (5 - (-5)) + (50));
        addToBuffer((uint8_t *)IND[(uint8_t)val]);
    }
}

void LX_Lcd_Vario::addScr(uint8_t val){
    addToBuffer((uint8_t *)SCR[(uint8_t)val]);
}

void LX_Lcd_Vario::addSym(uint8_t val){
    addToBuffer((uint8_t *)SYM[(uint8_t)val]);
}

void LX_Lcd_Vario::addNumber(uint8_t pos, uint8_t val){
    for (uint8_t i = 1; i <= NUM[val][0]; i++){
        //Serial.println(CHR9[(uint8_t)NUM[val][i]][2]);
        addToBuffer((uint8_t *)CHR1[(uint8_t)NUM[val][i]+((pos -1)*7)]);
    }
}

void LX_Lcd_Vario::upperNum(int16_t val){
    for (uint8_t i = 0; i < 2; i++){
        //addToBuffer((uint8_t *)NUM1[(uint8_t)NUM11[i]]);
        //addToBuffer((uint8_t *)SYM[(uint8_t)val]);
    }
    
}