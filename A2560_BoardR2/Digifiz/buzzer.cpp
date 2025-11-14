#include "buzzer.h"

extern digifiz_pars_t digifiz_parameters;
uint8_t buzzerIsOn;

void initBuzzer()
{
    //buzzerIsOn  = 1; //only for test
    buzzerIsOn = 0;
    pinMode(BUZZER_PIN,OUTPUT);
}

void buzzerConstantOn()
{
    digitalWrite(BUZZER_PIN,HIGH);
}

void buzzerConstantOff()
{
    digitalWrite(BUZZER_PIN,LOW);
}

void buzzerToggle()
{
    digitalWrite(BUZZER_PIN,!digitalRead(BUZZER_PIN));
}

void buzzerOn()
{
    buzzerIsOn = 1;
}

void buzzerOff()
{
    buzzerIsOn = 0;
}

uint8_t getBuzzerEnabled()
{
    return buzzerIsOn&&(!digifiz_parameters.buzzerOff.value);
}
