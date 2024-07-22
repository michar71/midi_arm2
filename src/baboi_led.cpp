#include "main.h"
#include "baboi_led.h"
#include "baboi_sensors.h"
#include "hsv2rgb.h"

// Define the array of leds
CRGB leds[NUM_LEDS];


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
    leds[ii+2].r = pArray[ii*3];
    leds[ii+2].g = pArray[ii*3 + 1];
    leds[ii+2].b = pArray[ii*3 + 2];
  }
}


void updateLED(void)
{
  FastLED.show(MAX_LED_BRIGHTNESS);
}