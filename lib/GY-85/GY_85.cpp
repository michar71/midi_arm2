#include "GY_85.h"


//TODO
//Need to add mag Calibration
//Need to add Acc Calibration

//Need to add Load/Store offset from calibration

typedef struct{
  float XAxis;
  float YAxis;
  float ZAxis;
}
MagnetometerScaled;

float a_offx = 0;
float a_offy = 0;
float a_offz = 0;


float g_offx = 0;
float g_offy = 0;
float g_offz = 0;

float m_offx = 0;
float m_offy = 0;
float m_offz = 0;

size_t n_filter_iter {1};
float q[4] = {1.0f, 0.0f, 0.0f, 0.0f};  // vector to hold quaternion
float rpy[3] {0.f, 0.f, 0.f};
float lin_acc[3] {0.f, 0.f, 0.f};  // linear acceleration (acceleration with gravity component subtracted) 
float magnetic_declination = 0;  // None, only measure relative...
QuaternionFilter quat_filter;


float GY_85::getGyroXoffset(void)
{
   return g_offx;
}

float GY_85::getGyroYoffset(void)
{
   return g_offy;
}

float GY_85::getGyroZoffset(void)
{
   return g_offz;
}

void GY_85::setGyroOffset(float offsX,float offsY,float offsZ)
{
  g_offx = offsX;
  g_offy = offsY;
  g_offz = offsZ;
}

float GY_85::getAccXoffset(void)
{
   return a_offx;
}

float GY_85::getAccYoffset(void)
{
   return a_offy;
}

float GY_85::getAccZoffset(void)
{
   return a_offz;
}

void GY_85::setAccOffset(float offsX,float offsY,float offsZ)
{
  a_offx = offsX;
  a_offy = offsY;
  a_offz = offsZ;
}

float GY_85::getMagXoffset(void)
{
   return m_offx;
}

float GY_85::getMagYoffset(void)
{
   return m_offy;
}

float GY_85::getMagZoffset(void)
{
   return m_offz;
}

void GY_85::setMagOffset(float offsX,float offsY,float offsZ)
{
  m_offx = offsX;
  m_offy = offsY;
  m_offz = offsZ;
}

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
    a_offz = a_offz + 1.0;  //(Add Gravity back... But make it positive becaue board is mounted upside down)

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


// calibrate offset of x, y and z
void GY_85::compassCalibrate(void)
{
 
  Serial.print("Calibrate the compass");
  
  MagnetometerScaled valueMax = {-100000, -100000, -100000};
  MagnetometerScaled valueMin = {100000, 100000, 100000};


  // calculate x, y and z offset
  Serial.print("please rotate the compass");
  
  int count = 0;
  MagnetometerScaled value;
  m_offx = 0;
  m_offy = 0;
  m_offz = 0;

  while (count < 2000) 
  {
    readRawFromCompass();
    value.XAxis = sensorData.magX;
    value.YAxis = sensorData.magY;
    value.ZAxis = sensorData.magZ;

    if ((fabs(value.XAxis) > 32600) || (fabs(value.YAxis) > 32600) || (fabs(value.ZAxis) > 32600)) 
    {
      continue;
    }
    
    if (valueMin.XAxis > value.XAxis) 
    {
      valueMin.XAxis = value.XAxis;
    } 
    else if (valueMax.XAxis < value.XAxis) 
    {
      valueMax.XAxis = value.XAxis;
    }
    
    if (valueMin.YAxis > value.YAxis) 
    {
      valueMin.YAxis = value.YAxis;
    } 
    else if (valueMax.YAxis < value.YAxis) 
    {
      valueMax.YAxis = value.YAxis;
    }
    
    if (valueMin.ZAxis > value.ZAxis) 
    {
      valueMin.ZAxis = value.ZAxis;
    } 
    else if (valueMax.ZAxis < value.ZAxis) 
    {
      valueMax.ZAxis = value.ZAxis;
    }
    
    count++;
    delay(10);
  }
  
  m_offx = (valueMax.XAxis + valueMin.XAxis) / 2;
  m_offy = (valueMax.YAxis + valueMin.YAxis) / 2;
  m_offz = (valueMax.ZAxis + valueMin.ZAxis) / 2;

  Serial.print("Mag cal offset X:");
  Serial.print(m_offx);
  Serial.print(" Y:");
  Serial.print(m_offy);
  Serial.print(" Z:");
  Serial.print(m_offz);
  Serial.print(" Max X:");
  Serial.print(valueMax.XAxis);
  Serial.print(" Max Y:");
  Serial.print(valueMax.YAxis);
  Serial.print(" Maz Z:");
  Serial.print(valueMax.ZAxis);
  Serial.print(" Min X:");
  Serial.print(valueMin.XAxis);
  Serial.print(" Min Y:");
  Serial.print(valueMin.YAxis);
  Serial.print(" Min Z:");
  Serial.println(valueMin.ZAxis);


  readFromCompass();

}



