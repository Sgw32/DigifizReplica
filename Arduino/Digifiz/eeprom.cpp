#include "eeprom.h"

ExternalEEPROM myMem;

void initEEPROM()
{
    Serial.begin(115200);
    Serial.println("PHL EEPROM test");

    Wire.begin();

    if (myMem.begin() == false)
    {
        Serial.println("No memory detected. ");
    }
    else
    {
        Serial.println("Memory detected!");

        Serial.print("Mem size in bytes: ");
        Serial.println(myMem.length());

        //Yes you can read and write bytes, but you shouldn't!
        byte myValue1 = 200;
        myMem.write(0, myValue1); //(location, data)

        byte myRead1 = myMem.read(0);
        Serial.print("I read: ");
        Serial.println(myRead1);

        //You should use gets and puts. This will automatically and correctly arrange
        //the bytes for larger variable types.
        int myValue2 = -366;
        myMem.put(10, myValue2); //(location, data)
        int myRead2;
        myMem.get(10, myRead2); //location to read, thing to put data into
        Serial.print("I read: ");
        Serial.println(myRead2);

        float myValue3 = -7.35;
        myMem.put(20, myValue3); //(location, data)
        float myRead3;
        myMem.get(20, myRead3); //location to read, thing to put data into
        Serial.print("I read: ");
        Serial.println(myRead3);
    }
}
