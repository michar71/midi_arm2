#include "Arduino.h"
#include <Wire.h>
#include "MPU9250/QuaternionFilter.h"

#ifndef GY_85_h
#define GY_85_h

//----------addresses----------//
#define ADXL345 (0x53)         // Device address as specified in data sheet //ADXL345 accelerometer
#define DATAX0  (0x32)         //X-Axis Data 0
//#define DATAX1 0x33          //X-Axis Data 1
//#define DATAY0 0x34          //Y-Axis Data 0
//#define DATAY1 0x35          //Y-Axis Data 1
//#define DATAZ0 0x36          //Z-Axis Data 0
//#define DATAZ1 0x37          //Z-Axis Data 1
#define HMC5883 (0x1E)         //Magnetometer
#define QMC5883 (0x0D)         //Magnetometer
#define ITG3200 (0x68)         //Gyroscope




class GY_85
{
    
typedef struct{
    float gyroX;
    float gyroY;
    float gyroZ;
    float accX;
    float accY;
    float accZ; 
    float magX;
    float magY;
    float magZ;
    float temp;
}t_GY85RawSensorData;

t_GY85RawSensorData sensorData;

bool isAvailable = false;
byte mag_addr = HMC5883;

private:

    void SetGyro();
    void SetCompass();
    void SetAccelerometer();
    bool idCompass(void);
    bool idAcc(void);
    bool idGyro(void);    
    bool available(void); 
    void update_rpy(float qw, float qx, float qy, float qz);
    void selectFilter(QuatFilterSel sel);
    bool compassReady(void);
    TwoWire* intWire;
    
public:
    bool   init(TwoWire* tw);
    void readFromAccelerometer();  //Read data and convert to G-Force
    void readFromCompass();
    void readGyro();
    void calibrate();
    void compassCalibrate(void);
    void GyroCalibrate();
    void AccCalibrate();    
    void readRawFromCompass();
    bool update();
    float getPitch();
    float getRoll();
    float getYaw();
    
    //callback functions
    inline float accelerometer_x(){ return sensorData.accX; }
    inline float accelerometer_y(){ return sensorData.accY; }
    inline float accelerometer_z(){ return sensorData.accZ; }
    
    //-----------------------------------
    
    inline float compass_x(){ return sensorData.magX; }
    inline float compass_y(){ return sensorData.magY; }
    inline float compass_z(){ return sensorData.magZ; }
    
    //-----------------------------------
    
    inline float gyro_x(){ return sensorData.gyroX; }
    inline float gyro_y(){ return sensorData.gyroY; }
    inline float gyro_z(){ return sensorData.gyroZ;}
    inline float temp  (){ return sensorData.temp; }

    float getGyroXoffset(void);
    float getGyroYoffset(void);
    float getGyroZoffset(void);
    void setGyroOffset(float offsX,float offsY,float offsZ);


    float getAccXoffset(void);
    float getAccYoffset(void);
    float getAccZoffset(void);
    void setAccOffset(float offsX,float offsY,float offsZ);

    float getMagXoffset(void);
    float getMagYoffset(void);
    float getMagZoffset(void);
    float getMagXscale(void);
    float getMagYscale(void);
    float getMagZscale(void);
    void setMagOffset(float offsX,float offsY,float offsZ);
    void setMagScale(float scaleX,float scaleY,float scaleZ);

    float getLinearAccX(void);
    float getLinearAccY(void);
    float getLinearAccZ(void);
};

#endif

