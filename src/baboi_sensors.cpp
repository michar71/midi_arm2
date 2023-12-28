#include <arduino.h>
#include "baboi_sensors.h"
#include "baboi_led.h"
#include "MPU9250.h"
#include <Adafruit_ADS1X15.h>
#include "settings.h"
#include "main.h"


MPU9250 mpu;
Adafruit_ADS1015 ads;     /* Use this for the 12-bit version */

extern setup_t settings;

#define ADC_COUNT 4
uint16_t adc_val[2][ADC_COUNT];
uint8_t adc_index1 = 0;
uint8_t adc_index2 = 0;

uint16_t adc_ch1_avg = 0;
uint16_t adc_ch2_avg = 0;

bool hasGlove = false;


#define ADS1115_ADDR  0x48     //Assuming ADDR pin is tied to GND. Its cvurrenrtly floating, might need wire patch...)
#define ADS1115_CH_COUNT 3
bool ADS1115_data_ready = false;
uint8_t curr_ch = 0;
int16_t ads1115_data[ADS1115_CH_COUNT] = {0,0,0};

#define MPU_ADDR 0x68


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
    float roll;
    float pitch;
    float yaw;

    init_settings_acc_gyro();
    mpu.calibrateAccelGyro();
    //run the MPU for a 3 Sec to get stable readings
  for (int ii=0;ii<100;ii++)
  {
    mpu_update();
    delay(10);
  }

    //Calculate offsets

    settings.offset_roll = -mpu_GetCurrentRoll();
    settings.offset_pitch = -mpu_GetCurrentPitch();
    settings.offset_yaw = -mpu_GetCurrentYaw();

    mpu_store_data();
}


void mpu_cal_mag(void)
{
    init_settings_mag();
    mpu.calibrateMag();
    mpu_store_data();
}



//Need to fit the infite roational code here from the Kaslman filter processing
//in the Processing sketch. Still doesn't solve windup from multiple roatations.
//Maybe do that calculation after the offset...

float mpu_GetCurrentYaw(void)
{
  //return (mpu.getYaw() * DEG_TO_RAD) + settings.offset_yaw;
  return (mpu.getYaw() * DEG_TO_RAD) ;
}

float mpu_GetCurrentPitch(void)
{
  //return (mpu.getPitch() * DEG_TO_RAD) + settings.offset_pitch;
  return (mpu.getPitch() * DEG_TO_RAD);
}

