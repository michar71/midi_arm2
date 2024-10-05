#include <arduino.h>
#include "baboi_sensors.h"
#include "baboi_led.h"
#include "MPU9250.h"
#include "settings.h"
#include "main.h"
#include <driver/adc.h>   // Analog to digital converter APIs
#include "GY_85.h"

MPU9250 mpu1;
GY_85 mpu2;


extern setup_t settings;

bool hasGlove = false;
bool hasMCU = false;

#define ADC_CH_COUNT 2

uint8_t curr_ch = 0;
int16_t adc_data[ADC_CH_COUNT];

#define MPU_ADDR 0x68

typedef enum{
    DOF_MPU9250,
    DOF_GY_85,
    DOF_NONE
}e_dof;


e_dof dof = DOF_NONE;




void mpu_store_data(void)
{
  switch(dof)
  {
    case DOF_MPU9250:
      settings.main_acc_bias_x = mpu1.getAccBiasX();
      settings.main_acc_bias_y = mpu1.getAccBiasY();
      settings.main_acc_bias_z = mpu1.getAccBiasZ();
      settings.main_gyro_bias_x = mpu1.getGyroBiasX();
      settings.main_gyro_bias_y = mpu1.getGyroBiasY();
      settings.main_gyro_bias_z = mpu1.getGyroBiasZ();
      settings.main_mag_bias_x = mpu1.getMagBiasX();
      settings.main_mag_bias_y = mpu1.getMagBiasY();
      settings.main_mag_bias_z = mpu1.getMagBiasZ();
      settings.main_mag_scale_x = mpu1.getMagScaleX();
      settings.main_mag_scale_y = mpu1.getMagScaleY();
      settings.main_mag_scale_z = mpu1.getMagScaleZ();
      break;
    case DOF_GY_85:
      settings.main_acc_bias_x = mpu2.getAccXoffset();
      settings.main_acc_bias_y = mpu2.getAccYoffset();
      settings.main_acc_bias_z = mpu2.getAccZoffset();
      settings.main_gyro_bias_x = mpu2.getGyroXoffset();
      settings.main_gyro_bias_y = mpu2.getGyroYoffset();
      settings.main_gyro_bias_z = mpu2.getGyroZoffset();
      settings.main_mag_bias_x = mpu2.getMagXoffset();
      settings.main_mag_bias_y = mpu2.getMagYoffset();
      settings.main_mag_bias_z = mpu2.getMagZoffset();
      settings.main_mag_scale_x = 0;
      settings.main_mag_scale_y = 0;
      settings.main_mag_scale_z = 0;
      break;  
  }
}

