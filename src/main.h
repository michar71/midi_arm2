

#ifndef main_h
#define main_h

#include <Arduino.h>
#include "FastLED.h"

#define DEBUG



#define BABOI_HW_VER 3

#define TH_CUTOFF 20000
#define TOUCH_TH_CTRL 23000
#define TOUCH_TH_A 41000
#define TOUCH_TH_B 27000
#define TOUCH_TH_C 30000

//Pin Stuff
//---------



#define SDA 4
#define SCL 5


#define DATA_PIN 10  //LED Pin
#define BUT_CTRL 2
#define BUT_A 3

#define ADC1_CH0 0
#define ADC1_CH1 1


#define ID_PIN 21 //Pull to GND for Touch pads


#define STATUS_LED 8 //Assuming D2 here...




typedef enum{
    STATE_STARTUP,
    STATE_WAITCONN,
    STATE_LIVE,
    STATE_PAUSED,
    STATE_CAL_BUTTONS,
    STATE_CAL_TENSION,
    STATE_CAL_GYRO,
    STATE_CAL_MAG,
    STATE_UPDATE,

}t_state;

typedef struct{
    float roll;
    float pitch;
    float yaw;
}t_sensorFusionData;



void setState(t_state newState);
void toggle_status_led(void);

#endif //main_h