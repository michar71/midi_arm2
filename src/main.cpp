#define DEBUG
#define WIFI

#include "main.h"
#include <Wire.h>
#include "MPU9250.h"
#include <FastLED.h>
#include "ButtonClass.h"
#include "settings.h"

#ifdef WIFI
//#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
//WiFiManager wm;
AsyncWebServer server(80);
#endif




//-------------------------------
// Add Very Long Press Mag Calib
// Store/load Mag Calib
//-------------------------------

//Sensor Stuff
//------------
MPU9250 mpu;

extern setup_t settings;

bool challenge = false;

//I2C Stuff
//---------
#define SDA 7
#define SCL 5

#define DATA_PIN 39
#define BUT_CTRL 9
#define BUT_A 3
#define BUT_B 11

#define TOUCH_TH1 23000
#define TOUCH_TH2 41000
#define TOUCH_TH3 27000

#define STATUS_LED LED_BUILTIN //Asuming D2 here...

bool isLive = false;
int led_state = LOW;    // the current state of LED
bool but_a_state = false;
bool but_b_state = false;
bool but_c_state = false;

//LED/Button Stuff
//----------------

// How many leds in your strip?
#define NUM_LEDS 2

// For led chips like WS2812, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
// Clock pin only needed for SPI based chipsets when not using hardware SPI

// Define the array of leds
CRGB leds[NUM_LEDS];


ButtonClass but_ctrl(BUT_CTRL,true);

void store_mpu_data()
{
    settings.acc_bias_x = mpu.getAccBiasX();
    settings.acc_bias_y = mpu.getAccBiasY();
    settings.acc_bias_z = mpu.getAccBiasZ();
    settings.gyro_bias_x = mpu.getGyroBiasX();
    settings.gyro_bias_y = mpu.getGyroBiasY();
    settings.gyro_bias_z = mpu.getGyroBiasZ();
    settings.mag_bias_x = mpu.getMagBiasX();
    settings.mag_bias_y = mpu.getMagBiasY();
    settings.mag_bias_z = mpu.getMagBiasZ();
    settings.mag_scale_x = mpu.getMagScaleX();
    settings.mag_scale_y = mpu.getMagScaleY();
    settings.mag_scale_z = mpu.getMagScaleZ();
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

void setLED(uint8_t led,uint8_t r, uint8_t g, uint8_t b)
{
    // Turn the LED on, then pause
  leds[led].r= r/6;
  leds[led].g= g/6;
  leds[led].b= b/6;
  FastLED.show();
}


void send_processing_data(bool senddata)
{
  //Plotting for Processing
  if (senddata)
  {
    Serial.print("C");
    Serial.print(":");
    Serial.print(mpu.getYaw()*DEG_TO_RAD);
    Serial.print(":");
    Serial.print(mpu.getPitch()*DEG_TO_RAD);
    Serial.print(":");
    Serial.print(mpu.getRoll()*DEG_TO_RAD);


    Serial.print(":");
    Serial.print(mpu.getLinearAccX());
    Serial.print(":");
    Serial.print(mpu.getLinearAccY());
    Serial.print(":");
    Serial.print(mpu.getLinearAccZ());           
  }
  else
  {
    Serial.print("C:0:0:0:0:0:0");
  }

  if (isLive)
  {
    Serial.print(":1");
  }
  else
  {
    Serial.print(":0");  
  }
  if (but_a_state)
  {
    Serial.print(":1");
  }
  else
  {
    Serial.print(":0");  
  }
  if (but_b_state)
  {
    Serial.print(":1");
  }
  else
  {
    Serial.print(":0");  
  }
  if (but_c_state)
  {
    Serial.println(":1");
  }
  else
  {
    Serial.println(":0");  
  }
}


void setup() 
{
    Serial.begin(230400);	
    setCpuFrequencyMhz(240);

    
  #ifdef DEBUG
    delay(2000);
    Serial.setDebugOutput(true);
    Serial.println("Startup...");
  #endif
    Wire.begin(SDA, SCL,1000000);

    pinMode(STATUS_LED, OUTPUT);     
    pinMode(BUT_CTRL,INPUT);
    pinMode(BUT_A,INPUT);
    pinMode(BUT_B,INPUT);

    touchSetCycles(0x500,0x1000);
    but_ctrl.setTouchThreshold(TOUCH_TH1);
        
    delay(10);
    settings_init();
    
  #ifdef DEBUG
    Serial.println("Pin Setup Done...");
  #endif

    //Init and test LED's
    FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed

    setLED(0,64,0,0);
    delay(200);
    setLED(0,0,64,0);
    delay(200);    
    setLED(0,0,0,64);
    delay(200);
    setLED(0,0,0,0);

  #ifdef DEBUG
    Serial.println("LED Setup Done...");
  #endif
    i2c_scan(&Wire);
    
    if (!mpu.setup(0x68)) {  // change to your own address
        Serial.println("ERROR");
        setLED(0,64,0,0);
        delay(5000);
        while(1)
        {}
    }

    delay(10);


  #ifdef DEBUG
    Serial.println("Sensor Setup Done...");
  #endif    


  #ifdef WIFI
    
    
    /*
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
    wm.setConfigPortalBlocking(false);
    wm.setConfigPortalTimeout(60);
    //automatically connect using saved credentials if they exist
    //If connection fails it starts an access point with the specified name
    if(wm.autoConnect("BABOI"))
    {
        Serial.println("connected...yeey :)");
    }
    else 
    {
        Serial.println("Configportal running");
    }
    */

    const char* ssid     = "BABOI";

    WiFi.softAP(ssid);
    IPAddress IP = WiFi.softAPIP();
    delay(100);
    Serial.println("Setting the AP");
    IPAddress Ip(192, 168, 0, 1);    //setto IP Access Point same as gateway
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(Ip, Ip, NMask);


    Serial.print("AP IP address: ");
    Serial.println(IP);

    //Tiny test page...
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
      request->send(200, "text/plain", "Hi. BABOI Webserver is running!");
    });

    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    server.begin();
    Serial.println("HTTP server started");    
  #endif

    if((touchRead(BUT_A) > TOUCH_TH2) && (touchRead(BUT_B) > TOUCH_TH3))
    {
      #ifdef WIFI
      //wm.resetSettings();
      #endif
      init_settings_acc_gyro();
      init_settings_mag();
      store_mpu_data();
      save_settings();
      setLED(1,64,64,64);
      delay(250);
      setLED(0,0,0,0);

      #ifdef DEBUG
        Serial.println("Settings Reset");
        print_settings();
      #endif      
    }
    else
    {
      load_settings();
      if ((settings.magic0 == 'M') && (settings.magic1 == 'A') && (settings.magic2 == 'G') && (settings.magic3 == 'I') && (settings.magic4 == 'C'))
      {
        mpu.setAccBias(settings.acc_bias_x ,settings.acc_bias_y ,settings.acc_bias_z);
        mpu.setGyroBias(settings.gyro_bias_x,settings.gyro_bias_y,settings.gyro_bias_z);
        mpu.setMagBias(settings.mag_bias_x,settings.mag_bias_y,settings.mag_bias_z);
        mpu.setMagScale(settings.mag_scale_x,settings.mag_scale_y,settings.mag_scale_z);
        delay(120);
        setLED(0,0,64,0);
        delay(120);
        setLED(0,0,0,0);

    #ifdef DEBUG
      Serial.println("Sensor Calib Load Done...");
    #endif      
      }
      else
      {
        init_settings_acc_gyro();
        init_settings_mag();
        store_mpu_data();
        save_settings();
        delay(400);
        setLED(0,64,0,0);
        delay(400);
        setLED(0,0,0,0);

    #ifdef DEBUG
      Serial.println("Sensor Calib Load Failed...");
      print_settings();
    #endif
      }
    }
  #ifdef DEBUG
    Serial.println("Setup Done...");
  #endif    
  setLED(0,64,64,0);
}


