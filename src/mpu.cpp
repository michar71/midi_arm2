/*

#include "MPU9250.h"
#include <Wire.h>
#include "settings.h"

MPU9250 mpu;

extern setup_t settings;

void store_mpu_data()
{
    settings.main_acc_bias_x = mpu.getAccBiasX();
    settings.main_acc_bias_y = mpu.getAccBiasY();
    settings.main_acc_bias_z = mpu.getAccBiasZ();
    settings.main_gyro_bias_x = mpu.getGyroBiasX();
    settings.main_gyro_bias_y = mpu.getGyroBiasY();
    settings.main_gyro_bias_z = mpu.getGyroBiasZ();
    settings.main_mag_bias_x = mpu.getMagBiasX();
    settings.main_mag_bias_y = mpu.getMagBiasY();
    settings.main_mag_bias_z = mpu.getMagBiasZ();
    settings.main_mag_scale_x = mpu.getMagScaleX();
    settings.main_mag_scale_y = mpu.getMagScaleY();
    settings.main_mag_scale_z = mpu.getMagScaleZ();
}

void scan_mpu()
{

}

void init_mpu()
{

}

void calib_gyro_acc()
{

}

void calib_mag()
{

}

void update_mpu()
{

}

*/