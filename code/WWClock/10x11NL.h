#include <Adafruit_NeoPixel.h>

#define WIFI_CLOCK_NAME "WWClock"

//Url to retrieve time. Set correct timezone: http://worldtimeapi.org/timezones
#define TIME_URL "http://worldtimeapi.org/api/timezone/Europe/Amsterdam"

const int rows = 9;
const int cols = 9;

//set correct type of ledstrip. Ref: https://github.com/adafruit/Adafruit_NeoPixel
//For WS2812b: NEO_GRB
//For SK6812: NEO_GRBW
#define STRIP_TYPE NEO_GRBW
#define DATA_PIN 4
#define BRIGHTNESS 255  //Value between 0 and 255

Adafruit_NeoPixel pixels(rows* cols, DATA_PIN, STRIP_TYPE);

//set colors
int colors[5][4] = { { 255, 255, 50, 0 },     //color of five,ten,quarter,twenty
                     { 120, 255, 228, 0 },    //color of to,past
                     { 255, 137, 137, 0 },    //color of half
                     { 125, 255, 116, 0 },    //color of the hours
                     { 255, 255, 255, 0 } };  //color of IT IS and o'clock

#define FIVE 12
#define TEN 13
#define QUARTER 14
#define TWENTY 15
#define HALF 16
#define PAST 17
#define TO 18
#define OCLOCK 19
#define WIFI 20
#define NOTIME 21
#define ITIS 22

const int pixelPos[rows][cols] = {
  { 99, 100, 101, 102, 103, 104, 105, 106, 107, 108, 109 },
  { 98, 97, 96, 95, 94, 93, 92, 91, 90, 89, 88 },
  { 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87 },
  { 76, 75, 74, 73, 72, 71, 70, 69, 68, 67, 66 },
  { 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65 },
  { 54, 53, 52, 51, 50, 49, 48, 47, 46, 45, 44 },
  { 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43 },
  { 32, 31, 30, 29, 28, 27, 26, 25, 24, 23, 22 },
  { 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21 },
  { 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0 }
};

//NETHERLANDS
//H E T K I S A V I J F
//T I E N A W Z V O O R
//O V E R M I K W A R T
//H A L F S F M P V E R
//V O O R T I G E E N S
//T W E E A M C D R I E
//V I E R V I J F Z E S
//Z E V E N O N E G E N
//A C H T T I E N E L F
//T W A A L F P M U U R

const int bitmap[23][rows] = {
  { 0, 0, 0, 0, 0b1110, 0, 0, 0, 0, 0 },                         // 1
  { 0, 0, 0, 0, 0, 0b11110000000, 0, 0, 0, 0 },                  // 2
  { 0, 0, 0, 0, 0, 0b1111, 0, 0, 0, 0 },                         // 3
  { 0, 0, 0, 0, 0, 0, 0b11110000000, 0, 0, 0 },                  // 4
  { 0, 0, 0, 0, 0, 0, 0b1111000, 0, 0, 0 },                      // 5
  { 0, 0, 0, 0, 0, 0, 0b111, 0, 0, 0 },                          // 6
  { 0, 0, 0, 0, 0, 0, 0, 0b11111000000, 0, 0 },                  // 7
  { 0, 0, 0, 0, 0, 0, 0, 0, 0b11110000000, 0 },                  // 8
  { 0, 0, 0, 0, 0, 0, 0, 0b11111, 0, 0 },                        // 9
  { 0, 0, 0, 0, 0, 0, 0, 0, 0b1111000, 0 },                      // 10
  { 0, 0, 0, 0, 0, 0, 0, 0, 0b111, 0 },                          // 11
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b11111100000 },                  // 12
  { 0b1111, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                         // five
  { 0, 0b11110000000, 0, 0, 0, 0, 0, 0, 0, 0 },                  // ten
  { 0, 0, 0b11111, 0, 0, 0, 0, 0, 0, 0 },                        // quarter
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },                              // twenty
  { 0, 0, 0, 0b11110000000, 0, 0, 0, 0, 0, 0 },                  // half
  { 0, 0, 0, 0b1111, 0, 0, 0, 0, 0, 0 },                         // past
  { 0, 0, 0, 0, 0b11110000000, 0, 0, 0, 0, 0 },                  // to
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b111 },                          // oclock
  { 0, 0b100000, 0b100000, 0b100000, 0b100000, 0, 0, 0, 0, 0 },  //wifi
  { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0b1 },                            // notime
  { 0b11101100000, 0, 0, 0, 0, 0, 0, 0, 0, 0 }                   // itis
};

void setClockState(const int targetPos[rows], const int setColor[4]) {
  for (int a = 0; a < rows; a++) {
    for (int b = 0; b < cols; b++) {
      int y = (targetPos[a] >> (cols - 1) - b) & 1;
      if (y == 1) {
        pixels.setPixelColor(pixelPos[a][b], pixels.gamma32(pixels.Color(setColor[0], setColor[1], setColor[2], setColor[3])));
      }
    }
  }
}

void setWord(const int x, const bool clear) {
  if (clear) pixels.clear();
  setClockState(bitmap[x], colors[0]);
  pixels.show();
}

void setClock(int hour, int minute) {
  pixels.clear();
  if (minute > 14) hour = hour + 1;
  if (hour == 0) hour = 12;
  if (hour > 12) hour = hour - 12;
  setClockState(bitmap[hour - 1], colors[3]);

  setClockState(bitmap[ITIS], colors[4]);

  if (minute < 5) {
    setClockState(bitmap[FIVE], colors[0]);
    setClockState(bitmap[PAST], colors[1]);
  } else if (minute < 10) {
    setClockState(bitmap[TEN], colors[0]);
    setClockState(bitmap[PAST], colors[1]);
  } else if (minute < 15) {
    setClockState(bitmap[QUARTER], colors[0]);
    setClockState(bitmap[PAST], colors[1]);
  } else if (minute < 20) {
    setClockState(bitmap[TEN], colors[0]);
    setClockState(bitmap[TO], colors[1]);
    setClockState(bitmap[HALF], colors[2]);
  } else if (minute < 25) {
    setClockState(bitmap[FIVE], colors[0]);
    setClockState(bitmap[TO], colors[1]);
    setClockState(bitmap[HALF], colors[2]);
  } else if (minute < 30) {
    setClockState(bitmap[HALF], colors[2]);
  } else if (minute < 35) {
    setClockState(bitmap[FIVE], colors[0]);
    setClockState(bitmap[PAST], colors[1]);
    setClockState(bitmap[HALF], colors[2]);
  } else if (minute < 40) {
    setClockState(bitmap[TEN], colors[0]);
    setClockState(bitmap[PAST], colors[1]);
    setClockState(bitmap[HALF], colors[2]);
  } else if (minute < 45) {
    setClockState(bitmap[QUARTER], colors[0]);
    setClockState(bitmap[TO], colors[1]);
  } else if (minute < 50) {
    setClockState(bitmap[TEN], colors[0]);
    setClockState(bitmap[TO], colors[1]);
  } else if (minute < 55) {
    setClockState(bitmap[FIVE], colors[0]);
    setClockState(bitmap[TO], colors[1]);
  } else {
    setClockState(bitmap[OCLOCK], colors[4]);
  }
  pixels.show();
}