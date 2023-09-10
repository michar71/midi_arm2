#include "GY_85.h"


//TODO
//Need to add mag Calibration
//Need to add Acc Calibration

//Need to add Load/Store offset from calibration

float a_offx = 0;
float a_offy = 0;
float a_offz = 0;

void GY_85::SetAccelerometer()
{
    //Put the ADXL345 into +/- 4G range by writing the value 0x01 to the DATA_FORMAT register.
    intWire->beginTransmission( ADXL345 );      // start transmission to device
    intWire->write( 0x31 );                     // send register address
    intWire->write( 0x01 );                     // send value to write
    intWire->endTransmission();                 // end transmission

    //Put the ADXL345 DIDO into stream mode
    intWire->beginTransmission( ADXL345 );      // start transmission to device
    intWire->write( 0x38 );                     // send register address
    intWire->write( 0x40 );                     // send value to write
    intWire->endTransmission();                 // end transmission  
    
    //Put the ADXL345 into Measurement Mode by writing 0x08 to the POWER_CTL register.
    intWire->beginTransmission( ADXL345 );      // start transmission to device
    intWire->write( 0x2D );                     // send register address  //Power Control Register
    intWire->write( 0x08 );                     // send value to write
    intWire->endTransmission();                 // end transmission
    
}

void GY_85::AccCalibrate()
{
    float tmpx = 0;
    float tmpy = 0;
    float tmpz = 0;
    
    a_offx = 0;
    a_offy = 0;
    a_offz = 0;
    
    for( uint8_t i = 0; i < 10; i ++ ) //take the mean from 10 gyro probes and divide it from the current probe
    {
        delay(10);
        readFromAccelerometer();
        tmpx += sensorData.accX;
        tmpy += sensorData.accY;
        tmpz += sensorData.accZ;
    }
    a_offx = tmpx/10.0;
    a_offy = tmpy/10.0;
    a_offz = tmpz/10.0;
    a_offz = a_offz - 1.0;  //(Add Gravity back...)

    Serial.print("AOX:");
    Serial.print(a_offx);
    Serial.print("AOY:");
    Serial.print(a_offy);
    Serial.print("AOZ:");
    Serial.println(a_offz);
}

#define ACC_RANGE 8
#define ACC_RES 10
void GY_85::readFromAccelerometer()
{
    byte buff[6];
    
    intWire->beginTransmission( ADXL345 );      // start transmission to device
    intWire->write( DATAX0 );                   // sends address to read from
    intWire->endTransmission();                 // end transmission
 
    intWire->requestFrom( ADXL345, 6 );         // request 6 bytes from device


    uint8_t i = 0;
    while(intWire->available())                 // device may send less than requested (abnormal)
    {
        buff[i] = intWire->read();              // receive a byte
        i++;
    } 
    int16_t valX = (int16_t) (((uint16_t)buff[1]<<8) | buff[0]);
    int16_t valY = (int16_t) (((uint16_t)buff[3]<<8) | buff[2]);
    int16_t valZ = (int16_t) (((uint16_t)buff[5]<<8) | buff[4]);

    float fact = (float)ACC_RANGE/pow(2.0,(float)ACC_RES);
    sensorData.accX = (float)valX * fact - a_offx; //Convert to G-Force
    sensorData.accY = (float)valY * fact - a_offy;
    sensorData.accZ = (float)valZ * fact - a_offz;
}
//----------------------------------------