void i2c_scan(TwoWire* tw,uint8_t startaddr, uint8_t endaddr)
{
  byte error, address;
  int nDevices;
 
  Serial.println("Scanning...");
 
  nDevices = 0;
  if (startaddr<1)
    startaddr = 1;
  if (endaddr > 127)
    endaddr = 127;  
  for(address = startaddr; address < endaddr; address++ )
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

    else if (error == 5)
    {
      Serial.print("Timeout Error on Bus");
      Serial.print(error);
      Serial.print(" at address 0x");
      if (address<16)
        Serial.print("0");
      Serial.println(address,HEX);
      Serial.print("Aborting Scan");
      return;
    }
    else
    {
      Serial.print("Error ");
      Serial.print(error);
      Serial.print(" at address 0x");
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
  switch(dof)
  {
    case DOF_MPU9250:
      mpu1.setAccBias(settings.main_acc_bias_x ,settings.main_acc_bias_y ,settings.main_acc_bias_z);
      mpu1.setGyroBias(settings.main_gyro_bias_x,settings.main_gyro_bias_y,settings.main_gyro_bias_z);
      mpu1.setMagBias(settings.main_mag_bias_x,settings.main_mag_bias_y,settings.main_mag_bias_z);
      mpu1.setMagScale(settings.main_mag_scale_x,settings.main_mag_scale_y,settings.main_mag_scale_z);
      break;
    case DOF_GY_85:
      mpu2.setAccOffset(settings.main_acc_bias_x ,settings.main_acc_bias_y ,settings.main_acc_bias_z);
      mpu2.setGyroOffset(settings.main_gyro_bias_x,settings.main_gyro_bias_y,settings.main_gyro_bias_z);
      mpu2.setMagOffset(settings.main_mag_bias_x,settings.main_mag_bias_y,settings.main_mag_bias_z);    
      break;
  }
}


void mpu_init_settings(void)
{
    init_settings_acc_gyro();
    init_settings_mag();
    mpu_store_data();
}

void mpu_cal_gyro_accel(void)
{
  switch(dof)
  {
    case DOF_MPU9250:
    {
      float roll;
      float pitch;
      float yaw;

      init_settings_acc_gyro();
      mpu1.calibrateAccelGyro();
      //run the MPU for a 3 Sec to get stable readings
    for (int ii=0;ii<100;ii++)
    {
      mpu_update();
      delay(10);
    }

      //Calculate offsets

      settings.offset_roll = -(mpu1.getRoll() * DEG_TO_RAD);  
      settings.offset_pitch = -(mpu1.getPitch() * DEG_TO_RAD);
      settings.offset_yaw = -(mpu1.getYaw() * DEG_TO_RAD);
      break;
    }
    case DOF_GY_85:
    {
      init_settings_acc_gyro();
      mpu2.calibrate();
      for (int ii=0;ii<100;ii++)
      {
        mpu_update();
        delay(10);
      }     
      settings.offset_roll = -(mpu2.getRoll() * DEG_TO_RAD);  
      settings.offset_pitch = -(mpu2.getPitch() * DEG_TO_RAD);
      settings.offset_yaw = -(mpu2.getYaw() * DEG_TO_RAD);     
      break;
    }
  }
  mpu_store_data();
}


void mpu_cal_mag(void)
{
  init_settings_mag();
  switch(dof)
  {
    case DOF_MPU9250:    
      mpu1.calibrateMag();
      break;
    case DOF_GY_85:
      mpu2.compassCalibrate();
      break;
  }
  mpu_store_data();
}


float mpu_GetCurrentYaw(void)
{
  float angle;
    switch(dof)
  {
    case DOF_MPU9250:    
      angle = (mpu1.getYaw() * DEG_TO_RAD);
      break;
    case DOF_GY_85:
      angle = (mpu2.getYaw() * DEG_TO_RAD);
      break;  
  }
  float new_angle = angle + settings.offset_yaw;
  if (new_angle < -PI)
     new_angle = new_angle + (2*PI);
  else if (new_angle > PI)
     new_angle = new_angle - (2*PI);
 

  return new_angle;

  //return (mpu.getYaw() * DEG_TO_RAD) + settings.offset_yaw;
  //return (mpu.getYaw() * DEG_TO_RAD) ;
}


//Ptich and roll swapped here for V3.5 HW!!!

float mpu_GetCurrentPitch(void)
{
  //return (mpu.getPitch() * DEG_TO_RAD) + settings.offset_pitch;

  switch(dof)
  {
    case DOF_MPU9250:      
        return (mpu1.getPitch() * DEG_TO_RAD);
      case DOF_GY_85:
        return (mpu2.getPitch() * DEG_TO_RAD);
  }
}

float mpu_GetCurrentRoll(void)
{
  //return (mpu.getRoll() * DEG_TO_RAD) + settings.offset_roll;

  switch(dof)
  {
    case DOF_MPU9250:      
        return (mpu1.getRoll() * DEG_TO_RAD);  
      case DOF_GY_85:
        return (mpu2.getRoll() * DEG_TO_RAD);  
  }  
}

float mpu_GetCurrentAX(void)
{
  
  //return mpu.getAccX();

  switch(dof)
  {
    case DOF_MPU9250:      
        return mpu1.getLinearAccX();
      case DOF_GY_85:
        return mpu2.getLinearAccX();
  }    
}

float mpu_GetCurrentAY(void)  
{
  
  //return mpu.getAccY();  

  switch(dof)
  {
    case DOF_MPU9250:      
        return mpu1.getLinearAccY();
      case DOF_GY_85:
        return mpu2.getLinearAccY();
  }    
}

float mpu_GetCurrentAZ(void)
{
  
  //return mpu.getAccZ();

  switch(dof)
  {
    case DOF_MPU9250:      
        return mpu1.getLinearAccZ();
      case DOF_GY_85:
        return mpu2.getLinearAccZ();
  }  
}


bool mpu_update(void)
{
  switch(dof)
  {
    case DOF_MPU9250:      
        return mpu1.update();
      case DOF_GY_85:
        return mpu2.update();
  }      
}

//ESP32-C3 ADC Continous read control
//------------------------------------
void ESP32_C3_ADC_SETUP(void)
{
    adc1_config_width((adc_bits_width_t)1); //10 bit.... Who knows why the define doesn't work....
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12); //0..2500mV range
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_12); //0..2500mV range
}


