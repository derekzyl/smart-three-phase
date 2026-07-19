/**
 * PG-PROTO-001 — Intelligent Three-Phase Load Protection
 * FreeRTOS tasks: sampling/protection on core 0; OLED/MQTT/GSM on core 1
 */
#include <Arduino.h>
#include "config.h"
#include "types.h"
#include "sensors.h"
#include "protection.h"
#include "relay.h"
#include "oled_ui.h"
#include "gsm_sms.h"
#include "mqtt_net.h"

SharedState gState;

static TaskHandle_t hSample = nullptr;
static TaskHandle_t hProtect = nullptr;

static void taskSampling(void* arg) {
  TickType_t last = xTaskGetTickCount();
  for (;;) {
    Meas m;
    sensorsReadRms(m);
    portENTER_CRITICAL(&gState.mux);
    gState.meas = m;
    portEXIT_CRITICAL(&gState.mux);
    if (hProtect) xTaskNotifyGive(hProtect);
    vTaskDelayUntil(&last, pdMS_TO_TICKS(SAMPLE_TASK_MS));
  }
}

static void taskProtection(void* arg) {
  FaultCode prev = FaultCode::NONE;
  for (;;) {
    ulTaskNotifyTake(pdTRUE, portMAX_DELAY);
    Meas m;
    bool resetReq = false;
    portENTER_CRITICAL(&gState.mux);
    m = gState.meas;
    resetReq = gState.resetRequest;
    gState.resetRequest = false;
    portEXIT_CRITICAL(&gState.mux);

    if (resetReq && gState.latched) {
      FaultCode now = evaluateFaults(m);
      if (now == FaultCode::NONE) {
        relayAllowClose();
        portENTER_CRITICAL(&gState.mux);
        gState.latched = false;
        gState.fault = FaultCode::NONE;
        gState.relayOpen = false;
        portEXIT_CRITICAL(&gState.mux);
        Serial.println("Reset accepted — relays closed");
      } else {
        Serial.println("Reset denied — still unhealthy");
      }
    }

    if (gState.latched) continue;

    FaultCode f = evaluateFaults(m);
    if (f != FaultCode::NONE) {
      relayIsolateAll();
      portENTER_CRITICAL(&gState.mux);
      gState.fault = f;
      gState.relayOpen = true;
      gState.latched = true;
      portEXIT_CRITICAL(&gState.mux);
      if (f != prev) {
        gsmEnqueueFaultSms(f, m);
        netPublishFault(m, f);
        Serial.printf("TRIP %s\n", faultName(f));
      }
    } else {
      portENTER_CRITICAL(&gState.mux);
      gState.fault = FaultCode::NONE;
      portEXIT_CRITICAL(&gState.mux);
    }
    prev = f;
  }
}

static void taskComms(void* arg) {
  uint32_t lastPub = 0;
  uint32_t lastOled = 0;
  for (;;) {
    netLoop();
    gsmTaskPoll();

    if (netConsumeResetCommand()) {
      portENTER_CRITICAL(&gState.mux);
      gState.resetRequest = true;
      portEXIT_CRITICAL(&gState.mux);
    }

    uint32_t now = millis();
    Meas m;
    FaultCode f;
    bool open;
    portENTER_CRITICAL(&gState.mux);
    m = gState.meas;
    f = gState.fault;
    open = gState.relayOpen;
    portEXIT_CRITICAL(&gState.mux);

    if (now - lastOled > 400) {
      oledRender(m, f, open);
      lastOled = now;
    }
    if (now - lastPub > 1000) {
      netPublishTelemetry(m, f, open);
      lastPub = now;
    }
    vTaskDelay(pdMS_TO_TICKS(50));
  }
}

void setup() {
  Serial.begin(115200);
  delay(200);
  Serial.println("\nPG-PROTO-001 boot");

  relayBegin();
  sensorsBegin();
  oledBegin();
  gsmBegin();
  netBegin();

  // Start closed only after first healthy evaluation would be ideal;
  // keep OPEN until optional button/MQTT reset after boot clear.
  // For lab convenience with DEMO_MODE healthy data, allow close:
#if DEMO_MODE
  relayAllowClose();
  gState.relayOpen = false;
  gState.latched = false;
#endif

  xTaskCreatePinnedToCore(taskSampling, "sample", 4096, nullptr, 3, &hSample, 0);
  xTaskCreatePinnedToCore(taskProtection, "protect", 4096, nullptr, 3, &hProtect, 0);
  xTaskCreatePinnedToCore(taskComms, "comms", 8192, nullptr, 1, nullptr, 1);
}

void loop() {
  vTaskDelay(pdMS_TO_TICKS(1000));
}
