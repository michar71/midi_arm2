#include "main.h"
#include "baboi_led.h"
#include "baboi_sensors.h"


// Define the array of leds
CRGB leds[NUM_LEDS];


typedef enum{
  LED_NONE,
  LED_CYLON,
  LED_ACC,
  LED_GYRO,
  LED_ACC_GYRO,
  LED_ARTNET
}T_LED_PROGRAM;

T_LED_PROGRAM current_led_program = LED_NONE;        

void setLED(uint8_t led,uint8_t r, uint8_t g, uint8_t b)
{
    // Turn the LED on, then pause
  leds[led].r= r/6;
  leds[led].g= g/6;
  leds[led].b= b/6;
  FastLED.show();
}


void led_test(void)
{
  for(int ii=0;ii<NUM_LEDS;ii++)
  {
    setLED(ii,64,0,0);
  }
  delay(100);
  for(int ii=0;ii<NUM_LEDS;ii++)
  {
    setLED(ii,0,64,0);
  }
  delay(100);
  for(int ii=0;ii<NUM_LEDS;ii++)
  {
    setLED(ii,0,0,64);
  }
  delay(100);
  for(int ii=0;ii<NUM_LEDS;ii++)
  {
    setLED(ii,0,0,0);
  }
}


void led_sweep(void)
{
  static byte count = 0;

  led_control(255, 255, 255, 0, count);
  count++;
  if (count > NUM_STRIP)
    count = 0;
}

//Maps it to 0..31
//Set everything else to black
void led_control(byte r, byte g, byte b, byte start, byte end)
{
  start = start + 2;
  end = end + 2;
  if (end > NUM_STRIP-1)
    end = NUM_LEDS-1;

  for(int ii=2;ii<=start-1;ii++)
  {
    setLED(ii,0,0,0);
  }
  for(int ii=start;ii<=end;ii++)
  {
    setLED(ii,r,g,b);
  }
  for(int ii=end+1;ii<NUM_LEDS-1;ii++)
  {
    setLED(ii,0,0,0);
  }  
}

void handle_led(void)
{
    //Do LED Stuff
    switch (current_led_program)
    {
      case LED_NONE:
      {
        led_control(0, 0, 0, 0, NUM_STRIP);        
        break;
      }
      case LED_GYRO:
      {
        byte r = (byte)round(map(mpu_GetCurrentYaw(),-PI,PI,0,255));
        byte g = (byte)round(map(mpu_GetCurrentPitch(),-PI,PI,0,255));
        byte b = (byte)round(map(mpu_GetCurrentRoll(),-PI,PI,0,255));
        led_control(r, g, b, 0, NUM_STRIP);
        break;
      }
    }
}

void set_led_program(int prog)
{
  current_led_program = (T_LED_PROGRAM)prog;
}

void led_init(void)
{
    //Init and test LED's
   FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed
}