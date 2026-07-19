#include "gsm_sms.h"
#include "config.h"
#include "secrets.h"
#include <HardwareSerial.h>

#if ENABLE_GSM
static HardwareSerial SerialAT(2);
static bool gsmReady = false;
static volatile bool smsPending = false;
static char smsBody[160];

static bool expectOk(uint32_t timeoutMs) {
  uint32_t start = millis();
  String buf;
  while (millis() - start < timeoutMs) {
    while (SerialAT.available()) {
      char c = SerialAT.read();
      buf += c;
      if (buf.indexOf("OK") >= 0) return true;
      if (buf.indexOf("ERROR") >= 0) return false;
    }
    delay(5);
  }
  return false;
}

void gsmBegin() {
  SerialAT.begin(GSM_BAUD, SERIAL_8N1, PIN_GSM_RX, PIN_GSM_TX);
  delay(500);
  SerialAT.println("AT");
  gsmReady = expectOk(1000);
  if (gsmReady) {
    SerialAT.println("AT+CMGF=1");
    expectOk(1000);
    Serial.println("GSM ready");
  } else {
    Serial.println("GSM not responding (will retry on send)");
  }
}

void gsmEnqueueFaultSms(FaultCode f, const Meas& m) {
  snprintf(smsBody, sizeof(smsBody),
           "%s FAULT %s Va=%.0f Vb=%.0f Vc=%.0f Ia=%.1f LOAD ISOLATED",
           DEVICE_ID, faultName(f), m.Va, m.Vb, m.Vc, m.Ia);
  smsPending = true;
}

void gsmTaskPoll() {
  if (!smsPending) return;
  smsPending = false;
  if (!gsmReady) {
    SerialAT.println("AT");
    gsmReady = expectOk(1000);
  }
  if (!gsmReady) {
    Serial.println("SMS skipped — modem offline");
    return;
  }
  SerialAT.print("AT+CMGS=\"");
  SerialAT.print(SMS_NUMBER);
  SerialAT.println("\"");
  delay(200);
  SerialAT.print(smsBody);
  SerialAT.write(26);
  bool ok = expectOk(12000);
  Serial.println(ok ? "SMS sent" : "SMS failed");
}
#else
void gsmBegin() {}
void gsmEnqueueFaultSms(FaultCode, const Meas&) {}
void gsmTaskPoll() {}
#endif
