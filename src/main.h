

#ifndef main_h
#define main_h

#include <Arduino.h>
#include <FastLED.h>


#define DEBUG
#define WIFI


//Pin Stuff
//---------
#define SDA 7
#define SCL 5

#define DATA_PIN 39
#define BUT_CTRL 9
#define BUT_A 3
#define BUT_B 11

#define STATUS_LED LED_BUILTIN //Assuming D2 here...

// How many leds in your strip?
#define NUM_LEDS 2


typedef enum{
    STATE_STARTUP,
    STATE_WAITCONN,
    STATE_LIVE,
    STATE_PAUSED,
    STATE_CAL_BUTTONS,
    STATE_CAL_GYRO,
    STATE_CAL_MAG,
    STATE_UPDATE,

}t_state;

typedef struct{
    float roll;
    float pitch;
    float yaw;
}t_sensorFusionData;



void setLED(uint8_t led,uint8_t r, uint8_t g, uint8_t b);
void setState(t_state newState);
void calibrate_buttons();

float GetCurrentYaw(void);
float GetCurrentPitch(void);
float GetCurrentRoll(void);
float GetCurrentAX(void);
float GetCurrentAY(void); 
float GetCurrentAZ(void);

#endif //main_h