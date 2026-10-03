// HW-628 / ESP8266 MQTT 測試

#include <ESP8266WiFi.h>
#include <PubSubClient.h>

const char* WIFI_SSID = "PHMHSCS02";
const char* WIFI_PASSWORD = "11011101";
const char* MQTT_SERVER = "mqttgo.io";
const uint16_t MQTT_PORT = 1883;
const char* MQTT_TOPIC = "jean/AQI";

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

WiFiClient espClient;
PubSubClient mqttClient(espClient);
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
    String clientId = "HW628-" + String(ESP.getChipId(), HEX);
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

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  Serial.begin(115200);
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
    const char* payload = "{\"pm01\":5,\"pm25\":8,\"pm10\":15}";
    bool success = mqttClient.publish(MQTT_TOPIC, payload);
    Serial.print("MQTT publish: ");
    Serial.println(success ? payload : "failed");
    digitalWrite(LED_BUILTIN, LOW);
    delay(100);
    digitalWrite(LED_BUILTIN, HIGH);
  }
}
