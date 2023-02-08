#include <Arduino.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h>
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//////////////////
//START USER INPUT
//////////////////

//Uncomment correct clock config or create your own file
#include "NL8x8_matrix.h"
//#include "NL9x9.h"

//SET Wifimanager network credentials
#define WMSSID "WWClock"
#define WMPSK "01234567"

//Set password for OTA updating from Arduino IDE
#define OTAPSK "01234567"

//Url to retrieve time from. Set correct timezone: http://worldtimeapi.org/timezones
String timeUrl = "http://worldtimeapi.org/api/timezone/America/Cancun";

//Set datapin for ledstrip
const int ledDataPin = 13;

//set true if ledstrip is rgbw ledstrip
const bool rgbw = false;

//set colors
int colors[5][4] = {{150,255,150,0},//color of five,ten,quarter,twenty
                    {255,255,0,0},//color of to,past
                    {255,120,120,0},//color of half
                    {0,255,255,0},//color of the hours
                    {255,255,0,0}};//color of IT IS and o'clock


//set correct type of ledstrip. Ref: https://github.com/adafruit/Adafruit_NeoPixel
//   NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
//   NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
//   NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
//   NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
//   NEO_RGBW    Pixels are wired for RGBW bitstream (NeoPixel RGBW products)
Adafruit_NeoPixel pixels(rows * cols, ledDataPin, NEO_GRB + NEO_KHZ800);

//Select which animation to display. 0 is no animation. -1 is random animation
int animationType = -1;
int animationInterval = 1;

///////////////////
//END USER INPUT
///////////////////

WiFiManager wm;

StaticJsonDocument<1024> doc;

const int cs = 0;
const int ds = 1;
int state[2][rows][cols][4];

int hour, minute;
unsigned long startMillis, newSeconds, seconds;
unsigned long startAnimation = 0L;
int animation = animationType;


void setup() {
  Serial.begin(74880);

  //Ledstrip config
  pixels.begin();
  pixels.setBrightness(140);

  setClockState(wifi,colors[0]);
  showState(cs);

  //WiFi config
  WiFi.mode(WIFI_STA);

  if (wm.autoConnect(WMSSID, WMPSK)){
        Serial.println("Connected to wifi network");
  } else {
        Serial.println("Configportal running");
  }

  //OTA config
  ArduinoOTA.setPassword(OTAPSK);

  ArduinoOTA.onStart([]() {
    String type;
    if (ArduinoOTA.getCommand() == U_FLASH) {
      type = "sketch";
    } else {  // U_FS
      type = "filesystem";
    }
    Serial.println("Start updating " + type);
  });

  ArduinoOTA.onEnd([]() {
    Serial.println("\nEnd");
  });

  ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
    Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    unsigned int disp_prog = 10 - (progress / (total / 10));
    if (disp_prog > 0) {
        resetState(cs);
        setClockState(hours[disp_prog - 1],colors[3]);
        showState(cs);
    }
  });

  ArduinoOTA.onError([](ota_error_t error) {
    Serial.printf("Error[%u]: ", error);
    if (error == OTA_AUTH_ERROR) {
      Serial.println("Auth Failed");
    } else if (error == OTA_BEGIN_ERROR) {
      Serial.println("Begin Failed");
    } else if (error == OTA_CONNECT_ERROR) {
      Serial.println("Connect Failed");
    } else if (error == OTA_RECEIVE_ERROR) {
      Serial.println("Receive Failed");
    } else if (error == OTA_END_ERROR) {
      Serial.println("End Failed");
    }
  });

  ArduinoOTA.begin();
}