void GY_85::SetCompass()
{
  if (mag_addr == HMC5883)
  {
    //Set Data Rate
    intWire->beginTransmission( mag_addr );      //open communication with HMC5883
    intWire->write( 0x00 );                     //select Config A Register
    intWire->write( 0x34);                     //0 01 101 00     -> Sample 2,Sample Rate 30Hz, Normal Mode
    intWire->endTransmission();    
    //Set Gain
    intWire->beginTransmission( mag_addr );      //open communication with HMC5883
    intWire->write( 0x01 );                     //Config B Register
    intWire->write( 0x40);                     //010 00000 Gain +/- 1.9Ga 
    intWire->endTransmission();    

    //Put the HMC5883 IC into the correct operating mode
    intWire->beginTransmission( mag_addr );      //open communication with HMC5883
    intWire->write( 0x02 );                     //select mode register
    intWire->write( 0x00 );                     //000000 00 continuous measurement mode
    intWire->endTransmission();
  }
  else
  {
    //Put the QMC5883 IC into the correct operating mode
    intWire->beginTransmission( mag_addr );      //open communication with QMC5883
    intWire->write( 0x09 );                     //select mode register
    intWire->write( 0x0D );                     //00 00 11 01-> Oversample 64, +/- 2Ga, Output Data rate 100Hz, Continous
    intWire->endTransmission();
  }
}


bool GY_85::idCompass(void)
{
    byte buff[3];
    
    //Try HMC5883
    intWire->beginTransmission(mag_addr);
    intWire->write( 0x0A );               //select register 0x0A, X MSB register
    byte error = intWire->endTransmission();

    if (error != 0)
    {
      Serial.println("No HMC5338 found, trying QMC5338");
      mag_addr = QMC5883;
      //Try QMC5883
      intWire->beginTransmission(mag_addr);
      intWire->write( 0x0D );               //select register 0x0A, X MSB register
      error = intWire->endTransmission();

      if (error != 0)
      {
        Serial.println("No QMC5338, giving up");
        return false;
      }
    }

    if (mag_addr == HMC5883)
    {
      intWire->requestFrom( mag_addr, 3 );         // request 6 bytes from device
      uint8_t i = 0;
      Serial.print("Compass ID ");
      while(intWire->available())                 // device may send less than requested (abnormal)
      {
        buff[i] = intWire->read();              // receive a byte
        i++;
        Serial.print("Reg:");
        Serial.print(i);
        Serial.print(" Val:");
        Serial.print(buff[i]);
      }
      Serial.println("");

      if ((buff[0] == 0x48)&&(buff[1] == 0x34)&&(buff[2] == 0x33))
        return true;
      else 
        return false;  
    }
    else
    {
      intWire->requestFrom( mag_addr, 1);         // request 6 bytes from device
      uint8_t i = 0;
      Serial.print("Compass ID ");
      while(intWire->available())                 // device may send less than requested (abnormal)
      {
        buff[i] = intWire->read();              // receive a byte
        i++;
        Serial.print("Reg:");
        Serial.print(i);
        Serial.print(" Val:");
        Serial.print(buff[i]);
      }
      Serial.println("");

      if (buff[0] == 0xFF)
        return true;
      else 
        return false;  
    }

}

bool GY_85::idAcc(void)
{
    byte buff[1];
    
    //Try ADXL345
    intWire->beginTransmission(ADXL345);
    intWire->write( 0x00 );               //select register 0x0A, X MSB register
    byte error = intWire->endTransmission();

      intWire->requestFrom( ADXL345, 1);         // request 6 bytes from device
      uint8_t i = 0;
      Serial.print("Acc ID ");
      while(intWire->available())                 // device may send less than requested (abnormal)
      {
        buff[i] = intWire->read();              // receive a byte
        i++;
        Serial.print("Reg:");
        Serial.print(i);
        Serial.print(" Val:");
        Serial.print(buff[i]);
      }
      Serial.println("");

      if (buff[0] == 0xE5)
        return true;
      else 
        return false;  
}

