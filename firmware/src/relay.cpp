#include "relay.h"
#include "config.h"

static bool openState = true;

static void writeRelay(int pin, bool open) {
#if RELAY_ACTIVE_LOW
  digitalWrite(pin, open ? LOW : HIGH);
#else
  digitalWrite(pin, open ? HIGH : LOW);
#endif
}

void relayBegin() {
  pinMode(PIN_RELAY_CH1, OUTPUT);
  pinMode(PIN_RELAY_CH2, OUTPUT);
  pinMode(PIN_RELAY_CH3, OUTPUT);
  pinMode(PIN_RELAY_CH4, OUTPUT);
  relayIsolateAll();  // safe default at boot
}

void relayIsolateAll() {
  writeRelay(PIN_RELAY_CH1, true);
  delay(2);
  writeRelay(PIN_RELAY_CH2, true);
  delay(2);
  writeRelay(PIN_RELAY_CH3, true);
  writeRelay(PIN_RELAY_CH4, true);
  openState = true;
}

void relayAllowClose() {
  writeRelay(PIN_RELAY_CH1, false);
  delay(2);
  writeRelay(PIN_RELAY_CH2, false);
  delay(2);
  writeRelay(PIN_RELAY_CH3, false);
  writeRelay(PIN_RELAY_CH4, false);
  openState = false;
}

bool relayIsOpen() { return openState; }