void loop() {
  ArduinoOTA.handle();

  //Load data from server every 15 minutes and on boot
  if (startMillis + 900000L < millis() || startMillis == 0) {
    if ((WiFi.status() == WL_CONNECTED)) {
      WiFiClient client;
      HTTPClient http;
      Serial.print("[HTTP] begin...\n");
      if (http.begin(client, timeUrl)) {
        int httpCode = http.GET();
        if (httpCode == 200) {
          String payload = http.getString();
          Serial.print("Paload: ");Serial.println(payload);
          DeserializationError error = deserializeJson(doc, payload);
          if (!error) {
            startMillis = millis();
            String datetime = doc["datetime"];
            int hours = datetime.substring(11,13).toInt();
            int minutes = datetime.substring(14,16).toInt();
            seconds = (hours * 3600L) + (minutes * 60L) + payload.substring(17, 19).toInt();
          } else {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
          }
        }
      }
    } else if (startMillis != 0) {
      setClockState(wifi,colors[0]);
      showState(cs);
    }
  }

  newSeconds = seconds + (millis() - startMillis) / 1000L;

  hour = int(newSeconds / 3600L) % 24;
  minute = (newSeconds % 3600L) / 60L;
  
  if (startMillis != 0) {
    setClockNL();
    if (animation == 0) {   
      showState(cs);
      delay(900);
    } else if (animationType != 0) {
      if (animationType == -1 && (newSeconds + 30) % 60 == 0) {
        animation = random(1,3);
        Serial.println(animation);
        delay(1000);
      }
      if ((minute + 1) % animationInterval == 0 && newSeconds % 60 == 56 && startAnimation == 0) {
        copyClockState();
        startAnimation = millis();
      }
      if (startAnimation != 0) {
        if (animation == 1) animationOne();
        if (animation == 2) animationTwo();
        showState(ds);
      } else {
        showState(cs);
      }
    }
  }

  //delay(10);
}


void showState(int s) {
  pixels.clear();
  for (int a = 0; a < rows; a++) {
    for (int b = 0; b < cols; b++) {
      if (rgbw) {
        pixels.setPixelColor(pixelPos[a][b], pixels.gamma32(pixels.Color(state[s][a][b][0], state[s][a][b][1], state[s][a][b][2], state[s][a][b][3])));
      } else {
        pixels.setPixelColor(pixelPos[a][b], pixels.gamma32(pixels.Color(state[s][a][b][0], state[s][a][b][1], state[s][a][b][2])));
      }
    }
  }
  pixels.show();
}

void setClockNL() {

  resetState(cs);
  //NETHERLANDS
  if (minute > 14) hour = hour + 1;
  if (hour == 0) hour = 12;
  if (hour > 12) hour = hour - 12;
  setClockState(hours[hour - 1],colors[3]);

  if (minute < 5) {
    setClockState(five,colors[0]);
    setClockState(past,colors[1]);
  } else if (minute < 10) {
    setClockState(ten,colors[0]);
    setClockState(past,colors[1]);
  } else if (minute < 15) {
    setClockState(quarter,colors[0]);
    setClockState(past,colors[1]);
  } else if (minute < 20) {
    setClockState(ten,colors[0]);
    setClockState(to,colors[1]);
    setClockState(half,colors[2]);
  } else if (minute < 25) {
    setClockState(five,colors[0]);
    setClockState(to,colors[1]);
    setClockState(half,colors[2]);
  } else if (minute < 30) {
    setClockState(half,colors[2]);
  } else if (minute < 35) {
    setClockState(five,colors[0]);
    setClockState(past,colors[1]);
    setClockState(half,colors[2]);
  } else if (minute < 40) {
    setClockState(ten,colors[0]);
    setClockState(past,colors[1]);
    setClockState(half,colors[2]);
  } else if (minute < 45) {
    setClockState(quarter,colors[0]);
    setClockState(to,colors[1]);
  } else if (minute < 50) {
    setClockState(ten,colors[0]);
    setClockState(to,colors[1]);
  } else if (minute < 55) {
    setClockState(five,colors[0]);
    setClockState(to,colors[1]);
  } else {
    setClockState(oclock,colors[4]);
  }
}

void setClockState(const int targetPos[rows], const int setColor[4]) {
  for (int a = 0; a < rows; a++) {
    for (int b = 0; b < cols; b++) {
      int y = (targetPos[a] >> (cols - 1) - b) & 1;
      if (y == 1) {
        state[cs][a][b][0] = setColor[0];
        state[cs][a][b][1] = setColor[1];
        state[cs][a][b][2] = setColor[2];
        state[cs][a][b][3] = setColor[3];
      }
    }
  }
}


