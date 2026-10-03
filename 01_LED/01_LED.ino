// HW-628 / ESP8266 LED 閃爍
// NodeMCU 1.0 (ESP-12E) 的內建 LED 通常接在 GPIO2，且為低電位亮。

#ifndef LED_BUILTIN
#define LED_BUILTIN 2
#endif

const uint16_t BLINK_INTERVAL = 500;

void setup() {
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);  // 內建 LED 熄滅
}

void loop() {
  digitalWrite(LED_BUILTIN, LOW);   // 亮
  delay(BLINK_INTERVAL);
  digitalWrite(LED_BUILTIN, HIGH);  // 滅
  delay(BLINK_INTERVAL);
}
