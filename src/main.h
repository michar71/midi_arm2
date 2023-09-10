

#ifndef main_h
#define main_h

#include <Arduino.h>


String devicename = "BABOI";
int maj_ver = 1;
int min_ver = 0;

typedef struct{
    float roll;
    float pitch;
    float yaw;
}t_sensorFusionData;


#endif //main_h