#include "settings.h"
#include <Arduino.h>
#include <EEPROM.h>

setup_t settings;

//---------------------------------
//--  Settings Stuff  -------------
//---------------------------------
void print_settings()
{     
    Serial.println("Settings");
    Serial.println("--------");
    Serial.print(settings.magic0);
    Serial.print(settings.magic1);
    Serial.print(settings.magic2);
    Serial.print(settings.magic3);
    Serial.print(settings.magic4);  
    Serial.println("--------");              
    Serial.print("Acc Bias X: ");
    Serial.println(settings.acc_bias_x);
    Serial.print("Acc Bias Y: ");
    Serial.println(settings.acc_bias_y);
    Serial.print("Acc Bias Z: ");
    Serial.println(settings.acc_bias_z);    
    Serial.println("");
    Serial.print("Gyro Bias X: ");
    Serial.println(settings.gyro_bias_x);
    Serial.print("Gyro Bias Y: ");
    Serial.println(settings.gyro_bias_y);
    Serial.print("Gyro Bias Z: ");
    Serial.println(settings.gyro_bias_z);    
    Serial.println("");
    Serial.print("Mag Bias X: ");
    Serial.println(settings.mag_bias_x);
    Serial.print("Mag Bias Y: ");
    Serial.println(settings.mag_bias_y);
    Serial.print("Mag Bias Z: ");
    Serial.println(settings.mag_bias_z);    
    Serial.println("");
    Serial.print("Mag Scale X: ");
    Serial.println(settings.mag_scale_x);
    Serial.print("Mag Scale Y: ");
    Serial.println(settings.mag_scale_y);
    Serial.print("Mag Scale Z X: ");
    Serial.println(settings.mag_scale_z);    
    Serial.println("");    

}


void settings_init()
{
    EEPROM.begin(1024);
}


void init_settings_acc_gyro()
{
  settings.magic0 = 'M';
  settings.magic1 = 'A';
  settings.magic2 = 'G';
  settings.magic3 = 'I';
  settings.magic4 = 'C';
  settings.acc_bias_x = 0;
  settings.acc_bias_y = 0;
  settings.acc_bias_z = 0;
  settings.gyro_bias_x = 0;
  settings.gyro_bias_y = 0;
  settings.gyro_bias_z = 0;
}

void init_settings_mag()
{
  settings.mag_bias_x = 0;
  settings.mag_bias_y = 0;
  settings.mag_bias_z = 0;
  settings.mag_scale_x = 1;
  settings.mag_scale_y = 1;
  settings.mag_scale_z = 1;
}

void save_settings()
{
    uint16_t ii;
    uint8_t* pData;

    EEPROM.put(0,settings);
    EEPROM.commit();

}

void load_settings()
{
    uint16_t ii;

    EEPROM.get(0,settings);       
#ifdef DEBUG  
    print_settings();    
#endif
  if (isnan(settings.acc_bias_x))
  {
    settings.acc_bias_x = 0;
  }
  if (isnan(settings.acc_bias_y))
  {
    settings.acc_bias_y = 0;
  }
  if (isnan(settings.acc_bias_z))
  {
    settings.acc_bias_z = 0;
  }  
  if (isnan(settings.gyro_bias_x))
  {
    settings.gyro_bias_x = 0;
  }
  if (isnan(settings.gyro_bias_y))
  {
    settings.gyro_bias_y = 0;
  }
  if (isnan(settings.gyro_bias_z))
  {
    settings.gyro_bias_z = 0;
  }  
  if (isnan(settings.mag_bias_x))
  {
    settings.mag_bias_x = 0;
  }
  if (isnan(settings.mag_bias_y))
  {
    settings.mag_bias_y = 0;
  }
  if (isnan(settings.mag_bias_z))
  {
    settings.mag_bias_z = 0;
  }   
    if (isnan(settings.mag_scale_x))
  {
    settings.mag_scale_x = 0;
  }
  if (isnan(settings.mag_scale_y))
  {
    settings.mag_scale_y = 0;
  }
  if (isnan(settings.mag_scale_z))
  {
    settings.mag_scale_z = 0;
  }  
}
