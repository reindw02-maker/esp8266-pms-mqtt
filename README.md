# ESP8266 PMS MQTT

HW-628 ESP8266 空氣品質物聯網專案，整合 PMS5003、Wi-Fi 與 MQTT。

## 專案內容

- `01_LED`：ESP8266 內建 LED 閃爍測試
- `02_mqtt`：每 10 秒發布固定 JSON 的 MQTT 測試
- `03_mqtt_pms`：讀取 PMS5003 的 PM1.0、PM2.5、PM10，並透過 MQTT 發布
- `1003學習歷程.docx`：專案學習歷程與成果紀錄
- `report_assets`：架構圖、流程圖與實作成果圖片

## MQTT 設定

- Broker：`mqttgo.io`
- Port：`1883`
- Topic：`jean/AQI`
- 傳送週期：10 秒

公開版本的程式將 Wi-Fi 密碼設為 `YOUR_WIFI_PASSWORD`，燒錄前請在本機程式中填入實際密碼。

若 PMS5003 無法收到有效封包，`03_mqtt_pms` 會傳送：

```json
{"pm01":-1,"pm25":-1,"pm10":-1}
```

## PMS5003 接線

- VCC → HW-628 5V / VU / VIN
- GND → GND
- TX → D5 / GPIO14
- RX → D6 / GPIO12
