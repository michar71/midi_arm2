

#ifndef main_h
#define main_h

#include <Arduino.h>
#include "FastLED.h"

#define DEBUG
#define WIFI

#define TH_CUTOFF 20000


//Pin Stuff
//---------
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
void toggle_status_led(void);




#endif //main_h