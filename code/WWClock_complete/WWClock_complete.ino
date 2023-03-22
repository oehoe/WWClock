#include <Arduino.h>
#include <ArduinoJson.h>
#include <Adafruit_NeoPixel.h> // https://github.com/adafruit/Adafruit_NeoPixel
#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFiUdp.h>
#include <ArduinoOTA.h>

//ESP8266
//#include <ESP8266HTTPClient.h>
//#include <WiFiClient.h>
//#include <ESP8266mDNS.h>
//#define B_RATE 74880

//ESP32
#include <WiFi.h>
#include <HTTPClient.h>
#include <ESPmDNS.h>
#define B_RATE 115200

const bool offline = false;
const int cs = 0;
const int ds = 1;
int hour, minute;

//////////////////
//START USER INPUT
//////////////////

//Uncomment correct clock config or create your own file

//#include "NL9x9.h"
#include "EN9x9.h"


//uncomment correct language file or create your own

#include "EN.h"
//#include "NL.h"

///////////////////
//END USER INPUT
///////////////////

//SET Wifimanager network credentials in clock config file
#ifndef WMSSID
#define WMSSID "WWClock"
#endif

#ifndef WMPSK
#define WMPSK "01234567"
#endif

//Set password for OTA updating from Arduino IDE in clock config file
#ifndef OTAPSK
#define OTAPSK "01234567"
#endif

//Url to retrieve time. Set correct timezone: http://worldtimeapi.org/timezones
//Set url in clock config file
#ifndef TIME_URL
#define TIME_URL "http://worldtimeapi.org/api/timezone/Europe/Amsterdam"
#endif

//set analog input for light measurement
//set pin in clock config file
#ifndef LUX_PIN
#define LUX_PIN A0
#endif

//Set datapin for ledstrip
//Set pin in clock config file
#ifndef LED_DATA_PIN
#define LED_DATA_PIN 5
#endif

//set ledstrip type
//set type in clock config file
#ifndef STRIP_TYPE
#define STRIP_TYPE NEO_GRB
#endif

#ifndef WIFI_CLOCK_NAME
#define WIFI_CLOCK_NAME "WWClock"
#endif

Adafruit_NeoPixel pixels(rows * cols, LED_DATA_PIN, STRIP_TYPE);

WiFiManager wm;

StaticJsonDocument<1024> doc;

unsigned long startMillis, newSeconds, seconds;

//animation vars
unsigned long startAnimation = 0L;
int animation = animationType;
int cc = 40;
int aniColors[10][4] = {{0,0,0,0},
                        {255,0,0,0},
                        {0,0,0,0},
                        {0,0,0,0},
                        {0,0,0,0},
                        {0,0,255,0},
                        {0,0,0,0},
                        {255,0,255,0},
                        {0,0,0,0},
                        {0,255,0,0}};

//Light measurement
int lux = 0;
int brightness = max_brightness;