bool GY_85::idGyro(void)
{
    byte buff[1];
    
    //Try ITG3200
    intWire->beginTransmission(ITG3200);
    intWire->write( 0x00 );               //select register 0x0A, X MSB register
    byte error = intWire->endTransmission();

      intWire->requestFrom( ITG3200, 1);         // request 6 bytes from device
      uint8_t i = 0;
      Serial.print("Acc ID ");
      while(intWire->available())                 // device may send less than requested (abnormal)
      {
        buff[i] = intWire->read();              // receive a byte
        i++;
        buff[i] = buff[i] | 0x81;
        Serial.print("Reg:");
        Serial.print(i);
        Serial.print(" Val:");
        Serial.print(buff[i]);
      }
      Serial.println("");

      

      if (buff[0] == 0xE9)
        return true;
      else 
        return false;  
}

bool GY_85::compassReady(void)
{
    byte data = 0;

    intWire->beginTransmission( mag_addr );
    intWire->write( 0x06 );               //select register 3, X MSB register
    if (0 != intWire->endTransmission())
      return false;

    intWire->requestFrom( mag_addr, 1 );         // request 6 bytes from device

    uint8_t i = 0;
    while(intWire->available())                 // device may send less than requested (abnormal)
    {
        data = intWire->read();              // receive a byte
        i++;
    } 

    #define QMC5883L_STATUS_DRDY 1
    if (data & QMC5883L_STATUS_DRDY)
      return true;
      else
      return false;
}


void GY_85::readFromCompass()
{
    byte buff[8];

  if (mag_addr == HMC5883)
  {
    intWire->beginTransmission( mag_addr );
    intWire->write( 0x03 );               //select register 3, X MSB register
    intWire->endTransmission();

  intWire->requestFrom( mag_addr, 6 );         // request 6 bytes from device

    uint8_t i = 0;
    while(intWire->available())                 // device may send less than requested (abnormal)
    {
        buff[i] = intWire->read();              // receive a byte
        i++;
    } 

    int16_t valX = (int16_t) (((uint16_t)buff[0]<<8) | buff[1]);
    int16_t valY = (int16_t) (((uint16_t)buff[2]<<8) | buff[3]);
    int16_t valZ = (int16_t) (((uint16_t)buff[4]<<8) | buff[5]);

    float cvalx = (float) valX - m_offx;
    float cvaly = (float) valY - m_offy;
    float cvalz = (float) valZ - m_offz;

    /*
    Serial.print("Mag Raw X:");
    Serial.print(valX);
    Serial.print(" Y:");
    Serial.print(valY);
    Serial.print(" Z:");
    Serial.println(valZ);
    */

    //Max Range is +/-2.5Ga
    sensorData.magX = cvalx / 820.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT
    sensorData.magY = cvaly / 820.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT
    sensorData.magZ = cvalz / 820.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT
  }
  else
  {
    if (compassReady() == false)
        return;
    intWire->beginTransmission( mag_addr );
    intWire->write( 0x00 );               //select register 3, X MSB register
    intWire->endTransmission();

    intWire->requestFrom( mag_addr, 6 );         // request 6 bytes from device

    uint8_t i = 0;
    while(intWire->available())                 // device may send less than requested (abnormal)
    {
        buff[i] = intWire->read();              // receive a byte
        i++;
    } 

    int16_t valX = (int16_t) (((uint16_t)buff[1]<<8) | buff[0]);
    int16_t valY = (int16_t) (((uint16_t)buff[3]<<8) | buff[2]);
    int16_t valZ = (int16_t) (((uint16_t)buff[5]<<8) | buff[4]);
    
    /*
    Serial.print(valX);
    Serial.print(",");
    Serial.print(valY);
    Serial.print(",");
    Serial.println(valZ);
    */

    float cvalx = (float) valX - m_offx;
    float cvaly = (float) valY - m_offy;
    float cvalz = (float) valZ - m_offz;

    //Not sure if the math checks out on this.....
    //Max Range is +/- 2Ga
    sensorData.magX = cvalx / 16383.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT
    sensorData.magY = cvaly / 16383.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT
    sensorData.magZ = cvalz / 16383.0 * 100;   //Gauss -> microTeslas... 1Ga = 100mT

    /*
    Serial.print(sensorData.magX);
    Serial.print(",");
    Serial.print(sensorData.magY);
    Serial.print(",");
    Serial.println(sensorData.magZ);
    */
  }
}


