#include "config.h"
#include "pattern_sender.h"
#include "serial_protocol.h"

void setup() {
  Serial.begin(UART_BAUD);
  initDisplayClockTimer();
  initPins();

  delay(50);  // as in Digifiz, 50 ms delay
  initDigifiz();
  delay(600);

  protocolInit(Serial);
}

uint32_t lastProtocol = 0;
uint32_t lastTrigger = 0;

void loop() {
  uint32_t now = millis();

  // Call protocolProcess every 2 ms
  static uint32_t lastProtocolUs = 0;
  uint32_t nowUs = micros();

  if ((uint32_t)(nowUs - lastProtocolUs) >= 2000) {
    lastProtocolUs = nowUs;
    protocolProcess();
  }

  // Keep original 70 ms trigger period
  if ((uint32_t)(now - lastTrigger) >= 70) {
    lastTrigger = now;
    triggerFrameIfIdle();
  }
}