float mpu_GetCurrentRoll(void)
{
  //return (mpu.getRoll() * DEG_TO_RAD) + settings.offset_roll;
  return (mpu.getRoll() * DEG_TO_RAD);  
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


//---------ADS1115 ADC ---------------

IRAM_ATTR void ads1115_irq()
{
  ADS1115_data_ready = true;
}



//Need to add downsampling to correct range here....
bool ads1115_update()
{
  if (hasGlove)
  {
    if (ADS1115_data_ready)
    {
      //Read Data via I2C
      //#TODO Add running average here? or a 15%/%85 filter? Or full Kalman?
      ads1115_data[curr_ch] = ads.getLastConversionResults();

      //Set Channel
      curr_ch++;
      if (curr_ch == ADS1115_CH_COUNT)
        curr_ch = 0;

      //Start Conversion
      ADS1115_data_ready = false;
      ads.startADCReading(MUX_BY_CHANNEL[curr_ch], false);
      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}


bool ads1115_update_manual()
{
    if (hasGlove)
  {
    if (ads.conversionComplete())
    {
      //Read Data via I2C
      //#TODO Add running average here? or a 15%/%85 filter? Or full Kalman?
      ads1115_data[curr_ch] = ads.getLastConversionResults();
     
      /*
      Serial.print("ADV:");
      Serial.print(ads1115_data[0]);
      Serial.print("/");
      Serial.print(ads1115_data[1]);
      Serial.print("/");
      Serial.print(ads1115_data[2]);
      Serial.print("/");
      Serial.print(ads1115_data[3]);              
      Serial.println("");
      */

      //Set Channel
      curr_ch++;
      if (curr_ch == ADS1115_CH_COUNT)
        curr_ch = 0;

      //Start Conversion
      ADS1115_data_ready = false;
      ads.startADCReading(MUX_BY_CHANNEL[curr_ch], false);

      return true;
    }
    else
    {
      return false;
    }
  }
  else
  {
    return false;
  }
}

//Need to add calibration range adjustment here...
int16_t ads1115_get_data(uint8_t ch)
{
  if (ch < ADS1115_CH_COUNT)
    return ads1115_data[ch];
  else
    return 0;  
}
//------ END ADS1115 -----------------


void glove_update(void)
{
    //ads1115_update();
    ads1115_update_manual();
}

bool checkForGlove(void)
{
  return hasGlove;
}

void calibrate_tension(void)
{
  uint8_t ccurr_ch = 0;

  if (hasGlove)
  {
    init_settings_tension();

    //Read Raw Values and calulate min/max. Filter out outliers.
    for (int ii=0;ii< 500 ;ii++)
    {
      //Start ADC
      ads.startADCReading(MUX_BY_CHANNEL[ccurr_ch], false);
      //Wait for ADC
      while (ads.conversionComplete() == false)
      {
        delay(1);
      }

      //Read ADC
      int16_t val = ads.getLastConversionResults();

      //#TODO define reasonable limits here to remove outliers

      //Record new min/max
      if (val > settings.tension_max[ccurr_ch])
        settings.tension_max[ccurr_ch] = val;

      if (val < settings.tension_min[ccurr_ch])
        settings.tension_min[ccurr_ch] = val;

      
      //Move on to Next ADC
      ccurr_ch++;
      if (ccurr_ch == ADS1115_CH_COUNT)
        ccurr_ch = 0;

      //Wait a little bit
      delay(2);
    }

    //We create some dead band around the middle for bertter range control.
    for(int ii=0;ii<ADS1115_CH_COUNT;ii++)
    {
      //settings.tension_min[ii] = (int16_t)((float)settings.tension_min[ii] * 1.2);
      //settings.tension_max[ii] = (int16_t)((float)settings.tension_max[ii] * 0.8);
    }
  }
}

int16_t tension_get_ch(uint8_t ch)
{
  int16_t val = 0;
  
  float res = 0;
  res = map(ads1115_data[ch],settings.tension_min[ch],settings.tension_max[ch],0,255);
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
    Wire.begin(SDA, SCL,4000000);

    //Strange but needed
    i2c_scan(&Wire,MPU_ADDR,127);

    if (!mpu.setup(MPU_ADDR)) 
    { 
        Serial.println("ERROR initalizing MPU!");
        setLED(0,64,0,0);
    }    

    //Just toggle I2C lines to test continuity
    /*
    pinMode(GLOVE_SDA,OUTPUT);
    pinMode(GLOVE_SCL,OUTPUT);
    while(1)
    {
      digitalWrite(GLOVE_SDA,HIGH);
      digitalWrite(GLOVE_SCL,HIGH);
      delay(50);
  
      digitalWrite(GLOVE_SDA,LOW);
      digitalWrite(GLOVE_SCL,LOW);
      delay(50);
      toggle_status_led();
    }
    */

    //Instantiate glove wire interface

    //HACKALERT!!!! Pin21 seems to have issues for I2C?
    //Moving it over to 34 and set 21 has input to not interfere...
    pinMode(GLOVE_SCL_OLD,INPUT);
    Wire1.begin(GLOVE_SDA, GLOVE_SCL,1000000);


    //TODO Check if needed here too...
    i2c_scan(&Wire1,ADS1115_ADDR,127);

    if (!ads.begin(ADS1115_ADDR,&Wire1)) 
    {
      Serial.println("Failed to initialize ADS1115. No Glove.");
      hasGlove = false;
    }
    else
    {
      Serial.println("ADS1115 Ready. Found Glove.");
      hasGlove =true;

      //Setup IRQ
      //pinMode(ADS1115_ALERT_PIN, INPUT);
      // We get a falling edge every time a new sample is ready.
      //attachInterrupt(ADS1115_ALERT_PIN, ads1115_irq, FALLING);

      //Start ADC
      ads.setGain(GAIN_TWO);
      ads.setDataRate(RATE_ADS1115_475SPS);
      curr_ch = 0;   
      ads.startADCReading(MUX_BY_CHANNEL[0], false);
    }

  #ifdef DEBUG
    Serial.println("Sensor Setup Done...");
  #endif        
}
