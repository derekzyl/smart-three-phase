#include "sensors.h"
#include "config.h"
#include <math.h>

static float readAdcVolts(int pin) {
  int raw = analogRead(pin);
  return (raw / ADC_MAX) * ADC_VREF;
}

static float rmsChannel(int pin, float offset, float gainToEng, bool isCurrent) {
  double acc = 0.0;
  for (int i = 0; i < RMS_SAMPLES; ++i) {
    float vs = readAdcVolts(pin);
    float x;
    if (isCurrent) {
      x = (vs - offset) / I_SENS_V_PER_A;  // amps
    } else {
      // Map instantaneous conditioned volts through gain toward mains volts
      x = (vs - offset) * gainToEng;
    }
    acc += (double)x * (double)x;
    delayMicroseconds(SAMPLE_US);
  }
  return sqrtf((float)(acc / RMS_SAMPLES));
}

#if DEMO_MODE
static uint32_t demoMs = 0;
#endif

void sensorsBegin() {
  analogReadResolution(12);
  analogSetAttenuation(ADC_11db);
  pinMode(PIN_VA, INPUT);
  pinMode(PIN_VB, INPUT);
  pinMode(PIN_VC, INPUT);
  pinMode(PIN_IA, INPUT);
  pinMode(PIN_IB, INPUT);
  pinMode(PIN_IC, INPUT);
}

void sensorsReadRms(Meas& out) {
#if DEMO_MODE
  demoMs = millis();
  float wobble = 1.5f * sinf(demoMs / 500.0f);
  out.Va = 230.0f + wobble;
  out.Vb = 229.5f - 0.5f * wobble;
  out.Vc = 230.5f + 0.3f * wobble;
  out.Ia = 4.0f + 0.1f * wobble;
  out.Ib = 4.1f;
  out.Ic = 3.9f;
  // Inject PL every ~20 s for demo
  if ((demoMs / 1000) % 25 == 20) {
    out.Va = 12.0f;
  }
#else
  out.Va = rmsChannel(PIN_VA, V_OFFSET_DEFAULT, V_GAIN_DEFAULT, false);
  out.Vb = rmsChannel(PIN_VB, V_OFFSET_DEFAULT, V_GAIN_DEFAULT, false);
  out.Vc = rmsChannel(PIN_VC, V_OFFSET_DEFAULT, V_GAIN_DEFAULT, false);
  out.Ia = fabsf(rmsChannel(PIN_IA, I_OFFSET_DEFAULT, 1.0f, true));
  out.Ib = fabsf(rmsChannel(PIN_IB, I_OFFSET_DEFAULT, 1.0f, true));
  out.Ic = fabsf(rmsChannel(PIN_IC, I_OFFSET_DEFAULT, 1.0f, true));
#endif
  out.ms = millis();
}