int32_t getADCValue(uint8_t ch)
{
    if (ch == 1)
       return adc1_get_raw(ADC1_CHANNEL_1);
    else 
      return adc1_get_raw(ADC1_CHANNEL_0);
}


bool adc_update_manual()
{
    //#TODO Add running average here? or a 15%/%85 filter? Or full Kalman?
    adc_data[curr_ch] = getADCValue(curr_ch);     

    //Serial.print("ADC ");
    //Serial.print(curr_ch);
    //Serial.print(" VAL ");
    //Serial.println(adc_data[curr_ch]);

    //Set Channel
    curr_ch++;
    if (curr_ch == ADC_CH_COUNT)
      curr_ch = 0;

    return true;
}

//Need to add calibration range adjustment here...
int16_t adc_get_data(uint8_t ch)
{
  if (ch < ADC_CH_COUNT)
    return adc_data[ch];
  else
    return -1;  
}
//------ END ADS1115 -----------------


void glove_update(void)
{
    adc_update_manual();
}

bool checkForGlove(void)
{
  return hasGlove;
}

bool checkForMCU(void)
{
  return hasMCU;
}


void calibrate_tension(void)
{
  uint8_t ccurr_ch = 0;

  if (checkForGlove())
  {
    init_settings_tension();

    //Read Raw Values and calulate min/max. Filter out outliers.
    for (int ii=0;ii< 1000 ;ii++)
    {
      //Read ADC
      int16_t val = getADCValue(ccurr_ch);


      //Record new min/max
      if (val > settings.tension_max[ccurr_ch])
        settings.tension_max[ccurr_ch] = val;

      if (val < settings.tension_min[ccurr_ch])
        settings.tension_min[ccurr_ch] = val;

      
      //Move on to Next ADC
      ccurr_ch++;
      if (ccurr_ch == ADC_CH_COUNT)
        ccurr_ch = 0;

      //Wait a little bit
      delay(5);
    }

    //We'll reduce the range of thre strips by 20% at the lower end and 10% at the upper end to create some sort of deadband.
    settings.tension_max[0] = (int16_t)((float)settings.tension_max[0] * 0.9);
    settings.tension_max[1] = (int16_t)((float)settings.tension_max[1] * 0.9);
    settings.tension_min[0] = (int16_t)((float)settings.tension_min[0] * 1.2);
    settings.tension_min[1] = (int16_t)((float)settings.tension_min[1] * 1.2);
  }
}

int16_t tension_get_ch(uint8_t ch)
{
  int16_t val = 0;
  
  float res = 0;

  if (settings.tension_min[ch] == settings.tension_max[ch])
    return 0;

  res = map(adc_data[ch],settings.tension_min[ch],settings.tension_max[ch],0,255);
  if (res<0)
    val = 0;
  else if (res>255)
    val = 255;
  else
    val = round(res);  
  return val;
}


void init_sensors(void)
{
  //Instantiate Gyro interface
  //------------------------
  Wire.begin(SDA, SCL,400000);
  //Strange but needed
  //i2c_scan(&Wire,MPU_ADDR,127);
  i2c_scan(&Wire,0,127);

  if (!mpu1.setup(MPU_ADDR)) 
  { 
      Serial.println("Failed to initialize MPU9250. No Gyro/Acc/Mag.");

      if (!mpu2.init(&Wire))
      {
        Serial.println("Failed to initialize GY-85. No Gyro/Acc/Mag.");
        setLED(0,64,0,0);
        hasMCU = false;
        dof = DOF_NONE;
      }
      else
      {
        Serial.println("Found GY-85");
        hasMCU = true;
        dof = DOF_GY_85;
      }
  }    
  else
  {
    Serial.println("Found MPU9250");
    hasMCU = true;
    dof = DOF_MPU9250;
  }
  
  delay(200);

  //Setup Glove ADC's
  //-----------------


  //Set pions to analog
  pinMode(ADC1_CH0,ANALOG);
  pinMode(ADC1_CH1,ANALOG);
  ESP32_C3_ADC_SETUP();


  //mayube we use one of the unused pins and tie it to ground in the glove connector to check if its plugged in...
  hasGlove =true;

  Serial.println("Glove ADC Setup done.");


  #ifdef DEBUG
    Serial.println("Sensor Setup Done...");
  #endif        
}
