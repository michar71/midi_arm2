#include "settings.h"
#include <Arduino.h>
#include <EEPROM.h>
#include "main.h"



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
    Serial.println(settings.main_acc_bias_x);
    Serial.print("Acc Bias Y: ");
    Serial.println(settings.main_acc_bias_y);
    Serial.print("Acc Bias Z: ");
    Serial.println(settings.main_acc_bias_z);    
    Serial.println("");
    Serial.print("Gyro Bias X: ");
    Serial.println(settings.main_gyro_bias_x);
    Serial.print("Gyro Bias Y: ");
    Serial.println(settings.main_gyro_bias_y);
    Serial.print("Gyro Bias Z: ");
    Serial.println(settings.main_gyro_bias_z);    
    Serial.println("");
    Serial.print("Mag Bias X: ");
    Serial.println(settings.main_mag_bias_x);
    Serial.print("Mag Bias Y: ");
    Serial.println(settings.main_mag_bias_y);
    Serial.print("Mag Bias Z: ");
    Serial.println(settings.main_mag_bias_z);    
    Serial.println("");
    Serial.print("Mag Scale X: ");
    Serial.println(settings.main_mag_scale_x);
    Serial.print("Mag Scale Y: ");
    Serial.println(settings.main_mag_scale_y);
    Serial.print("Mag Scale Z: ");
    Serial.println(settings.main_mag_scale_z);    
    Serial.println("");    

    Serial.print("Touch TH CTRL min: ");
    Serial.println(settings.th_but_ctrl_min);
    Serial.print("Touch TH A min: ");
    Serial.println(settings.th_but_a_min);
    Serial.print("Touch TH B min: ");
    Serial.println(settings.th_but_b_min);    
    Serial.print("Touch TH C min: ");
    Serial.println(settings.th_but_c_min);    
    Serial.print("Touch TH CTRL max: ");
    Serial.println(settings.th_but_ctrl_max);
    Serial.print("Touch TH A max: ");
    Serial.println(settings.th_but_a_max);
    Serial.print("Touch TH B max: ");
    Serial.println(settings.th_but_b_max);    
    Serial.print("Touch TH C max: ");
    Serial.println(settings.th_but_c_max);        

    Serial.println("");
    Serial.print("Tension Ch1 Min: ");
    Serial.println(settings.tension_min[0]);    
    Serial.print("Tension Ch1 Max: ");
    Serial.println(settings.tension_max[0]);   
    Serial.print("Tension Ch2 Min: ");
    Serial.println(settings.tension_min[1]);    
    Serial.print("Tension Ch2 Max: ");
    Serial.println(settings.tension_max[1]);   
    Serial.print("Tension Ch3 Min: ");
    Serial.println(settings.tension_min[2]);    
    Serial.print("Tension Ch3 Max: ");
    Serial.println(settings.tension_max[2]);   
    Serial.print("Tension Ch4 Min: ");
    Serial.println(settings.tension_min[3]);    
    Serial.print("Tension Ch4 Max: ");
    Serial.println(settings.tension_max[3]);           
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
  settings.main_acc_bias_x = 0;
  settings.main_acc_bias_y = 0;
  settings.main_acc_bias_z = 0;
  settings.main_gyro_bias_x = 0;
  settings.main_gyro_bias_y = 0;
  settings.main_gyro_bias_z = 0;
  settings.offset_roll = 0;
  settings.offset_pitch = 0;
  settings.offset_yaw = 0;
}

void init_settings_mag()
{
  settings.main_mag_bias_x = 0;
  settings.main_mag_bias_y = 0;
  settings.main_mag_bias_z = 0;
  settings.main_mag_scale_x = 1;
  settings.main_mag_scale_y = 1;
  settings.main_mag_scale_z = 1;
}

void init_settings_tension()
{
    for (int ii=0;ii<4;ii++)
    {
      settings.tension_max[ii] = -32768;
      settings.tension_min[ii] = 32767;
    }
}

void init_settings_but()
{
  settings.th_but_ctrl_max = 0;
  settings.th_but_a_max = 0;
  settings.th_but_b_max = 0;   
  settings.th_but_c_max = 0; 
  settings.th_but_ctrl_min = 0xFFFF;
  settings.th_but_a_min = 0xFFFF;
  settings.th_but_b_min = 0xFFFF;   
  settings.th_but_c_min = 0xFFFF;   
}

void init_settings_other()
{
  settings.autoConnect = false;  
  sprintf(settings.ID,"BABOI");  
  settings.pos = 0;
}

void save_settings()
{
    uint16_t ii;
    uint8_t* pData;

    EEPROM.put(0,settings);
    EEPROM.commit();

}

bool load_settings()
{
    uint16_t ii;

    EEPROM.get(0,settings);       
#ifdef DEBUG  
    print_settings();    
#endif
  if (isnan(settings.main_acc_bias_x))
  {
    settings.main_acc_bias_x = 0;
  }
  if (isnan(settings.main_acc_bias_y))
  {
    settings.main_acc_bias_y = 0;
  }
  if (isnan(settings.main_acc_bias_z))
  {
    settings.main_acc_bias_z = 0;
  }  
  if (isnan(settings.main_gyro_bias_x))
  {
    settings.main_gyro_bias_x = 0;
  }
  if (isnan(settings.main_gyro_bias_y))
  {
    settings.main_gyro_bias_y = 0;
  }
  if (isnan(settings.main_gyro_bias_z))
  {
    settings.main_gyro_bias_z = 0;
  }  
  if (isnan(settings.main_mag_bias_x))
  {
    settings.main_mag_bias_x = 0;
  }
  if (isnan(settings.main_mag_bias_y))
  {
    settings.main_mag_bias_y = 0;
  }
  if (isnan(settings.main_mag_bias_z))
  {
    settings.main_mag_bias_z = 0;
  }   
    if (isnan(settings.main_mag_scale_x))
  {
    settings.main_mag_scale_x = 0;
  }
  if (isnan(settings.main_mag_scale_y))
  {
    settings.main_mag_scale_y = 0;
  }
  if (isnan(settings.main_mag_scale_z))
  {
    settings.main_mag_scale_z = 0;
  }  

  if ((settings.magic0 == 'M') && (settings.magic1 == 'A') && (settings.magic2 == 'G') && (settings.magic3 == 'I') && (settings.magic4 == 'C'))
  {
    return true;
  }
  else
  {
    return false;
  }
}