void GY_85::readRawFromCompass()
{
    byte buff[8];

    if (mag_addr == HMC5883)
    {  
    
      //Tell the HMC5883 where to begin reading data
      intWire->beginTransmission( mag_addr );
      intWire->write( 0x03 );               //select register 3, X MSB register
      intWire->endTransmission();

    intWire->requestFrom( mag_addr, 6 );         // request 6 bytes from device

      uint8_t i = 0;
      while(intWire->available())                 // device may send less than requested (abnormal)
      {
          buff[i] = intWire->read();              // receive a byte
          i++;
      } 

      int16_t valX = (int16_t) (((uint16_t)buff[0]<<8) | buff[1]);
      int16_t valY = (int16_t) (((uint16_t)buff[2]<<8) | buff[3]);
      int16_t valZ = (int16_t) (((uint16_t)buff[4]<<8) | buff[5]);

      sensorData.magX = (float) valX;
      sensorData.magY = (float) valY;
      sensorData.magZ = (float) valZ;
    }
    else
    {
      //Tell the HMC5883 where to begin reading data
      intWire->beginTransmission( mag_addr );
      intWire->write( 0x00 );               //select register 3, X MSB register
      intWire->endTransmission();

      intWire->requestFrom( mag_addr, 6 );         // request 6 bytes from device

      uint8_t i = 0;
      while(intWire->available())                 // device may send less than requested (abnormal)
      {
          buff[i] = intWire->read();              // receive a byte
          i++;
      } 

      int16_t valX = (int16_t) (((uint16_t)buff[1]<<8) | buff[0]);
      int16_t valY = (int16_t) (((uint16_t)buff[3]<<8) | buff[2]);
      int16_t valZ = (int16_t) (((uint16_t)buff[5]<<8) | buff[4]);

      sensorData.magX = (float) valX;
      sensorData.magY = (float) valY;
      sensorData.magZ = (float) valZ;
    }
}


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

bool GY_85::init(TwoWire* tw)
{
    bool idPass = false;
    intWire = tw;
    //Probe for Components
    bool hasCompass = idCompass();
    bool hasGyro = idGyro();
    bool hasAcc = idAcc();

    //Init components
    SetAccelerometer();
    SetCompass();
    SetGyro();

    if (hasCompass && hasGyro && hasAcc)
    {
      idPass = true;
      isAvailable = true;
      Serial.println("GY-85 Ready");
    }
    return idPass;
}

bool GY_85::available(void) 
{
    return isAvailable;
}

void GY_85::calibrate()
{
  GyroCalibrate();
  AccCalibrate();
}

