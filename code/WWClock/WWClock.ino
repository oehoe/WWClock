#include <WiFi.h>
#include <HTTPClient.h>
#include <WiFiManager.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <ArduinoOTA.h>
#include "Clock.h"
#include "9x9.h"

#define WIFI_CLOCK_NAME "WWClockNiels"

//Wifi manager SSID and Password
#define WMSSID "WWClock"
#define WMPSK "01234567"

//Set password for OTA updating from Arduino IDE in clock config file
#define OTAPSK "01234567"

#define B_RATE 115200
//Url to retrieve time. Set correct timezone: http://worldtimeapi.org/timezones
#define TIME_URL "http://worldtimeapi.org/api/timezone/Europe/Amsterdam"

Preferences preferences;
WiFiManager wm;
String lang;
Clock* thisClock;
volatile unsigned long startMillis, seconds;
bool shouldSaveConfig = false;
StaticJsonDocument<1024> doc;

TaskHandle_t ledTaskHandle;
TaskHandle_t wifiTaskHandle;

void saveConfigCallback() {
  shouldSaveConfig = true;
  Serial.println("Should save config");
}

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
              fetchInterval = 15UL * 60UL * 1000UL;  // reset naar 15 min
            } else {
              Serial.print(F("deserializeJson() failed: "));
              Serial.println(error.f_str());
              fetchInterval = 20UL * 1000UL;  // probeer sneller opnieuw
            }
          } else {
            Serial.printf("HTTP error: %d\n", httpCode);
            fetchInterval = 10UL * 1000UL;  // probeer sneller opnieuw
          }
          http.end();
        } else {
          Serial.println("No http begin");
          fetchInterval = 20UL * 1000UL;  // probeer sneller opnieuw
        }
      }
    } else {
      Serial.println("WiFi niet verbonden!");
      fetchInterval = 20UL * 1000UL;  // sneller opnieuw zodra wifi terug is
    }

    //Serial.printf("Free heap: %u\n", ESP.getFreeHeap());
    vTaskDelay(100 / portTICK_PERIOD_MS);  // CPU ademruimte
  }
}

void ledTask(void* parameter) {
  for (;;) {
    if (startMillis != 0) {
      unsigned long newSeconds = seconds + (millis() - startMillis) / 1000L;
      int hour = int(newSeconds / 3600L) % 24;
      int minute = (newSeconds % 3600L) / 60L;
      thisClock->setClock(hour, minute);


      //indicate long time (21600000 = 6 hours) no timing. Red if not connected. Blue if connected but unable to load url.
      if (startMillis + 21600000 < millis()) {
        thisClock->setWord(NOTIME, false);
      }
    }
    vTaskDelay(500 / portTICK_PERIOD_MS);
  }
}

void setup() {
  Serial.begin(B_RATE);

  pixels.setBrightness(BRIGHTNESS);

  // Initialize Preferences
  preferences.begin("wwclock", false);

  // Load stored language or default
  lang = preferences.getString("lang", "EN");  // default "EN"
  if (lang == "NL") {
    ClockNL tmpClock;
    tmpClock.setWord(WIFI, true);
  } else {
    ClockEN tmpClock;
    tmpClock.setWord(WIFI, true);
  }

  WiFi.mode(WIFI_STA);
  WiFi.hostname(WIFI_CLOCK_NAME);
  WiFi.setSleep(false);

  // Prepare buffer for WiFiManagerParameter
  char langBuffer[3];
  lang.toCharArray(langBuffer, sizeof(langBuffer));

  // Setup WiFiManager
  wm.setSaveConfigCallback(saveConfigCallback);
  WiFiManagerParameter custom_lang("lang", "Language", langBuffer, sizeof(langBuffer));
  wm.addParameter(&custom_lang);

  if (!wm.autoConnect(WMSSID, WMPSK)) {
    Serial.println("Config portal running");
  } else {
    Serial.println("Connected to WiFi network");
  }

  // Read value from WiFiManagerParameter
  lang = String(custom_lang.getValue());
  Serial.printf("Selected language: %s\n", lang.c_str());

  if (lang == "NL") {
    thisClock = new ClockNL();
  } else {
    thisClock = new ClockEN();
  }

  // Save updated language if needed
  if (shouldSaveConfig) {
    preferences.putString("lang", lang);
    Serial.println("Language saved to Preferences");
  }

  //OTA config
  ArduinoOTA.setPassword(OTAPSK);
  ArduinoOTA.setHostname(WIFI_CLOCK_NAME);

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    startMillis = 0;
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    unsigned int disp_prog = 10 - (progress / (total / 10));
    if (disp_prog > 0) {
      thisClock->setWord(disp_prog - 1, true);
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