void serial_info_request(void)
{
  char incomingByte;
  static bool filter = false;

  if (Serial.available() > 0) 
  {
    // read the incoming byte:
    incomingByte = Serial.read();
    if (incomingByte == 'Q')
    {
      for (int ii=0;ii<10;ii++)
      {
        Serial.print(devicename);
        Serial.print(":");
        Serial.print(maj_ver);
        Serial.print(":");
        Serial.println(min_ver);
      }
      setLED(0,0,0,0);
      isLive = true;
      challenge = true;
    }
    else if (incomingByte == 'S')
    {
      if (filter == false)
      {
        filter = true;
        setLED(0,64,64,0);
        mpu.selectFilter(QuatFilterSel::MAHONY);
      }
      else
      {
        filter = false;
        setLED(0,0,0,64);
        mpu.selectFilter(QuatFilterSel::MADGWICK);  
      }
    }
  } 
}

void loop() 
{
  //Check Mode button
  mode_button_e button_res;

#ifdef WIFI
  //wm.process();
#endif

  serial_info_request();

  if (mpu.update()) 
  {
      EVERY_N_MILLIS(33)
      {

        //Serial.println(touchRead(BUT_CTRL));
        //Serial.println(touchRead(BUT_A));
        //Serial.println(touchRead(BUT_B));


        button_res = but_ctrl.check_button();
        if (SHORT_PRESS == button_res)
        {
          isLive = !isLive;
        }  
        else if (button_res == LONG_PRESS)
        {
          setLED(1,0,0,255);
          delay(500);
          init_settings_acc_gyro();
          mpu.calibrateAccelGyro();
          delay(300);
          store_mpu_data();
          save_settings();
        #ifdef DEBUG    
          Serial.println("Sensor Calib Gyro/Acc Done...");
          print_settings();
        #endif  
        }
        else if (button_res == VERY_LONG_PRESS)
        {
          setLED(1,0,255,255);
          delay(1000);
          init_settings_mag();
          mpu.calibrateMag();
          delay(300);
          store_mpu_data();
          save_settings();    
        #ifdef DEBUG    
          Serial.println("Sensor Calib Mag Done...");
          print_settings();
        #endif      
        }
        else if (button_res == VERY_VERY_LONG_PRESS)
        {
          mpu.selftest();
        }


        if (touchRead(BUT_A) < TOUCH_TH2)
          but_a_state = false;
        else
          but_a_state = true; 
        if (touchRead(BUT_B) < TOUCH_TH3)
          but_b_state = false;
        else
          but_b_state = true; 


        if (isLive)
        {
          if (challenge)
            send_processing_data(true);
            setLED(1,0,64,0);
        }
        else
        {
          if (challenge)
            send_processing_data(false);
            setLED(1,64,0,0);
        }
      }
  }

  //Heartbeat
  EVERY_N_SECONDS(1)
  {
    led_state = !led_state;
    digitalWrite(STATUS_LED, led_state);
  }
}

