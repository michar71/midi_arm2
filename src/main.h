

#ifndef main_h
#define main_h

#include <Arduino.h>
#include "FastLED.h"

#define DEBUG
#define WIFI


#define BABOI_HW_VER 3

#define TH_CUTOFF 20000
#define TOUCH_TH_CTRL 23000
#define TOUCH_TH_A 41000
#define TOUCH_TH_B 27000
#define TOUCH_TH_C 30000

//Pin Stuff
//---------


#if BABOI_HW_VER == 2

#define SDA 7
#define SCL 5

#define GLOVE_SDA 40
//#define GLOVE_SCL_OLD 21   //Old SCL Pin
#define GLOVE_SCL 21

#define ADS1115_ALERT_PIN 37

#define DATA_PIN 39  //LED Pin
#define BUT_CTRL 9
#define BUT_A 3
#define BUT_B 10
#define BUT_C 2


#define ID_PIN 17 //Pull to GND for Touch pads


#define STATUS_LED LED_BUILTIN //Assuming D2 here...

#endif

#if BABOI_HW_VER == 3

#define SDA 4
#define SCL 5


#define DATA_PIN 10  //LED Pin
#define BUT_CTRL 2
#define BUT_A 3
#define BUT_B 6
#define BUT_C 7

#define ADC1_CH0 0
#define ADC1_CH1 1


#define ID_PIN 21 //Pull to GND for Touch pads


#define STATUS_LED 8 //Assuming D2 here...


#endif

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
void calibrate_buttons();
bool checkForTouchpad(void);
bool CheckTouchpadsForStrips(void);
void toggle_status_led(void);
uint16_t getTouchAnalogValue(uint8_t pad);




#endif //main_h