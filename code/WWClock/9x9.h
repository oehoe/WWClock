#include <Adafruit_NeoPixel.h>
#include "Clock.h"

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

const int pixelPos[rows][cols] = {
  { 80, 79, 78, 77, 76, 75, 74, 73, 72 },
  { 63, 64, 65, 66, 67, 68, 69, 70, 71 },
  { 62, 61, 60, 59, 58, 57, 56, 55, 54 },
  { 45, 46, 47, 48, 49, 50, 51, 52, 53 },
  { 44, 43, 42, 41, 40, 39, 38, 37, 36 },
  { 27, 28, 29, 30, 31, 32, 33, 34, 35 },
  { 26, 25, 24, 23, 22, 21, 20, 19, 18 },
  { 9, 10, 11, 12, 13, 14, 15, 16, 17 },
  { 8, 7, 6, 5, 4, 3, 2, 1, 0 }
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

//NETHERLANDS
//K','W','A','R','T','T','I','E','N'],
//V','I','J','F','W','V','O','O','R'],
//O','V','E','R','I','H','A','L','F'],
//Z','E','S','V','F','V','I','J','F'],
//E','L','F','D','I','D','R','I','E'],
//V','I','E','R','B','T','I','E','N'],
//Z','E','V','E','N','A','C','H','T'],
//T','W','E','E','N','E','G','E','N'],
//T','W','A','A','L','F','U','U','R'],

class ClockNL : public Clock {
public:
  const int bitmap[22][rows] = {
    { 0, 0, 0, 0, 0, 0, 0b111, 0, 0 },                      // 1
    { 0, 0, 0, 0b1111, 0, 0, 0, 0, 0 },                     // 2
    { 0, 0, 0, 0, 0, 0b111100000, 0, 0, 0 },                // 3
    { 0, 0, 0, 0, 0b111100000, 0, 0, 0, 0 },                // 4
    { 0, 0, 0, 0, 0b1111, 0, 0, 0, 0 },                     // 5
    { 0, 0, 0, 0, 0, 0b111, 0, 0, 0 },                      // 6
    { 0, 0, 0, 0, 0, 0, 0, 0b111110000, 0 },                // 7
    { 0, 0, 0, 0b111100000, 0, 0, 0, 0, 0 },                // 8
    { 0, 0, 0, 0, 0, 0, 0, 0b11111, 0 },                    // 9
    { 0, 0, 0, 0, 0, 0, 0, 0, 0b111100000 },                // 10
    { 0, 0, 0, 0, 0, 0b111000, 0, 0, 0 },                   // 11
    { 0, 0, 0, 0, 0, 0, 0b111111000, 0, 0 },                // 12
    { 0, 0b111100000, 0, 0, 0, 0, 0, 0, 0 },                // five
    { 0b000001111, 0, 0, 0, 0, 0, 0, 0, 0 },                // ten
    { 0b111110000, 0, 0, 0, 0, 0, 0, 0, 0 },                // quarter
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },                          // twenty
    { 0, 0, 0b1111, 0, 0, 0, 0, 0, 0 },                     // half
    { 0, 0, 0b111100000, 0, 0, 0, 0, 0, 0 },                // past
    { 0, 0b1111, 0, 0, 0, 0, 0, 0, 0 },                     // to
    { 0, 0, 0, 0, 0, 0, 0, 0, 0b111 },                      // oclock
    { 0, 0b10000, 0b10000, 0b10000, 0b10000, 0, 0, 0, 0 },  //wifi
    { 0, 0, 0, 0, 0, 0, 0, 0, 0b1 }                         // notime
  };
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
      setClockState(bitmap[TWENTY], colors[0]);
      setClockState(bitmap[PAST], colors[1]);
    } else if (minute < 25) {
      setClockState(bitmap[TWENTY], colors[0]);
      setClockState(bitmap[FIVE], colors[0]);
      setClockState(bitmap[PAST], colors[1]);
    } else if (minute < 30) {
      setClockState(bitmap[HALF], colors[2]);
      setClockState(bitmap[PAST], colors[1]);
    } else if (minute < 35) {
      setClockState(bitmap[TWENTY], colors[0]);
      setClockState(bitmap[FIVE], colors[0]);
      setClockState(bitmap[TO], colors[1]);
    } else if (minute < 40) {
      setClockState(bitmap[TWENTY], colors[0]);
      setClockState(bitmap[TO], colors[1]);
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
};

//ENGLISH
//A','U','Q','U','A','R','T','E','R'],
//T','W','E','N','T','Y','T','E','N'],
//F','I','V','E','X','H','A','L','F'],
//P','A','S','T','O','F','T','W','O'],
//F','O','U','R','T','H','R','E','E'],
//S','E','V','E','N','F','I','V','E'],
//E','L','E','V','E','N','I','N','E'],
//O','N','E','I','G','H','T','E','N'],
//T','W','E','L','V','E','S','I','X'],

class ClockEN : public Clock {
public:
  const int bitmap[22][rows] = {
    { 0, 0, 0, 0, 0, 0, 0, 0b111000000, 0 },               //1
    { 0, 0, 0, 0b111, 0, 0, 0, 0, 0 },                     //2
    { 0, 0, 0, 0, 0b11111, 0, 0, 0, 0 },                   //3
    { 0, 0, 0, 0, 0b111100000, 0, 0, 0, 0 },               //4
    { 0, 0, 0, 0, 0, 0b1111, 0, 0, 0 },                    //5
    { 0, 0, 0, 0, 0, 0, 0, 0, 0b111 },                     //6
    { 0, 0, 0, 0, 0, 0b111110000, 0, 0, 0 },               //7
    { 0, 0, 0, 0, 0, 0, 0, 0b1111100, 0 },                 //8
    { 0, 0, 0, 0, 0, 0, 0b1111, 0, 0 },                    //9
    { 0, 0, 0, 0, 0, 0, 0, 0b111, 0 },                     //10
    { 0, 0, 0, 0, 0, 0, 0b111111000, 0, 0 },               //11
    { 0, 0, 0, 0, 0, 0, 0, 0, 0b111111000 },               // 12
    { 0, 0, 0b111100000, 0, 0, 0, 0, 0, 0 },               // five
    { 0, 0b111, 0, 0, 0, 0, 0, 0, 0 },                     // ten
    { 0b001111111, 0, 0, 0, 0, 0, 0, 0, 0 },               // quarter
    { 0, 0b111111000, 0, 0, 0, 0, 0, 0, 0 },               // twenty
    { 0, 0, 0b1111, 0, 0, 0, 0, 0, 0 },                    // half
    { 0, 0, 0, 0b111100000, 0, 0, 0, 0, 0 },               // past
    { 0, 0, 0, 0b110000, 0, 0, 0, 0, 0 },                  // to
    { 0, 0, 0, 0, 0, 0, 0, 0, 0 },                         // oclock
    { 0, 0b10000000, 0b10000000, 0, 0, 0b1100, 0, 0, 0 },  //wifi
    { 0, 0, 0, 0, 0, 0, 0, 0, 0b1 }                        // notime
  };
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
};
