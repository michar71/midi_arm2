#include "main.h"
#include "baboi_led.h"
#include "baboi_sensors.h"
#include "hsv2rgb.h"

// Define the array of leds
CRGB leds[NUM_LEDS];

const uint8_t  gamma8[] = {
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
    0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  1,  1,  1,  1,
    1,  1,  1,  1,  1,  1,  1,  1,  1,  2,  2,  2,  2,  2,  2,  2,
    2,  3,  3,  3,  3,  3,  3,  3,  4,  4,  4,  4,  4,  5,  5,  5,
    5,  6,  6,  6,  6,  7,  7,  7,  7,  8,  8,  8,  9,  9,  9, 10,
   10, 10, 11, 11, 11, 12, 12, 13, 13, 13, 14, 14, 15, 15, 16, 16,
   17, 17, 18, 18, 19, 19, 20, 20, 21, 21, 22, 22, 23, 24, 24, 25,
   25, 26, 27, 27, 28, 29, 29, 30, 31, 32, 32, 33, 34, 35, 35, 36,
   37, 38, 39, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 50,
   51, 52, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 66, 67, 68,
   69, 70, 72, 73, 74, 75, 77, 78, 79, 81, 82, 83, 85, 86, 87, 89,
   90, 92, 93, 95, 96, 98, 99,101,102,104,105,107,109,110,112,114,
  115,117,119,120,122,124,126,127,129,131,133,135,137,138,140,142,
  144,146,148,150,152,154,156,158,160,162,164,167,169,171,173,175,
  177,180,182,184,186,189,191,193,196,198,200,203,205,208,210,213,
  215,218,220,223,225,228,231,233,236,239,241,244,247,249,252,255 };


#define MAX_LED_BRIGHTNESS 255

void setLED(uint8_t led,uint8_t r, uint8_t g, uint8_t b,bool update)
{
  leds[led].r= r;
  leds[led].g= g;
  leds[led].b= b;
  if (update)
    FastLED.show(MAX_LED_BRIGHTNESS);
}

void led_test(void)
{

  for(int ii=0;ii<NUM_LEDS;ii++)
  {
    setLED(ii,MAX_LED_BRIGHTNESS,0,0,true);
  }
  delay(100);
  for(int ii=0;ii<NUM_LEDS;ii++)
  {
    setLED(ii,0,MAX_LED_BRIGHTNESS,0,true);
  }
  delay(100);
  for(int ii=0;ii<NUM_LEDS;ii++)
  {
    setLED(ii,0,0,MAX_LED_BRIGHTNESS,true);
  }
  delay(100);
  for(int ii=0;ii<NUM_LEDS;ii++)
  {
    setLED(ii,0,0,0,true);
  }
}



void led_init(void)
{
    //Init and test LED's
   FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
}

void SetLEDsFromArray(uint8_t *pArray)
{
  //Copy data
  for (uint8_t ii=0;ii<16;ii++)
  {
    leds[ii+2].r = gamma8[pArray[ii*3]];
    leds[ii+2].g = gamma8[pArray[ii*3 + 1]];
    leds[ii+2].b = gamma8[pArray[ii*3 + 2]];
  }
}


void updateLED(void)
{
  FastLED.show(MAX_LED_BRIGHTNESS);
}