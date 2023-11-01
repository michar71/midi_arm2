#include <arduino.h>
#include "baboi_sensors.h"
#include "MPU9250.h"
#include "settings.h"
#include "main.h"


MPU9250 mpu;

extern setup_t settings;

#define ADC_COUNT 8
uint16_t adc_val[2][ADC_COUNT] = {{0,0,0,0,0,0,0,0},{0,0,0,0,0,0,0,0}};
uint8_t adc_index = 0;

uint16_t adc_ch1_avg = 0;
uint16_t adc_ch2_avg = 0;



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

void tension_update(void)
{
  uint32_t s1;
  uint32_t s2;

  adc_val[0][adc_index]= analogRead(ANALOG_CH1);
  adc_val[1][adc_index]= analogRead(ANALOG_CH2);
  adc_index++;
  if (adc_index == ADC_COUNT)
    adc_index = 0;

  s1 = 0;
  s2 = 0;
  for (uint8_t ii=0;ii<ADC_COUNT;ii++)
  {
    s1 = s1 + adc_val[0][ii];
    s2 = s2 + adc_val[01][ii];
  }
  adc_ch1_avg = s1/ADC_COUNT;
  adc_ch2_avg = s2/ADC_COUNT;
 
  //Add Kalman Filter here...
}


void calibrate_tension(void)
{
  int ii=0;
  //Check if we have a glove
  for (ii=0;ii<50;ii++)
  {
    tension_update();
    yield();
  }
  //No glove found
  if ((tension_get_ch1() < 0) || (tension_get_ch2() < 0))
    return;

  init_settings_tension();

  for (ii=0;ii<1000;ii++)
  {
    int16_t val;
    
    val = analogRead(ANALOG_CH1);
    if (val>settings.tension_ch1_max)
      settings.tension_ch1_max = val;
    else if (val<settings.tension_ch1_min)
      settings.tension_ch1_min = val;

    val = analogRead(ANALOG_CH2);
    if (val>settings.tension_ch2_max)
      settings.tension_ch2_max = val;
    else if (val<settings.tension_ch2_min)
      settings.tension_ch2_min = val;

    delay(5);
  }
}

int16_t tension_get_ch1(void)
{
  int16_t val = 0;
  
  if (adc_ch1_avg < 2000)
    val = -1;
  else
  {
    float res = 0;
    res = map(adc_ch1_avg,settings.tension_ch1_min,settings.tension_ch1_max,0,255);
    if (res<0)
      val = 0;
    else if (res>255)
      val = 255;
    else
      val = round(res);  
  }  
  return val;
}

int16_t tension_get_ch2(void)
{
  int16_t val = 0;
  
  if (adc_ch2_avg < 2000)
    val = -1;
  else
  {
    float res = 0;
    res = map(adc_ch2_avg,settings.tension_ch2_min,settings.tension_ch2_max,0,255);
    if (res<0)
      val = 0;
    else if (res>255)
      val = 255;
    else
      val = round(res);  
  }  
  return val;
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

    //Setup analog pins
    pinMode(ANALOG_CH1,ANALOG);
    pinMode(ANALOG_CH2,ANALOG);
    analogSetPinAttenuation(ANALOG_CH1,ADC_0db);
    analogSetPinAttenuation(ANALOG_CH2,ADC_0db);    


  #ifdef DEBUG
    Serial.println("Sensor Setup Done...");
  #endif        
}
