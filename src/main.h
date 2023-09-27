

#ifndef main_h
#define main_h

#include <Arduino.h>


String devicename = "BABOI";
int maj_ver = 1;
int min_ver = 0;


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


#endif //main_h