/*
// calibrate offset of x, y and z
void compassCalibrate(void)
{
  Serial << ">>>> calibrate the compass\n";
  
  MagnetometerScaled valueMax = {0, 0, 0};
  MagnetometerScaled valueMin = {0, 0, 0};

  // calculate x, y and z offset
  
  Serial << "please rotate the compass" << endl;
  
  int xcount = 0;
  int ycount = 0;
  int zcount = 0;
  boolean xZero = false;
  boolean yZero = false;
  boolean zZero = false;
  MagnetometerScaled value;
  while (xcount < 3 || ycount < 3 || zcount < 3) {
    value = compass.readScaledAxis();
    if ((fabs(value.XAxis) > 600) || (fabs(value.YAxis) > 600) || (fabs(value.ZAxis) > 600)) {
      continue;
    }
    
    if (valueMin.XAxis > value.XAxis) {
      valueMin.XAxis = value.XAxis;
    } else if (valueMax.XAxis < value.XAxis) {
      valueMax.XAxis = value.XAxis;
    }
    
    if (valueMin.YAxis > value.YAxis) {
      valueMin.YAxis = value.YAxis;
    } else if (valueMax.YAxis < value.YAxis) {
      valueMax.YAxis = value.YAxis;
    }
    
    if (valueMin.ZAxis > value.ZAxis) {
      valueMin.ZAxis = value.ZAxis;
    } else if (valueMax.ZAxis < value.ZAxis) {
      valueMax.ZAxis = value.ZAxis;
    }
    
    
    if (xZero) {
      if (fabs(value.XAxis) > 50) {
        xZero = false;
        xcount++;
      }
    } else {
      if (fabs(value.XAxis) < 40) {
        xZero = true;
      }
    }
    
    if (yZero) {
      if (fabs(value.YAxis) > 50) {
        yZero = false;
        ycount++;
      }
    } else {
      if (fabs(value.YAxis) < 40) {
        yZero = true;
      }
    }
    
    if (zZero) {
      if (fabs(value.ZAxis) > 50) {
        zZero = false;
        zcount++;
      }
    } else {
      if (fabs(value.ZAxis) < 40) {
        zZero = true;
      }
    }
    
    delay(30);
  }
  
  valueOffset.XAxis = (valueMax.XAxis + valueMin.XAxis) / 2;
  valueOffset.YAxis = (valueMax.YAxis + valueMin.YAxis) / 2;
  valueOffset.ZAxis = (valueMax.ZAxis + valueMin.ZAxis) / 2;
  
  Serial << "max: " << valueMax.XAxis << '\t' << valueMax.YAxis << '\t' << valueMax.ZAxis << endl;
  Serial << "min: " << valueMin.XAxis << '\t' << valueMin.YAxis << '\t' << valueMin.ZAxis << endl;
  Serial << "offset: " << valueOffset.XAxis << '\t' << valueOffset.YAxis << '\t' << valueOffset.ZAxis << endl;
  
  Serial << "<<<<" << endl;
}
*/



void GY_85::SetCompass()
{
    //Set Data Rate
    intWire->beginTransmission( HMC5883 );      //open communication with HMC5883
    intWire->write( 0x00 );                     //select mode register
    intWire->write( 0xB4);                     //continuous measurement mode
    intWire->endTransmission();    
    //Set Gain
    intWire->beginTransmission( HMC5883 );      //open communication with HMC5883
    intWire->write( 0x01 );                     //select mode register
    intWire->write( 0x80);                     //continuous measurement mode
    intWire->endTransmission();    

    //Put the HMC5883 IC into the correct operating mode
    intWire->beginTransmission( HMC5883 );      //open communication with HMC5883
    intWire->write( 0x02 );                     //select mode register
    intWire->write( 0x00 );                     //continuous measurement mode
    intWire->endTransmission();
}