bool GY_85::update()
{
  if (!available()) 
  {
    Serial.println("MCU data not available");
    return false;
  }

        readFromAccelerometer();  
        readGyro();
        readFromCompass();

        // Madgwick function needs to be fed North, East, and Down direction like
        // (AN, AE, AD, GN, GE, GD, MN, ME, MD)
        // Accel and Gyro direction is Right-Hand, X-Forward, Z-Up
        // Magneto direction is Right-Hand, Y-Forward, Z-Down
        // So to adopt to the general Aircraft coordinate system (Right-Hand, X-Forward, Z-Down),
        // we need to feed (ax, -ay, -az, gx, -gy, -gz, my, -mx, mz)
        // but we pass (-ax, ay, az, gx, -gy, -gz, my, -mx, mz)
        // because gravity is by convention positive down, we need to ivnert the accel data

        // get quaternion based on aircraft coordinate (Right-Hand, X-Forward, Z-Down)
        // acc[mg], gyro[deg/s], mag [mG]
        // gyro will be convert from [deg/s] to [rad/s] inside of this function
        // quat_filter.update(-a[0], a[1], a[2], g[0] * DEG_TO_RAD, -g[1] * DEG_TO_RAD, -g[2] * DEG_TO_RAD, m[1], -m[0], m[2], q);             

        /*
        Intial alignment code
        float an = -a[0];
        float ae = +a[1];
        float ad = +a[2];
        float gn = +g[0] * DEG_TO_RAD;
        float ge = -g[1] * DEG_TO_RAD;
        float gd = -g[2] * DEG_TO_RAD;
        float mn = +m[1];
        float me = -m[0];
        float md = +m[2];
        */

        
        //Debug output
        /*
        Serial.print(" A:");
        Serial.print(accelerometer_x());
        Serial.print(":");
        Serial.print(accelerometer_y());
        Serial.print(":");
        Serial.print(accelerometer_z());
        Serial.print(" G:");                
        Serial.print(gyro_x());
        Serial.print(":");
        Serial.print(gyro_y());
        Serial.print(":");        
        Serial.print(gyro_z());
        Serial.print(" C:");
        Serial.print(compass_x());
        Serial.print(":");        
        Serial.print(compass_y());
        Serial.print(":");        
        Serial.println(compass_z());
        */

       //selectFilter(QuatFilterSel::MAHONY);

        float an = accelerometer_x();
        float ae = accelerometer_y();
        float ad = accelerometer_z();
        float gn = gyro_x() * DEG_TO_RAD;
        float ge = gyro_y() * DEG_TO_RAD;
        float gd = gyro_z() * DEG_TO_RAD;

        float mn = compass_x();
        float me = compass_y();
        float md = compass_z();

        for (size_t i = 0; i < n_filter_iter; ++i) 
        {
            quat_filter.update(an, ae, ad, gn, ge, gd, mn, me, md, q);
        }

        update_rpy(q[0], q[1], q[2], q[3]);

        //Swap pitch and roll here to compensate for V3.5 HW
        float tmp = rpy[0];
        rpy[0]= rpy[1];
        rpy[1] = tmp;

        /*
        Serial.print("Roll:");        
        Serial.print(getRoll());
        Serial.print(" Pitch:");
        Serial.print(getPitch());
        Serial.print(" Yaw:");        
        Serial.println(getYaw());
       */
        return true;  
}

void GY_85::update_rpy(float qw, float qx, float qy, float qz) {
        // Define output variables from updated quaternion---these are Tait-Bryan angles, commonly used in aircraft orientation.
        // In this coordinate system, the positive z-axis is down toward Earth.
        // Yaw is the angle between Sensor x-axis and Earth magnetic North (or true North if corrected for local declination, looking down on the sensor positive yaw is counterclockwise.
        // Pitch is angle between sensor x-axis and Earth ground plane, toward the Earth is positive, up toward the sky is negative.
        // Roll is angle between sensor y-axis and Earth ground plane, y-axis up is positive roll.
        // These arise from the definition of the homogeneous rotation matrix constructed from quaternions.
        // Tait-Bryan angles as well as Euler angles are non-commutative; that is, the get the correct orientation the rotations must be
        // applied in the correct order which for this configuration is yaw, pitch, and then roll.
        // For more see http://en.wikipedia.org/wiki/Conversion_between_quaternions_and_Euler_angles which has additional links.
        float a12, a22, a31, a32, a33;  // rotation matrix coefficients for Euler angles and gravity components
        a12 = 2.0f * (qx * qy + qw * qz);
        a22 = qw * qw + qx * qx - qy * qy - qz * qz;
        a31 = 2.0f * (qw * qx + qy * qz);
        a32 = 2.0f * (qx * qz - qw * qy);
        a33 = qw * qw - qx * qx - qy * qy + qz * qz;
        rpy[0] = atan2f(a31, a33);
        rpy[1] = -asinf(a32);
        rpy[2] = atan2f(a12, a22);
        rpy[0] *= 180.0f / PI;
        rpy[1] *= 180.0f / PI;
        rpy[2] *= 180.0f / PI;
        rpy[2] += magnetic_declination;
        if (rpy[2] >= +180.f)
            rpy[2] -= 360.f;
        else if (rpy[2] < -180.f)
            rpy[2] += 360.f;

        lin_acc[0] = accelerometer_x() + a31;
        lin_acc[1] = accelerometer_y() + a32;
        lin_acc[2] = accelerometer_z() - a33;
    }


float GY_85::getPitch()
{
  return rpy[0];
}

float GY_85::getRoll()
{
 return rpy[1];
}

float GY_85::getYaw()
{
  return rpy[2];
}

float GY_85::getLinearAccX(void)
{
  return lin_acc[0];
}

float GY_85::getLinearAccY(void)
{
  return lin_acc[1];
}
float GY_85::getLinearAccZ(void)
{
  return lin_acc[2];
}

void GY_85::selectFilter(QuatFilterSel sel) 
{
  quat_filter.select_filter(sel);
}