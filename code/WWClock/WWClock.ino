#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <ArduinoOTA.h>
#include "9x9EN.h"

//Wifi manager SSID and Password
#define WMSSID "WWClock"
#define WMPSK "01234567"

#define OTAPSK "01234567"

#define B_RATE 115200


WiFiManager wm;
volatile unsigned long startMillis, seconds;
StaticJsonDocument<1024> doc;

TaskHandle_t ledTaskHandle;
TaskHandle_t wifiTaskHandle;

void wifiTask(void* parameter) {
  unsigned long fetchInterval = 20UL * 1000UL;
  unsigned long lastFetch = 0;

  for (;;) {
    if (WiFi.status() == WL_CONNECTED) {
      unsigned long now = millis();

      if (now - lastFetch >= fetchInterval || lastFetch == 0) {
        lastFetch = now;
        WiFiClient client;
        HTTPClient http;
        if (http.begin(client, TIME_URL)) {
          int httpCode = http.GET();
          if (httpCode == HTTP_CODE_OK) {
            String payload = http.getString();
            Serial.println("JSON:");
            Serial.println(payload);

            DeserializationError error = deserializeJson(doc, payload);
            if (!error) {
              startMillis = millis();
              String datetime = doc["datetime"];
              int hours = datetime.substring(11, 13).toInt();
              int minutes = datetime.substring(14, 16).toInt();
              seconds = (hours * 3600L) + (minutes * 60L) + payload.substring(17, 19).toInt();
              fetchInterval = 15UL * 60UL * 1000UL;
            } else {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
              fetchInterval = 20UL * 1000UL;
            }
          } else {
            Serial.printf("HTTP error: %d\n", httpCode);
            fetchInterval = 10UL * 1000UL;
          }
          http.end();
        } else {
          Serial.println("No http begin");
          fetchInterval = 20UL * 1000UL;
        }
      }
    } else {
      Serial.println("WiFi not connected!");
      fetchInterval = 20UL * 1000UL;
    }

    //Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
    vTaskDelay(100 / portTICK_PERIOD_MS);
  }
}



void ledTask(void* parameter) {
  for (;;) {
    if (startMillis != 0) {
      unsigned long newSeconds = seconds + (millis() - startMillis) / 1000L;
      int hour = int(newSeconds / 3600L) % 24;
      int minute = (newSeconds % 3600L) / 60L;
      setClock(hour, minute);


      //indicate long time (21600000 = 6 hours) no timing. Red if not connected. Blue if connected but unable to load url.
      if (startMillis + 21600000 < millis()) {
        setWord(NOTIME, false);
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(B_RATE);

  pixels.setBrightness(BRIGHTNESS);

  setWord(WIFI, true);

  WiFi.mode(WIFI_STA);
  WiFi.hostname(WIFI_CLOCK_NAME);
  WiFi.setSleep(false);

  if (!wm.autoConnect(WMSSID, WMPSK)) {
    Serial.println("Config portal running");
  } else {
    Serial.println("Connected to WiFi network");
  }

  //OTA config
  ArduinoOTA.setPassword(OTAPSK);
  ArduinoOTA.setHostname(WIFI_CLOCK_NAME);

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    startMillis = 0;
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    unsigned int disp_prog = 10 - (progress / (total / 10));
    if (disp_prog > 0) {
      setWord(disp_prog - 1, true);
    } else {
      pixels.clear();
    }
  });

  ArduinoOTA.begin();

  // FreeRTOS taken starten
  xTaskCreate(wifiTask, "WiFiTask", 4096, NULL, 3, &wifiTaskHandle);
  xTaskCreate(ledTask, "LedTask", 2048, NULL, 1, &ledTaskHandle);
}

void loop() {
  ArduinoOTA.handle();
}