void GY_85::readFromCompass()
{
    byte buff[8];
    
    //Tell the HMC5883 where to begin reading data
    intWire->beginTransmission( HMC5883 );
    intWire->write( 0x03 );               //select register 3, X MSB register
    intWire->endTransmission();

  intWire->requestFrom( HMC5883, 6 );         // request 6 bytes from device

    uint8_t i = 0;
    while(intWire->available())                 // device may send less than requested (abnormal)
    {
        buff[i] = intWire->read();              // receive a byte
        i++;
    } 

    int16_t valX = (int16_t) (((uint16_t)buff[0]<<8) | buff[1]);
    int16_t valZ = (int16_t) (((uint16_t)buff[2]<<8) | buff[3]);
    int16_t valY = (int16_t) (((uint16_t)buff[4]<<8) | buff[5]);

    /*
    Serial.print("Mag Raw X:");
    Serial.print(valX);
    Serial.print(" Y:");
    Serial.print(valY);
    Serial.print(" Z:");
    Serial.println(valZ);
    */

    sensorData.magX = (float)valX / 440.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT
    sensorData.magY = (float)valY / 440.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT
    sensorData.magZ = (float)valZ / 440.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT

}

//----------------------------------------

float g_offx = 0;
float g_offy = 0;
float g_offz = 0;


void GY_85::SetGyro()
{
    intWire->beginTransmission( ITG3200 );
    intWire->write( 0x3E );
    intWire->write( 0x00 );
    intWire->endTransmission();
    
    intWire->beginTransmission( ITG3200 );
    intWire->write( 0x15 );
    intWire->write( 0x07 );
    intWire->endTransmission();
    
    intWire->beginTransmission( ITG3200 );
    intWire->write( 0x16 );
    intWire->write( 0x1C );                         // +/- 2000 dgrs/sec, 1KHz, 1E, 19
    intWire->endTransmission();
    
    intWire->beginTransmission( ITG3200 );
    intWire->write( 0x17 );
    intWire->write( 0x00 );
    intWire->endTransmission();
    
    delay(10);
    
}

void GY_85::GyroCalibrate()
{
    float tmpx = 0;
    float tmpy = 0;
    float tmpz = 0;
    
    g_offx = 0;
    g_offy = 0;
    g_offz = 0;
    delay(200);
    
    for( uint8_t i = 0; i < 10; i ++ ) //take the mean from 10 gyro probes and divide it from the current probe
    {
        delay(10);
        readGyro();
        tmpx += sensorData.gyroX;
        tmpy += sensorData.gyroY;
        tmpz += sensorData.gyroZ;
    }
    g_offx = tmpx/10.0;
    g_offy = tmpy/10.0;
    g_offz = tmpz/10.0;

    Serial.print("GOX:");
    Serial.print(g_offx);
    Serial.print("GOY:");
    Serial.print(g_offy);
    Serial.print("GOZ:");
    Serial.println(g_offz);
}

void GY_85::readGyro()
{  
    intWire->beginTransmission( ITG3200 );
    intWire->write( 0x1B );
    intWire->endTransmission();

    intWire->requestFrom( ITG3200, 8 );             // request 8 bytes from ITG3200
    
    int i = 0;
    byte buff[8];
    while(intWire->available())
    {
        buff[i] = intWire->read();
        i++;
    }
    
    int16_t temp = (int16_t) (((uint16_t)buff[0]<<8) | buff[1]);
    sensorData.temp  = 35.0 + ((float)temp + 13200.0)/280.0;

    int16_t valX = (int16_t) (((uint16_t)buff[2]<<8) | buff[3]);
    sensorData.gyroX = (float)valX;
    sensorData.gyroX = (sensorData.gyroX / 14.375) - g_offx;

    int16_t valY = (int16_t) (((uint16_t)buff[4]<<8) | buff[5]);
    sensorData.gyroY = (float)valY;
    sensorData.gyroY = (sensorData.gyroY / 14.375) - g_offy; 

    int16_t valZ = (int16_t) (((uint16_t)buff[6]<<8) | buff[7]);
    sensorData.gyroZ = (float)valZ;
    sensorData.gyroZ = (sensorData.gyroZ / 14.375) - g_offz; 
   
}

void GY_85::init(TwoWire* tw)
{
    intWire = tw;
    SetAccelerometer();
    SetCompass();
    SetGyro();
}


  void GY_85::calibrate()
  {
    GyroCalibrate();
    AccCalibrate();
  }