void copyClockState() {
  for (int a = 0; a < rows; a++) {
    for (int b = 0; b < cols; b++) {
      for (int c = 0; c < 4; c++) {
        state[ds][a][b][c] = state[cs][a][b][c];
      }
    }
  }
}

void resetState(int clearState) {
  for (int a = 0; a < rows; a++) {
    for (int b = 0; b < cols; b++) {
      state[clearState][a][b][0] = 0;
      state[clearState][a][b][1] = 0;
      state[clearState][a][b][2] = 0;
      state[clearState][a][b][3] = 0;
    }
  }
}


void animationOne() {
  int timing = millis() - startAnimation;
  if (timing < 3000) {
    float factor = (float)timing / 3000;
    for (int a = 0; a < rows; a++) {
      for (int b = 0; b < cols; b++) {
        if (state[cs][a][b][0] + state[cs][a][b][1] + state[cs][a][b][2] + state[cs][a][b][3] > 0) {
          for (int c = 0; c < 4; c++) {
            state[ds][a][b][c] = abs(state[cs][a][b][c] - factor*(state[cs][a][b][c] - colors[3][c]));
          }
        }
      }
    }
  } else if (timing > 4000 && timing < 6000) {
    float stepsize = 2000 / (float)(rows + cols + 4);
    int step = (timing - 4000) / stepsize;
    for (int a = 0; a < rows; a++) {
      for (int b = 0; b < cols; b++) {
        int d = step - b;
        if (d == a) {
          for (int c = 0; c < 4; c++) {
            state[ds][a][b][c] = colors[3][c];
          }
        } else if (d - 1 > a && state[cs][a][b][0] + state[cs][a][b][1] + state[cs][a][b][2] + state[cs][a][b][3] > 0) {
          for (int c = 0; c < 4; c++) {
            state[ds][a][b][c] = colors[3][c];
          }
        } else if (d - 2 == a) {
          for (int c = 0; c < 4; c++) {
            state[ds][a][b][c] = (int)(colors[3][c] * 0.7);
          }
        } else if (d - 3 == a) {
          for (int c = 0; c < 4; c++) {
            state[ds][a][b][c] = (int)(colors[3][c] * 0.5);
          }
        } else if (d - 4 == a) {
          for (int c = 0; c < 4; c++) {
            state[ds][a][b][c] = 0;
          }
        }
      }
    }
  } else if (timing > 7000 && timing < 10000) {
    float factor = (float)(timing - 7000) / 3000;
    for (int a = 0; a < rows; a++) {
      for (int b = 0; b < cols; b++) {
        if (state[cs][a][b][0] + state[cs][a][b][1] + state[cs][a][b][2] + state[cs][a][b][3] > 0) {
          for (int c = 0; c < 4; c++) {
            state[ds][a][b][c] = abs(colors[3][c] - factor*(colors[3][c] - state[cs][a][b][c]));
          }
        }
      }
    }
  } else if (timing > 11000) {
    startAnimation = 0;
  }
  
}


void animationTwo() {
  long timing = (millis() - startAnimation) / 1000;
  delay(120);
  //int max = 40000 / timing;
  if (timing > 3) {
    bool allSame = true;
    int max = 40 - (timing - 3 * 6);
    if (max < 4) max = 4;
    for (int a = 0; a < rows; a++) {
      for (int b = 0; b < cols; b++) {    
        long rnd = random(max);
        bool thisSame = compareColors(a,b);
        if (timing < 5 || !thisSame) {
          allSame = false;
          for (int c = 0; c < 4; c++) {
            if (rnd < 5) {
              state[ds][a][b][c] = colors[rnd][c];
            } else {
              if (rnd < 20) state[ds][a][b][c] = 0;
            }
          }
        }
      }
    }
    if (allSame) startAnimation = 0;
  }
}


bool compareColors(int a, int b) {
  bool same = true;
  for (int c = 0; c < 4; c++) {
    if (state[ds][a][b][c] != state[cs][a][b][c]) {
      same = false;
      break;
    }
  }
  return same;
}