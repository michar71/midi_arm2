
#ifndef baboi_led_h
#define baboi_led_h

#include <Arduino.h>


// How many leds in your strip?
#define NUM_LEDS 3
#define NUM_STRIP (NUM_LEDS-2) //0..31

void setLED(uint8_t led,uint8_t r, uint8_t g, uint8_t b,bool update = false);
void led_test(void);
void led_control(byte r, byte g, byte b, byte start, byte end);
void handle_led(void);
void led_init(void);
void updateLED(void);
#endif