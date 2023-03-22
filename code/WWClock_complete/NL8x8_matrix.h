//NETHERLANDS 8x8
#define CLOCK_LANG "NL"

//Url to retrieve time. Set correct timezone: http://worldtimeapi.org/timezones
#define TIME_URL "http://worldtimeapi.org/api/timezone/Europe/Amsterdam"

//set correct type of ledstrip. Ref: https://github.com/adafruit/Adafruit_NeoPixel
#define STRIP_TYPE NEO_GRB

//set true if ledstrip is rgbw ledstrip
const bool rgbw = false;

//set output pin for led data
#define LED_DATA_PIN 13

//Use brightness control based on light intensity.
const bool measureLight = false;

//set analog input pin for light measurement
#define LUX_PIN A0

//Settings for lux measurement -> led brightness
//max brightness not above 255
float max_brightness = 255;
float min_brightness = 15;
float lux_max_brightness = 2.5;
float lux_min_brightness = 1;
float lux_cutoff = 0.25;

//Select which animation to display. 0 is no animation. -1 is random animation.
int animationType = -1;
//Set interval for animation. 1 is every minute. 5 is every 5 minutes. 60 is every hour.
int animationInterval = 1;

//set colors
const int nr_of_colors = 5;
int colors[5][4] = {{150,255,150,0},//color of five,ten,quarter,twenty
                    {255,255,0,0},//color of to,past
                    {255,120,120,0},//color of half
                    {0,255,255,0},//color of the hours
                    {255,255,0,0}};//color of IT IS and o'clock

const int rows = 8;
const int cols = 8;

const int pixelPos[8][8] = {
  {0,1,2,3,4,5,6,7},
  {8,9,10,11,12,13,14,15},
  {16,17,18,19,20,21,22,23},
  {24,25,26,27,28,29,30,31},
  {32,33,34,35,36,37,38,39},
  {40,41,42,43,44,45,46,47},
  {48,49,50,51,52,53,54,55},
  {56,57,58,59,60,61,62,63}
};

//NETHERLANDS
//'K','W','A','R','T','I','E','N'],
//'V','I','J','F','V','O','O','R'],
//'O','V','E','R','H','A','L','F'],
//'A','C','H','T','T','I','E','N'],
//'V','V','T','W','A','A','L','F'],
//'I','I','Z','E','V','E','N','I'],
//'E','J','E','E','D','R','I','E'],
//'R','F','S','N','E','G','E','N'],

const int wifi[rows] = {0b1000000,0b1000000,0,0,0b1,0b1,0,0};
const int itis[rows] = {0,0,0,0,0,0,0,0};
const int half[rows] = {0,0,0b1111,0,0,0,0,0};
const int five[rows] = {0,0b11110000,0,0,0,0,0,0};
const int ten[rows] = {0b1111,0,0,0,0,0,0,0};
const int quarter[rows] = {0b11111000,0,0,0,0,0,0,0};
const int twenty[rows] = {0,0,0,0,0,0,0,0};
const int past[rows] = {0,0,0b11110000,0,0,0,0,0};
const int to[rows] = {0,0b1111,0,0,0,0,0,0};
const int oclock[rows] = {0,0,0,0,0,0,0,0};
const int hours[12][rows] = {{0,0,0,0,0,0b10000,0b10000,0b10000},  //1
                             {0,0,0,0b10000,0b10000,0b10000,0b10000,0},//2
                             {0,0,0,0,0,0,0b1111,0},//3
                             {0,0,0,0,0b10000000,0b10000000,0b10000000,0b10000000},//4
                             {0,0,0,0,0b1000000,0b1000000,0b1000000,0b1000000},//5
                             {0,0,0,0,0,0b100000,0b100000,0b100000},//6
                             {0,0,0,0,0,0b111110,0,0},//7
                             {0,0,0,0b11110000,0,0,0,0},//8
                             {0,0,0,0,0,0,0,0b11111},//9
                             {0,0,0,0b1111,0,0,0,0},//10
                             {0,0,0,0b10,0b11,0,0,0},//11
                             {0,0,0,0,0b111111,0,0,0}};//12