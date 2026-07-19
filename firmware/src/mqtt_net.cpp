#include "mqtt_net.h"
#include "config.h"
#include "secrets.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

#if ENABLE_MQTT
static WiFiClient wifiClient;
static PubSubClient mqtt(wifiClient);
static volatile bool resetCmd = false;

static const char* TOPIC_TEL = "plant/" DEVICE_ID "/telemetry";
static const char* TOPIC_FLT = "plant/" DEVICE_ID "/faults";
static const char* TOPIC_CMD = "plant/" DEVICE_ID "/command";

static void onMqtt(char* topic, byte* payload, unsigned int len) {
  JsonDocument doc;
  if (deserializeJson(doc, payload, len)) return;
  const char* cmd = doc["cmd"] | "";
  if (strcmp(cmd, "RESET") == 0) resetCmd = true;
}

static void ensureMqtt() {
  if (mqtt.connected()) return;
  String cid = String("esp32-") + DEVICE_ID;
  bool ok = (strlen(MQTT_USER) > 0)
                ? mqtt.connect(cid.c_str(), MQTT_USER, MQTT_PASS_MQTT)
                : mqtt.connect(cid.c_str());
  if (ok) {
    mqtt.subscribe(TOPIC_CMD, 1);
    Serial.println("MQTT connected");
  }
}

void netBegin() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASS);
  Serial.print("WiFi");
  for (int i = 0; i < 40 && WiFi.status() != WL_CONNECTED; ++i) {
    delay(250);
    Serial.print(".");
  }
  Serial.println();
  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("IP ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi failed — MQTT deferred");
  }
  mqtt.setServer(MQTT_HOST, MQTT_PORT);
  mqtt.setCallback(onMqtt);
  mqtt.setBufferSize(512);
}

void netLoop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.reconnect();
    return;
  }
  if (!mqtt.connected()) ensureMqtt();
  mqtt.loop();
}

static void publishMeas(const char* topic, const Meas& m, FaultCode f, bool relayOpen, const char* message) {
  if (!mqtt.connected()) return;
  char buf[384];
  int n;
  if (message) {
    n = snprintf(
        buf, sizeof(buf),
        "{\"schema_ver\":1,\"id\":\"%s\",\"ts\":%lu,"
        "\"Va\":%.1f,\"Vb\":%.1f,\"Vc\":%.1f,"
        "\"Ia\":%.2f,\"Ib\":%.2f,\"Ic\":%.2f,"
        "\"fault\":\"%s\",\"relay\":\"%s\",\"message\":\"%s\"}",
        DEVICE_ID, (unsigned long)(millis() / 1000UL),
        m.Va, m.Vb, m.Vc, m.Ia, m.Ib, m.Ic,
        faultName(f), relayOpen ? "OPEN" : "CLOSED", message);
  } else {
    n = snprintf(
        buf, sizeof(buf),
        "{\"schema_ver\":1,\"id\":\"%s\",\"ts\":%lu,"
        "\"Va\":%.1f,\"Vb\":%.1f,\"Vc\":%.1f,"
        "\"Ia\":%.2f,\"Ib\":%.2f,\"Ic\":%.2f,"
        "\"fault\":\"%s\",\"relay\":\"%s\"}",
        DEVICE_ID, (unsigned long)(millis() / 1000UL),
        m.Va, m.Vb, m.Vc, m.Ia, m.Ib, m.Ic,
        faultName(f), relayOpen ? "OPEN" : "CLOSED");
  }
  if (n > 0 && n < (int)sizeof(buf)) mqtt.publish(topic, buf);
}

void netPublishTelemetry(const Meas& m, FaultCode f, bool relayOpen) {
  publishMeas(TOPIC_TEL, m, f, relayOpen, nullptr);
}

void netPublishFault(const Meas& m, FaultCode f) {
  char msg[32];
  snprintf(msg, sizeof(msg), "Fault %s", faultName(f));
  publishMeas(TOPIC_FLT, m, f, true, msg);
}

bool netConsumeResetCommand() {
  if (!resetCmd) return false;
  resetCmd = false;
  return true;
}
#else
void netBegin() {}
void netLoop() {}
void netPublishTelemetry(const Meas&, FaultCode, bool) {}
void netPublishFault(const Meas&, FaultCode) {}
bool netConsumeResetCommand() { return false; }
#endif
