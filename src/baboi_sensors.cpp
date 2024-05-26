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
bool hasGyro = false;


#define ADS1115_ADDR  0x48     //Assuming ADDR pin is tied to GND. Its cvurrenrtly floating, might need wire patch...)
#if BABOI_HW_VER == 2
#define ADC_CH_COUNT 4
#endif

#if BABOI_HW_VER == 3
#define ADC_CH_COUNT 2
#endif

uint8_t curr_ch = 0;
int16_t adc_data[ADC_CH_COUNT];

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

//ESP32-C3 ADC Continous read control
//------------------------------------
 #if BABOI_HW_VER == 3
#include <driver/adc.h>   // Analog to digital converter APIs


void ESP32_C3_ADC_SETUP(void)
{
    adc1_config_width((adc_bits_width_t)1); //10 bit.... Who knows why the define doesn't work....
    adc1_config_channel_atten(ADC1_CHANNEL_0, ADC_ATTEN_DB_12); //0..2500mV range
    adc1_config_channel_atten(ADC1_CHANNEL_1, ADC_ATTEN_DB_12); //0..2500mV range
}



 #endif 
//---------ADS1115 ADC ---------------


bool is_adc_data_ready(void)
{
 #if BABOI_HW_VER == 2
  return ads.conversionComplete();
 #endif

 #if BABOI_HW_VER == 3
   return true;
 #endif 
}

void startADCConversion(uint8_t ch)
{
 #if BABOI_HW_VER == 2
     ads.startADCReading(MUX_BY_CHANNEL[ch], false);
 #endif 

 #if BABOI_HW_VER == 3
 #endif
}

int32_t getADCValue(uint8_t ch)
{
 #if BABOI_HW_VER == 2
    return ads.getLastConversionResults();
 #endif  

 #if BABOI_HW_VER == 3
    if (ch == 1)
       return adc1_get_raw(ADC1_CHANNEL_1);
    else 
      return adc1_get_raw(ADC1_CHANNEL_0);
 #endif 
}


bool adc_update_manual()
{
    if (is_adc_data_ready())
    {
      //Read Data via I2C
      //#TODO Add running average here? or a 15%/%85 filter? Or full Kalman?
      adc_data[curr_ch] = getADCValue(curr_ch);
     
     /* 
      Serial.print("ADC ");
      Serial.print(curr_ch);
      Serial.print("\t:");
      Serial.print(adc_data[0]);
      Serial.print("\t:");
      Serial.println(adc_data[1]);
      */

      //Set Channel
      curr_ch++;
      if (curr_ch == ADC_CH_COUNT)
        curr_ch = 0;

      //Start Conversion
      startADCConversion(curr_ch);

      return true;
    }
    else
    {
      return false;
    }
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

bool checkForGyro(void)
{
  return hasGyro;
}


void calibrate_tension(void)
{
  uint8_t ccurr_ch = 0;

  if (checkForGlove())
  {
    init_settings_tension();

    //Read Raw Values and calulate min/max. Filter out outliers.
    for (int ii=0;ii< 800 ;ii++)
    {
      //Start ADC
      startADCConversion(ccurr_ch);
      //Wait for ADC
      while (is_adc_data_ready() == false)
      {
        delay(1);
      }

      //Read ADC
 #if BABOI_HW_VER == 2
      int16_t val = getADCValue(ccurr_ch);
#endif

 #if BABOI_HW_VER == 3
      int16_t val = getADCValue(ccurr_ch);
#endif


      //#TODO define reasonable limits here to remove outliers


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
      delay(4);
    }

    //We create some dead band around the middle for better range control.
    /*
    for(int ii=0;ii<ADC_CH_COUNT;ii++)
    {
      settings.tension_min[ii] = (int16_t)((float)settings.tension_min[ii] * 1.2);
      settings.tension_max[ii] = (int16_t)((float)settings.tension_max[ii] * 0.8);
    }
    */
  }

}

int16_t tension_get_ch(uint8_t ch)
{
  int16_t val = 0;
  
  float res = 0;
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
  Wire.begin(SDA, SCL,900000);
  //Strange but needed
  i2c_scan(&Wire,MPU_ADDR,127);
  if (!mpu.setup(MPU_ADDR)) 
  { 
      Serial.println("Failed to initialize MPU9250. No Gyro/Acc/Mag.");
      setLED(0,64,0,0);
      hasGyro = false;
  }    
  hasGyro = true;
  delay(200);


  #if BABOI_HW_VER == 2
  //Instantiate glove wire interface
  //--------------------------------
  Wire1.begin(GLOVE_SDA, GLOVE_SCL,4000000);
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
    ads.setGain(GAIN_FOUR);
    ads.setDataRate(RATE_ADS1115_128SPS);
    curr_ch = 0;   
    ads.startADCReading(MUX_BY_CHANNEL[0], false);
  }
#endif

#if BABOI_HW_VER == 3
  //Setup Glove ADC's
  //-----------------


  //Set pions to analog
  pinMode(ADC1_CH0,ANALOG);
  pinMode(ADC1_CH1,ANALOG);
  ESP32_C3_ADC_SETUP();
  //Set up ADC's?
  hasGlove =true;

  Serial.println("Glove ADC Setup done.");
#endif

  #ifdef DEBUG
    Serial.println("Sensor Setup Done...");
  #endif        
}
