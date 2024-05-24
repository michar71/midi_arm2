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

bool adc_update_manual();
int16_t adc_get_data(uint8_t ch);
void glove_update(void);
bool checkForGlove(void);
bool checkForGyro(void);

void i2c_scan(TwoWire* tw,uint8_t startaddr, uint8_t endaddr);
void init_sensors(void);
bool mpu_update(void);
int16_t tension_get_ch(uint8_t ch);
void calibrate_tension(void);

#endif