#include "buzzer.h"
#include "esp_log.h"
#include "reg_inout.h"

extern digifiz_pars digifiz_parameters;
uint8_t buzzerIsOn;
 
void initBuzzer()
{
    ESP_LOGI(LOG_TAG, "initBuzzer started");
    //buzzerIsOn  = 1; //only for test
    buzzerIsOn = 0;
    ESP_LOGI(LOG_TAG, "initBuzzer ended");
}

void buzzerConstantOn()
{
    digifiz_reg_out.buzzer = 1;
}

void buzzerConstantOff()
{
    digifiz_reg_out.buzzer = 0;
}

void buzzerToggle()
{
    digifiz_reg_out.buzzer = !digifiz_reg_out.buzzer;
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
    return buzzerIsOn&&(!digifiz_parameters.buzzerOff);
}
