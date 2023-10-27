#include <arduino.h>
#include "baboi_sensors.h"
#include "MPU9250.h"
#include "settings.h"
#include "main.h"


MPU9250 mpu;

extern setup_t settings;

void mpu_store_data(void)
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

void i2c_scan(TwoWire* tw)
{
  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
  nDevices = 0;
  for(address = 1; address < 127; address++ )
  {
    // The i2c_scanner uses the return value of
    // the Write.endTransmisstion to see if
    // a device did acknowledge to the address.
    tw->beginTransmission(address);
    error = tw->endTransmission();
 
    if (error == 0)
    {
      Serial.print("I2C device found at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.print(address,HEX);
      Serial.println("  !");
 
      nDevices++;
    }
    else if (error==4)
    {
      Serial.print("Unknown error at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
    }    
  }
  if (nDevices == 0)
    Serial.println("No I2C devices found\n");
  else
    Serial.println("done\n");
}

void mpu_set_settings(void)
{
    mpu.setAccBias(settings.main_acc_bias_x ,settings.main_acc_bias_y ,settings.main_acc_bias_z);
    mpu.setGyroBias(settings.main_gyro_bias_x,settings.main_gyro_bias_y,settings.main_gyro_bias_z);
    mpu.setMagBias(settings.main_mag_bias_x,settings.main_mag_bias_y,settings.main_mag_bias_z);
    mpu.setMagScale(settings.main_mag_scale_x,settings.main_mag_scale_y,settings.main_mag_scale_z);
}


void mpu_init_settings(void)
{
    init_settings_acc_gyro();
    init_settings_mag();
    mpu_store_data();
}

void mpu_cal_gyro_accel(void)
{
    init_settings_acc_gyro();
    mpu.calibrateAccelGyro();
    mpu_store_data();
}


void mpu_cal_mag(void)
{
    init_settings_mag();
    mpu.calibrateMag();
    mpu_store_data();
}


  float mpu_GetCurrentYaw(void)
  {
    return mpu.getYaw()*DEG_TO_RAD;
  }

  float mpu_GetCurrentPitch(void)
  {
    return mpu.getPitch()*DEG_TO_RAD;
  }

  float mpu_GetCurrentRoll(void)
  {
    return mpu.getRoll()*DEG_TO_RAD;
  }

  float mpu_GetCurrentAX(void)
  {
    return mpu.getLinearAccX();
  }

  float mpu_GetCurrentAY(void)  
  {
    return mpu.getLinearAccY();
  }

  float mpu_GetCurrentAZ(void)
  {
    return mpu.getLinearAccZ();
  }


bool mpu_update(void)
{
    return mpu.update();
}

void init_sensors(void)
{
    Wire.begin(SDA, SCL,1000000);
    i2c_scan(&Wire);
    
    if (!mpu.setup(0x68)) 
    { 
        Serial.println("ERROR");
        setLED(0,64,0,0);
        delay(5000);
        while(1)
        {}
    }    

    delay(10);

  #ifdef DEBUG
    Serial.println("Sensor Setup Done...");
  #endif        
}
