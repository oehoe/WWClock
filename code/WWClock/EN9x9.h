//ENGLISH 9x9

//////////////////
//START USER INPUT
//////////////////

//Clock name
#define WIFI_CLOCK_NAME "WWClock"

//Url to retrieve time. Set correct timezone: http://worldtimeapi.org/timezones
#define TIME_URL "http://worldtimeapi.org/api/timezone/Europe/Amsterdam"

//set correct type of ledstrip. Ref: https://github.com/adafruit/Adafruit_NeoPixel
//For WS2812b: NEO_GRB
//For SK6812: NEO_GRBW

//#define STRIP_TYPE NEO_GRB
#define STRIP_TYPE NEO_GRBW

//set true if ledstrip is rgbw ledstrip
const bool rgbw = false;

//set output pin for led data
#define LED_DATA_PIN 14

//SET Wifimanager network credentials
#define WMSSID "WWClock"

#define WMPSK "01234567"

//Set password for OTA updating from Arduino IDE in clock config file
#define OTAPSK "01234567"

//brightness not above 255
int brightness = 255;

//Select which animation to display. 0 is no animation. -1 is random animation.
int animationType = 0;
//Set interval for animation. 1 is every minute. 5 is every 5 minutes. 60 is every hour.
int animationInterval = 5;

//set colors
const int nr_of_colors = 5;
int colors[5][4] = {{255,255,50,0},//color of five,ten,quarter,twenty
                    {120,255,228,0},//color of to,past
                    {255,137,137,0},//color of half
                    {125,255,116,0},//color of the hours
                    {255,255,255,0}};//color of IT IS and o'clock


///////////////////
//END USER INPUT
///////////////////

const int rows = 9;
const int cols = 9;
int state[2][rows][cols][4];

const int pixelPos[9][9] = {
  {80,79,78,77,76,75,74,73,72},
  {63,64,65,66,67,68,69,70,71},
  {62,61,60,59,58,57,56,55,54},
  {45,46,47,48,49,50,51,52,53},
  {44,43,42,41,40,39,38,37,36},
  {27,28,29,30,31,32,33,34,35},
  {26,25,24,23,22,21,20,19,18},
  {9,10,11,12,13,14,15,16,17},
  {8,7,6,5,4,3,2,1,0}
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

const int wifi[rows] = {0,0b10000000,0b10000000,0,0,0b1100,0,0,0};
//indicate long time no time update
const int notime[rows] = {0,0,0,0,0,0,0,0,0b1};

const int itis[rows] = {0,0,0,0,0,0,0,0,0};
const int half[rows] = {0,0,0b1111,0,0,0,0,0,0};
const int five[rows] = {0,0,0b111100000,0,0,0,0,0,0};
const int ten[rows] = {0,0b111,0,0,0,0,0,0,0};
const int quarter[rows] = {0b001111111,0,0,0,0,0,0,0,0};
const int twenty[rows] = {0,0b111111000,0,0,0,0,0,0,0};
const int past[rows] = {0,0,0,0b111100000,0,0,0,0,0};
const int to[rows] = {0,0,0,0b110000,0,0,0,0,0};
const int oclock[rows] = {0,0,0,0,0,0,0,0,0};
const int hours[12][rows] = {{0,0,0,0,0,0,0,0b111000000,0},  //1
                             {0,0,0,0b111,0,0,0,0,0},//2
                             {0,0,0,0,0b11111,0,0,0,0},//3
                             {0,0,0,0,0b111100000,0,0,0,0},//4
                             {0,0,0,0,0,0b1111,0,0,0},//5
                             {0,0,0,0,0,0,0,0,0b111},//6
                             {0,0,0,0,0,0b111110000,0,0,0},//7
                             {0,0,0,0,0,0,0,0b1111100,0},//8
                             {0,0,0,0,0,0,0b1111,0,0},//9
                             {0,0,0,0,0,0,0,0b111,0},//10
                             {0,0,0,0,0,0,0b111111000,0,0},//11
                             {0,0,0,0,0,0,0,0,0b111111000}};//12
