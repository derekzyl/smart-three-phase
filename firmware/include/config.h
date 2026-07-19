#pragma once
// Pin map and thresholds — PG-PROTO-001 (Chapter Three Table 3.6)

#include <Arduino.h>

// ---- Device identity ----
#ifndef DEVICE_ID
#define DEVICE_ID "PG-PROTO-001"
#endif

// ---- ADC (ADC1 — WiFi-safe) ----
static const int PIN_VA = 34;
static const int PIN_VB = 35;
static const int PIN_VC = 32;
static const int PIN_IA = 33;
static const int PIN_IB = 39;
static const int PIN_IC = 36;

// ---- Relay (active-LOW optocoupler modules common) ----
static const int PIN_RELAY_CH1 = 25;  // Phase R
static const int PIN_RELAY_CH2 = 26;  // Phase Y
static const int PIN_RELAY_CH3 = 27;  // Phase B
static const int PIN_RELAY_CH4 = 14;  // Aux

#ifndef RELAY_ACTIVE_LOW
#define RELAY_ACTIVE_LOW 1
#endif

// ---- OLED I2C SH1106 ----
static const int PIN_SDA = 21;
static const int PIN_SCL = 22;
static const uint8_t OLED_ADDR = 0x3C;

// ---- SIM800L UART2 ----
static const int PIN_GSM_RX = 16;  // ESP32 RX <- SIM TX
static const int PIN_GSM_TX = 17;  // ESP32 TX -> SIM RX
static const int GSM_BAUD = 9600;

// ---- Sampling ----
static const int RMS_SAMPLES = 100;
static const int SAMPLE_US = 200;          // ~5 kHz burst
static const int SAMPLE_TASK_MS = 5;
static const uint8_t FAULT_CONFIRM_N = 3;  // consecutive windows

// ---- Thresholds (phase-neutral, configurable) ----
static const float V_UV = 180.0f;
static const float V_OV = 250.0f;
static const float V_PL = 40.0f;
static const float V_PRESENT = 100.0f;
static const float I_OC = 10.0f;           // setpoint (A)
static const float IMB_LIMIT_PCT = 5.0f;

// ---- ADC / sensor calibration defaults ----
static const float ADC_VREF = 3.3f;
static const float ADC_MAX = 4095.0f;
// Mid-rail offset (adjust after zeroing)
static const float V_OFFSET_DEFAULT = 1.65f;
static const float I_OFFSET_DEFAULT = 1.65f;
// Engineering gains (tune during calibration)
static const float V_GAIN_DEFAULT = 151.0f;   // volts RMS scale helper
static const float I_SENS_V_PER_A = 0.066f;   // ACS712-30A

// ---- Feature toggles (1=on) ----
#ifndef ENABLE_OLED
#define ENABLE_OLED 1
#endif
#ifndef ENABLE_GSM
#define ENABLE_GSM 1
#endif
#ifndef ENABLE_MQTT
#define ENABLE_MQTT 1
#endif
#ifndef DEMO_MODE
#define DEMO_MODE 0   // synthetic readings when no sensors
#endif
