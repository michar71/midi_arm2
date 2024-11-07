#ifndef settings_h
#define settings_h

#include <stdint.h>


typedef struct{
    char magic0;
    char magic1;
    char magic2;
    char magic3;
    char magic4;
    float main_acc_bias_x;
    float main_acc_bias_y;
    float main_acc_bias_z;
    float main_gyro_bias_x;
    float main_gyro_bias_y;
    float main_gyro_bias_z;
    float main_mag_bias_x;
    float main_mag_bias_y;
    float main_mag_bias_z;
    float main_mag_scale_x;
    float main_mag_scale_y;
    float main_mag_scale_z;
    float second_acc_bias_x;
    float second_acc_bias_y;
    float second_acc_bias_z;
    float second_gyro_bias_x;
    float second_gyro_bias_y;
    float second_gyro_bias_z;      
    int16_t tension_max[2];
    int16_t tension_min[2];           
    float offset_roll;
    float offset_pitch;
    float offset_yaw;
    bool autoConnect;
    char ID[9];       //ID String, 8 Character + Zero
    uint16_t pos;      //0 = right, 1 = left
    uint8_t led_brightness;
}setup_t;



void settings_init();
void print_settings();
bool load_settings();
void save_settings();
void init_settings_mag();
void init_settings_acc_gyro();
void init_settings_tension();
void init_settings_other();


#endif //settings_h