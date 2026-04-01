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

void loop() {
  protocolProcess();

  delay(70);
  triggerFrameIfIdle();

  protocolProcess();

  delay(70);
  triggerFrameIfIdle();
}