void setup() {
  Serial.begin(B_RATE);

  //Ledstrip config
  pixels.begin();
  pixels.setBrightness(max_brightness);

  for (int a = 0; a < nr_of_colors; a++) {
    for (int c = 0; c < 4; c++) {
      aniColors[a * 2][c] = colors[a][c];
    }
  }

  setClockState(wifi,colors[0]);
  showState(cs);

  //WiFi config
  WiFi.mode(WIFI_STA);
  WiFi.hostname(WIFI_CLOCK_NAME);

  //reset wifi settings
  //wm.resetSettings();

  if (offline) {
    testAllWords();
    seconds = 290;
    startMillis = millis();
  } else {
    if (wm.autoConnect(WMSSID, WMPSK)){
      Serial.println("Connected to wifi network");
    } else {
      Serial.println("Configportal running");
    }
  }

  //OTA config
  ArduinoOTA.setPassword(OTAPSK);
  ArduinoOTA.setHostname(WIFI_CLOCK_NAME);

  ArduinoOTA.onStart([]() {
    String type;
    pixels.setBrightness(max_brightness);
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
  //Set brightness level
  if (measureLight && startAnimation == 0) {
    lux = measureLux();
    //Serial.println(lux);
    if (lux < lux_cutoff) {
      if (brightness != 0) {
        pixels.setBrightness(0);
        brightness = 0;      
      }
    } else if ((brightness == 0 && lux > (float)lux_cutoff * 1.2) || (brightness > 0 && lux > lux_cutoff)) {
      int lux_constrained = constrain(lux, lux_min_brightness, lux_max_brightness);
      int new_brightness = map(lux_constrained,lux_min_brightness,lux_max_brightness,min_brightness,max_brightness);
      //Serial.print(lux);Serial.print(" - ");Serial.print(lux_constrained);Serial.print(" - ");Serial.println(new_brightness);
      if (abs(brightness - new_brightness) > 20) {
        brightness = new_brightness;
        pixels.setBrightness(new_brightness);
      }
    }
  }

  ArduinoOTA.handle();

  //Load data from server every 15 minutes and on boot
  if (startMillis + 900000L < millis() || startMillis == 0) {
  //if (startMillis + 60000L < millis() || startMillis == 0) {
    if ((WiFi.status() == WL_CONNECTED)) {
      WiFiClient client;
      HTTPClient http;
      String url = String(TIME_URL);
      if (true) url = String(TIME_URL) + "&lux=" + String(lux) + "&bright=" + String(brightness);
      Serial.println("[HTTP] begin...\n");
      Serial.println("URL: " + url);
      if (http.begin(client, url)) {
        int httpCode = http.GET();
        if (httpCode == 200) {
          String payload = http.getString();
          Serial.print("Payload: ");Serial.println(payload);
          DeserializationError error = deserializeJson(doc, payload);
          if (!error) {
            startMillis = millis();
            String datetime = doc["datetime"];
            int hours = datetime.substring(11,13).toInt();
            int minutes = datetime.substring(14,16).toInt();
            seconds = (hours * 3600L) + (minutes * 60L) + payload.substring(17, 19).toInt();
            //seconds = 290;
            
            //if colordata in custom url.
            if (doc["colors"]) {
              for (int a = 0; a < nr_of_colors; a++) {
                int r, g, b;
                const char* thiscolor = doc["colors"][a];
                //HEX color to rgb
                sscanf(thiscolor, "#%02x%02x%02x", &r, &g, &b);
                Serial.println(String(thiscolor) + " - " + String(r) + " - " + String(g) + " - " + String(b));
                if (rgbw) {
                  //calculate white led if ledstrip is rgbw
                  int minimum = min(min(r, g), min(g, b));
                  colors[a][0] = r - minimum;
                  colors[a][1] = g - minimum;
                  colors[a][2] = b - minimum;
                  colors[a][3] = minimum;
                } else {
                  colors[a][0] = r;
                  colors[a][1] = g;
                  colors[a][2] = b;
                  colors[a][3] = 0;
                }
                for (int c = 0; c < 4; c++) {
                  aniColors[a * 2][c] = colors[a][c];
                }
              }
            }
          } else {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.f_str());
          }
        }
      }
    }
  }

  newSeconds = seconds + (millis() - startMillis) / 1000L;

  hour = int(newSeconds / 3600L) % 24;
  minute = (newSeconds % 3600L) / 60L;
  
  if (startMillis != 0) {
    resetState(cs);
    setClock();
    if (animation == 0) {   
      showState(cs);
      delay(900);
    } else if (animationType != 0) {
      if (animationType == -1 && (newSeconds + 30) % 60 == 0) {
        animation = random(1,4);
        delay(1000);
      }
      if ((minute + 1) % animationInterval == 0 && newSeconds % 60 == 56 && startAnimation == 0) {
        copyClockState();
        cc = 40;
        startAnimation = millis();
      }
      if (startAnimation != 0) {
        if (animation == 1) animationOne();
        if (animation == 2) animationTwo();
        if (animation == 3) animationThree();
        showState(ds);
      } else {
        showState(cs);
        delay(900);
      }
    }
  }

  //delay(10);
}


void showState(int s) {
  pixels.clear();
  for (int a = 0; a < rows; a++) {
    for (int b = 0; b < cols; b++) {
      pixels.setPixelColor(pixelPos[a][b], pixels.gamma32(pixels.Color(state[s][a][b][0], state[s][a][b][1], state[s][a][b][2], state[s][a][b][3])));
    }
  }
  pixels.show();
}

float measureLux() {
  int output = 0;
  
  for (int a = 0; a < 5; a++) {
    int sensorValue = analogRead(LUX_PIN);
    output = output + sensorValue;
    delay(50);
  }
  return output / 5;
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
    int max = 40 - ((timing - 3) * 6);
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


bool compareColors(int m, int n) {
  bool same = true;
  for (int o = 0; o < 4; o++) {
    if (state[ds][m][n][o] != state[cs][m][n][o]) {
    //if (abs(state[ds][m][n][o] - state[cs][m][n][o]) > 3) {
      same = false;
      break;
    }
  }
  return same;
}

void WheelFour(int a, int b, int x) {
  if (x < 50) {
    x = x - 0;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[9][0] - factor*(aniColors[9][0] - aniColors[0][0]));
    state[ds][a][b][1] = abs(aniColors[9][1] - factor*(aniColors[9][1] - aniColors[0][1]));
    state[ds][a][b][2] = abs(aniColors[9][2] - factor*(aniColors[9][2] - aniColors[0][2]));
    state[ds][a][b][3] = abs(aniColors[9][3] - factor*(aniColors[9][3] - aniColors[0][3]));
  } else if (x < 100) {
    x = x - 50;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[0][0] - factor*(aniColors[0][0] - aniColors[1][0]));
    state[ds][a][b][1] = abs(aniColors[0][1] - factor*(aniColors[0][1] - aniColors[1][1]));
    state[ds][a][b][2] = abs(aniColors[0][2] - factor*(aniColors[0][2] - aniColors[1][2]));
    state[ds][a][b][3] = abs(aniColors[0][3] - factor*(aniColors[0][3] - aniColors[1][3]));
  } else if (x < 150) {
    x = x - 100;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[1][0] - factor*(aniColors[1][0] - aniColors[2][0]));
    state[ds][a][b][1] = abs(aniColors[1][1] - factor*(aniColors[1][1] - aniColors[2][1]));
    state[ds][a][b][2] = abs(aniColors[1][2] - factor*(aniColors[1][2] - aniColors[2][2]));
    state[ds][a][b][3] = abs(aniColors[1][3] - factor*(aniColors[1][3] - aniColors[2][3]));
  } else if (x < 200) {
    x = x - 150;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[2][0] - factor*(aniColors[2][0] - aniColors[3][0]));
    state[ds][a][b][1] = abs(aniColors[2][1] - factor*(aniColors[2][1] - aniColors[3][1]));
    state[ds][a][b][2] = abs(aniColors[2][2] - factor*(aniColors[2][2] - aniColors[3][2]));
    state[ds][a][b][3] = abs(aniColors[2][3] - factor*(aniColors[2][3] - aniColors[3][3]));
  } else if (x < 250) {
    x = x - 200;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[3][0] - factor*(aniColors[3][0] - aniColors[4][0]));
    state[ds][a][b][1] = abs(aniColors[3][1] - factor*(aniColors[3][1] - aniColors[4][1]));
    state[ds][a][b][2] = abs(aniColors[3][2] - factor*(aniColors[3][2] - aniColors[4][2]));
    state[ds][a][b][3] = abs(aniColors[3][3] - factor*(aniColors[3][3] - aniColors[4][3]));
  } else if (x < 300) {
    x = x - 250;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[4][0] - factor*(aniColors[4][0] - aniColors[5][0]));
    state[ds][a][b][1] = abs(aniColors[4][1] - factor*(aniColors[4][1] - aniColors[5][1]));
    state[ds][a][b][2] = abs(aniColors[4][2] - factor*(aniColors[4][2] - aniColors[5][2]));
    state[ds][a][b][3] = abs(aniColors[4][3] - factor*(aniColors[4][3] - aniColors[5][3]));
  } else if (x < 350) {
    x = x - 300;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[5][0] - factor*(aniColors[5][0] - aniColors[6][0]));
    state[ds][a][b][1] = abs(aniColors[5][1] - factor*(aniColors[5][1] - aniColors[6][1]));
    state[ds][a][b][2] = abs(aniColors[5][2] - factor*(aniColors[5][2] - aniColors[6][2]));
    state[ds][a][b][3] = abs(aniColors[5][3] - factor*(aniColors[5][3] - aniColors[6][3]));
  } else if (x < 400) {
    x = x - 350;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[6][0] - factor*(aniColors[6][0] - aniColors[7][0]));
    state[ds][a][b][1] = abs(aniColors[6][1] - factor*(aniColors[6][1] - aniColors[7][1]));
    state[ds][a][b][2] = abs(aniColors[6][2] - factor*(aniColors[6][2] - aniColors[7][2]));
    state[ds][a][b][3] = abs(aniColors[6][3] - factor*(aniColors[6][3] - aniColors[7][3]));
  } else if (x < 450) {
    x = x - 400;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[7][0] - factor*(aniColors[7][0] - aniColors[8][0]));
    state[ds][a][b][1] = abs(aniColors[7][1] - factor*(aniColors[7][1] - aniColors[8][1]));
    state[ds][a][b][2] = abs(aniColors[7][2] - factor*(aniColors[7][2] - aniColors[8][2]));
    state[ds][a][b][3] = abs(aniColors[7][3] - factor*(aniColors[7][3] - aniColors[8][3]));
  } else if (x < 500) {
    x = x - 450;
    float factor = (float)x / 49;
    state[ds][a][b][0] = abs(aniColors[8][0] - factor*(aniColors[8][0] - aniColors[9][0]));
    state[ds][a][b][1] = abs(aniColors[8][1] - factor*(aniColors[8][1] - aniColors[9][1]));
    state[ds][a][b][2] = abs(aniColors[8][2] - factor*(aniColors[8][2] - aniColors[9][2]));
    state[ds][a][b][3] = abs(aniColors[8][3] - factor*(aniColors[8][3] - aniColors[9][3]));
  }
}

