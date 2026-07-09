#include "emergency.h"
#include "params.h"
#include "display_next.h"
#include "reg_inout.h"
#include "esp_log.h"
#include "millis.h"

uint8_t emergency_state = 0;
uint16_t emergencyRPM = 3000;
uint8_t last_emergency_state = 0;

#define EMERGENCY_OIL03_LOW       0x01
#define EMERGENCY_OIL18_HIGH_RPM  0x02

static uint8_t oil18BlinkState = 0;

void initEmergencyModule()
{
    ESP_LOGI(LOG_TAG, "initEmergencyModule started");
}

uint8_t processCHECKEngine()
{
    //TODO implement
    if (digifiz_reg_in.checkEng==1)
    {
        setCheckEngine(false);
    }
    else
    {
        setCheckEngine(true);
    }
    return 0;
}

uint8_t processOilPressure(int mRPM)
{
    uint8_t state = 0;

    /*
     * oil03 == 0 means 0.3 bar sensor is active / low pressure.
     * This must always show solid oil indicator, but without buzzer by itself.
     */
    if (digifiz_reg_in.oil03 == 0)
    {
        state |= EMERGENCY_OIL03_LOW;
    }

#if !defined(EMERGENCY_DISABLE_SENSOR_CHECK) && !defined(AUDI_DISPLAY) && !defined(AUDI_RED_DISPLAY)
    /*
     * Assumption, same as old code:
     * oil18 == 1 means 1.8 bar sensor is NOT active, i.e. pressure is not enough.
     */
    if ((mRPM > emergencyRPM) && (digifiz_reg_in.oil18 == 1))
    {
        state |= EMERGENCY_OIL18_HIGH_RPM;
    }
#endif

    emergency_state = state;
    last_emergency_state = emergency_state;

    return emergency_state;
}

void checkEmergency(int mRPM)
{
  if (digifiz_parameters.option_testmode_on.value)
  {  
    // digifiz_reg_out.byte+=1;
    // digifiz_reg_out.byte = digifiz_reg_out.byte%8;
  }
  else
  {
    emergency_state = processOilPressure(mRPM);
    processCHECKEngine();

    uint8_t oilIndicatorOn = 0;
    uint8_t buzzerOnFlag = 0;

    /*
     * 0.3 bar low pressure:
     * solid oil indicator, no buzzer by itself.
     */
    if (emergency_state & EMERGENCY_OIL03_LOW)
    {
        oilIndicatorOn = 1;
    }

    /*
     * 1.8 bar missing at high RPM:
     * buzzer enabled.
     * Indicator blinks unless 0.3 bar is also low,
     * in which case indicator stays solid.
     */
    if (emergency_state & EMERGENCY_OIL18_HIGH_RPM)
    {
        buzzerOnFlag = 1;

        if (emergency_state & EMERGENCY_OIL03_LOW)
        {
            oilIndicatorOn = 1;      // full lit, not blinking
        }
        else
        {
            oil18BlinkState = !oil18BlinkState;
            oilIndicatorOn = oil18BlinkState;
        }
    }
    else
    {
        oil18BlinkState = 0;         // reset blink when fault disappears
    }

#ifdef USE_DISPLAY_LEDS
    digifiz_reg_out.led_p_oil = oilIndicatorOn;
#endif

#ifdef DIGIFIZ_LCD_DISPLAY
    setLCDOilIndicator(oilIndicatorOn);
#endif

#ifdef DIGIFIZ_NEXT_DISPLAY
    setOilIndicator(oilIndicatorOn);
#endif

#ifdef DIGIFIZ_REFIZ_DISPLAY
    setOilIndicator(oilIndicatorOn);
#endif

    if (buzzerOnFlag)
    {
        buzzerOn();
    }
    else
    {
        buzzerOff();
    }
  }
}
