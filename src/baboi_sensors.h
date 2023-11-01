#ifndef baboi_sensors_h
#define baboi_sensors_h

#include <Wire.h>

float mpu_GetCurrentYaw(void);
float mpu_GetCurrentPitch(void);
float mpu_GetCurrentRoll(void);
float mpu_GetCurrentAX(void);
float mpu_GetCurrentAY(void); 
float mpu_GetCurrentAZ(void);
void mpu_cal_mag(void);
void mpu_cal_gyro_accel(void);
void mpu_init_settings(void);
void mpu_set_settings(void);
void mpu_store_data(void);
void i2c_scan(TwoWire* tw);
void init_sensors(void);
bool mpu_update(void);
void tension_update(void);
int16_t tension_get_ch1(void);
int16_t tension_get_ch2(void);
void calibrate_tension(void);

#endif