void animationThree() {
  long timing = (millis() - startAnimation) / 1000;
  bool allSame = true;
  for (int d = 0; d < rows; d++) {
      for (int e = 0; e < cols; e++) {
        bool thisSame = compareColors(d,e);
        if (timing < 5 || !thisSame) {
          allSame = false;
          WheelFour(d,e,(cc + d * cols + e) % 500);
          if (cc == 40) {
            showState(ds);
            delay(25);
          }
        }
      }
  }
  if (allSame) startAnimation = 0;
  cc += 1;
  delay(12);
}

void testAllWords() {
    resetState(cs);
    int pause = 500;
    setClockState(five,colors[0]);showState(cs);delay(pause);resetState(cs);
    setClockState(ten,colors[0]);showState(cs);delay(pause);resetState(cs);
    setClockState(quarter,colors[0]);showState(cs);delay(pause);resetState(cs);
    setClockState(twenty,colors[0]);
    setClockState(past,colors[1]);showState(cs);delay(pause);resetState(cs);
    setClockState(to,colors[1]);showState(cs);delay(pause);resetState(cs);
    setClockState(half,colors[2]);
    setClockState(itis,colors[4]);
    setClockState(oclock,colors[4]);showState(cs);delay(pause);resetState(cs);
    for (int a = 0; a < 12; a++) {
      setClockState(hours[a],colors[3]);showState(cs);delay(pause);resetState(cs);
    }
}
