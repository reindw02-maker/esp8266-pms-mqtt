// HW-628 + PMS5003 空氣品質感測與 MQTT 上傳
// PMS5003 TX -> ESP8266 D5/GPIO14 (SoftwareSerial RX)
// PMS5003 RX -> ESP8266 D6/GPIO12 (SoftwareSerial TX，可選)

#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "PHMHSCS02";
const char* WIFI_PASSWORD = "11011101";
const char* MQTT_SERVER = "mqttgo.io";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "jean/AQI";

const uint8_t PMS_RX_PIN = 14;  // D5：接 PMS5003 TX
const uint8_t PMS_TX_PIN = 12;  // D6：接 PMS5003 RX，可不接
SoftwareSerial pmsSerial(PMS_RX_PIN, PMS_TX_PIN);

WiFiClient espClient;
PubSubClient mqttClient(espClient);

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

unsigned long lastPublish = 0;
const unsigned long PUBLISH_INTERVAL = 10000;

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    String clientId = "HW628-PMS-" + String(ESP.getChipId(), HEX);
    Serial.print("Connecting to MQTT...");
    if (mqttClient.connect(clientId.c_str())) {
      Serial.println("connected");
    } else {
      Serial.print("failed, state=");
      Serial.print(mqttClient.state());
      Serial.println("; retry in 5 seconds");
      delay(5000);
    }
  }
}

bool readPMS5003(int& pm01, int& pm25, int& pm10) {
  const unsigned long timeout = 2000;
  unsigned long start = millis();

  while (millis() - start < timeout) {
    if (!pmsSerial.available()) {
      yield();
      continue;
    }

    if (pmsSerial.read() != 0x42) continue;
    unsigned long headerTime = millis();
    while (!pmsSerial.available() && millis() - headerTime < 100) yield();
    if (!pmsSerial.available() || pmsSerial.read() != 0x4D) continue;

    uint8_t frame[32];
    frame[0] = 0x42;
    frame[1] = 0x4D;
    uint8_t index = 2;
    unsigned long frameStart = millis();
    while (index < sizeof(frame) && millis() - frameStart < 500) {
      if (pmsSerial.available()) frame[index++] = pmsSerial.read();
      else yield();
    }
    if (index < sizeof(frame)) continue;

    uint16_t frameLength = (uint16_t(frame[2]) << 8) | frame[3];
    if (frameLength != 28) continue;

    uint16_t checksum = (uint16_t(frame[30]) << 8) | frame[31];
    uint16_t calculated = 0;
    for (uint8_t i = 0; i < 30; i++) calculated += frame[i];
    if (checksum != calculated) continue;

    pm01 = (int(frame[4]) << 8) | frame[5];
    pm25 = (int(frame[6]) << 8) | frame[7];
    pm10 = (int(frame[8]) << 8) | frame[9];
    return true;
  }

  return false;
}

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
  pmsSerial.begin(9600);
  delay(100);

  connectWiFi();
  mqttClient.setServer(MQTT_SERVER, MQTT_PORT);
  connectMQTT();
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  unsigned long now = millis();
  if (now - lastPublish >= PUBLISH_INTERVAL || lastPublish == 0) {
    lastPublish = now;

    int pm01 = -1;
    int pm25 = -1;
    int pm10 = -1;
    bool valid = readPMS5003(pm01, pm25, pm10);

    char payload[80];
    snprintf(payload, sizeof(payload),
             "{\"pm01\":%d,\"pm25\":%d,\"pm10\":%d}",
             pm01, pm25, pm10);

    bool published = mqttClient.publish(MQTT_TOPIC, payload);
    Serial.print("PMS5003: ");
    Serial.println(valid ? "valid" : "not received");
    Serial.print("MQTT publish: ");
    Serial.println(published ? payload : "failed");

    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
