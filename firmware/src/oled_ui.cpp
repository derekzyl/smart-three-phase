#include "oled_ui.h"
#include "config.h"

#if ENABLE_OLED
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SH110X.h>

static Adafruit_SH1106G display(128, 64, &Wire, -1);
static bool oledOk = false;
#endif

void oledBegin() {
#if ENABLE_OLED
  Wire.begin(PIN_SDA, PIN_SCL);
  oledOk = display.begin(OLED_ADDR, true);
  if (!oledOk) {
    Serial.println("OLED init failed");
    return;
  }
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SH110X_WHITE);
  display.setCursor(0, 0);
  display.println("PG-PROTO-001");
  display.println("Protect boot...");
  display.display();
#endif
}

void oledRender(const Meas& m, FaultCode f, bool relayOpen) {
#if ENABLE_OLED
  if (!oledOk) return;
  display.clearDisplay();
  display.setCursor(0, 0);
  display.printf("Va%3.0f Vb%3.0f Vc%3.0f\n", m.Va, m.Vb, m.Vc);
  display.printf("Ia%4.1f Ib%4.1f Ic%4.1f\n", m.Ia, m.Ib, m.Ic);
  display.printf("ST:%s  RLY:%s\n", faultName(f), relayOpen ? "OPEN" : "CLSD");
  display.printf("ID:%s", DEVICE_ID);
  display.display();
#endif
}
