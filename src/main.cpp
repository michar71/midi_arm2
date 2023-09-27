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
#include <DNSServer.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>

//WiFiManager wm;
DNSServer dnsServer;
AsyncWebServer server(80);
#endif

void calibrate_buttons();


//-------------------------------
// Add Very Long Press Mag Calib
// Store/load Mag Calib
//-------------------------------

//Sensor Stuff
//------------
MPU9250 mpu;

extern setup_t settings;


//I2C Stuff
//---------
#define SDA 7
#define SCL 5

#define DATA_PIN 39
#define BUT_CTRL 9
#define BUT_A 3
#define BUT_B 11



#define STATUS_LED LED_BUILTIN //Asuming D2 here...

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

t_state state = STATE_STARTUP;
t_state lastState = STATE_STARTUP;


void store_mpu_data()
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

void setState(t_state newState)
{
  lastState = state;
  state = newState;
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

  if (state == STATE_LIVE)
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

#ifdef WIFI

class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler() {}
  virtual ~CaptiveRequestHandler() {}

  bool canHandle(AsyncWebServerRequest *request){
    //request->addInterestingHeader("ANY");
    return true;
  }

  void handleRequest(AsyncWebServerRequest *request) {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Setup Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Main Page</title></head><body>");
    response->printf("<h1>%s FW V%d.%d Webserver running.</h1><br>",devicename,maj_ver,min_ver);
    response->print("<p>This is out captive portal front page.</p>");
    response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
    response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());

    response->print("<a href=\"http://192.168.0.1/update\">Update</a><br><br>");
    response->print("<a href=\"http://192.168.0.1/settings\">Settings</a><br><br>");
    response->print("<a href=\"http://192.168.0.1/reset\">Reset Settings</a><br><br>");
    response->print("<a href=\"http://192.168.0.1/calgyroacc\">Calibrate Gyro/Accelerometer</a><br><br>");    
    response->print("<a href=\"http://192.168.0.1/calmag\">Calibrate Magnetometer</a><br><br>");       
    response->print("<a href=\"http://192.168.0.1/calbuttons\">Calibrate Buttons</a><br><br>");       
    response->print("</body></html>");
    request->send(response);

  }
};

void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Page Not found");
}

void setup_settings_webpage()
{
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Settings Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Main Page</title></head><body>");

    response->print("<h1>Settings</h1><br>");
          
    response->printf("Acc Bias X: %f <br>",settings.main_acc_bias_x);
    response->printf("Acc Bias Y: %f <br>",settings.main_acc_bias_y);
    response->printf("Acc Bias Z: %f <br>",settings.main_acc_bias_z);

    response->printf("Gyro Bias X: %f <br>",settings.main_gyro_bias_x);
    response->printf("Gyro Bias Y: %f <br>",settings.main_gyro_bias_y);
    response->printf("Gyro Bias Z: %f <br>",settings.main_gyro_bias_z);        

    response->printf("Mag Bias X: %f <br>",settings.main_mag_bias_x);    
    response->printf("Mag Bias Y: %f <br>",settings.main_mag_bias_y);    
    response->printf("Mag Bias Z: %f <br>",settings.main_mag_bias_z);    

    response->printf("Mag Scale X: %f <br>",settings.main_mag_scale_x);   
    response->printf("Mag Scale Y: %f <br>",settings.main_mag_scale_y);   
    response->printf("Mag Scale Z: %f <br>",settings.main_mag_scale_z);   

    response->printf("Touch TH CTRL: %d <br>",settings.th_but_ctrl); 
    response->printf("Touch TH A: %d <br>",settings.th_but_a); 
    response->printf("Touch TH B: %d <br>",settings.th_but_b); 

    response->print("<h1>Debug Data</h1><br>");
    response->printf("Free Heap: %d <br>",ESP.getFreeHeap());     
    response->printf("BUTTON CTRL VAL: %d <br>",touchRead(BUT_CTRL));  
    response->printf("BUTTON A VAL: %d <br>",touchRead(BUT_A));  
    response->printf("BUTTON B VAL: %d <br>",touchRead(BUT_B));  
    request->send(response);
  });
}

void setup_main_webpage()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Main Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Main Page</title></head><body>");
    response->printf("<h1>%s FW V%d.%d Webserver running.</h1><br>",devicename,maj_ver,min_ver);
    response->print("<a href=\"http://192.168.0.1/update\">Update</a><br><br>");
    response->print("<a href=\"http://192.168.0.1/setup\">Setup</a><br><br>");
    response->print("<a href=\"http://192.168.0.1/reset\">Reset Settings</a><br><br>");
    response->print("<a href=\"http://192.168.0.1/calgyroacc\">Calibrate Gyro/Accelerometer</a><br><br>");    
    response->print("<a href=\"http://192.168.0.1/calmag\">Calibrate Magnetometer</a><br><br>");       
    response->print("<a href=\"http://192.168.0.1/calbuttons\">Calibrate Buttons</a><br><br>");         
    response->print("</body></html>");
    request->send(response);
  });
}

void setup_reset_webpage()
{

   server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request){

      request->send(200, "text/plain", "Settings Reset");
      init_settings_acc_gyro();
      init_settings_mag();
      init_settings_but();
      save_settings();
      setLED(1,64,64,64);
      delay(250);
      setLED(0,0,0,0);
    });
}

void setup_cal_gyro_acc_webpage()
{
  server.on("/calgyroacc", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Calibrating Gyro/Accelerometer");
    delay(1000);
    setState(STATE_CAL_GYRO);
  });
}

void setup_cal_mag_webpage()
{
  server.on("/calmag", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Calibrating Magnetometer");
    delay(1000);
    setState(STATE_CAL_MAG);   
  });
}

void setup_cal_buttons_webpage()
{
  server.on("/calbuttons", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Calibrating Buttons");
    setState(STATE_CAL_BUTTONS); 
  });
}


void setup_captive_webpage()
{
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
}


#endif


void setup() 
{
    setState(STATE_STARTUP);
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

    dnsServer.start(53, "*", WiFi.softAPIP()); 

    //web pages
    //setup_captive_webpage();
    server.onNotFound(notFound);
    setup_reset_webpage();
    setup_settings_webpage();
    setup_main_webpage();
    setup_cal_gyro_acc_webpage();
    setup_cal_mag_webpage();
    setup_cal_buttons_webpage();


    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    server.begin();
    Serial.println("HTTP server started");    
  #endif

  if (load_settings())
  {
    mpu.setAccBias(settings.main_acc_bias_x ,settings.main_acc_bias_y ,settings.main_acc_bias_z);
    mpu.setGyroBias(settings.main_gyro_bias_x,settings.main_gyro_bias_y,settings.main_gyro_bias_z);
    mpu.setMagBias(settings.main_mag_bias_x,settings.main_mag_bias_y,settings.main_mag_bias_z);
    mpu.setMagScale(settings.main_mag_scale_x,settings.main_mag_scale_y,settings.main_mag_scale_z);
    but_ctrl.setTouchThreshold(settings.th_but_ctrl);
    delay(120);
    setLED(0,0,64,0);
    delay(120);
    setLED(0,0,0,0);


    if ((settings.th_but_ctrl == 0) || (settings.th_but_a == 0) || (settings.th_but_b == 0) || (settings.th_but_ctrl == 65535) || (settings.th_but_a == 65535) || (settings.th_but_b == 65535))
    {
      setLED(1,255,0,255);
      calibrate_buttons();
      delay(1000);          
      save_settings();  
    }


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


  #ifdef DEBUG
    Serial.println("Setup Done...");
  #endif    

  setState(STATE_WAITCONN);
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
      setLED(0,0,64,0);
      setState(STATE_LIVE);
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

//We need to illimate high outliers...
#define CUTOFF 40000
void calibrate_buttons()
{
  uint16_t min[3] = {0xFFFF,0xFFFF,0xFFFF};
  uint16_t max[3] = {0,0,0};
  uint16_t val = 0;

  //Do cal for 30 sec
  for (int ii=0;ii<700;ii++)
  {
    val = touchRead(BUT_A);
    if (val<CUTOFF)
    {
      if (val > max[0])
        max[0] = val;
      if (val < min[0])
        min[0] = val;
    }
    val = touchRead(BUT_B);
    if (val<CUTOFF)
    {
      if (val > max[1])
        max[1] = val;
      if (val < min[1])
        min[1] = val;
    }
    val = touchRead(BUT_CTRL);
    if (val<CUTOFF)
    {
      if (val > max[2])
        max[2] = val;
      if (val < min[2])
        min[2] = val;
    }
    delay(20);
  }
  //Take average of min/max
  //(Is this a good idea? Do we need to exclude outlieers?)
  settings.th_but_a = (min[0] + max[0])/2;
  settings.th_but_b = (min[1] + max[1])/2;
  settings.th_but_ctrl = (min[2] + max[2])/2;    
  but_ctrl.setTouchThreshold(settings.th_but_ctrl);
}


void process_state(void)
{
  switch(state)
  {
    case STATE_STARTUP:
    
    break;

    case STATE_WAITCONN:
      setLED(0,64,64,0);
    break;

    case STATE_LIVE:
      send_processing_data(true);
      setLED(1,0,64,0);
    break;

    case STATE_PAUSED:
      send_processing_data(false);
      setLED(1,64,0,0);
    break;

    case STATE_CAL_BUTTONS:
      setLED(1,255,0,255);
      init_settings_but();
      calibrate_buttons();        
      save_settings();    
    #ifdef DEBUG    
      Serial.println("Button Calib Done...");
      print_settings();
      #endif   
      setState(lastState);
    break;

    case STATE_CAL_GYRO:
      setLED(1,0,0,255);
      delay(500);
      init_settings_acc_gyro();
      mpu.calibrateAccelGyro();
      store_mpu_data();
      save_settings();
    #ifdef DEBUG    
      Serial.println("Sensor Calib Gyro/Acc Done...");
      print_settings();
    #endif  
      setState(lastState);
    break;

    case STATE_CAL_MAG:
      setLED(1,0,255,255);
      init_settings_mag();
      mpu.calibrateMag();
      store_mpu_data();
      save_settings();
    #ifdef DEBUG    
      Serial.println("Sensor Calib Mag Done...");
      print_settings();
    #endif  
      setState(lastState);          
    break;

    case STATE_UPDATE:

    break;
  }
}

void loop() 
{
  //Check Mode button
  mode_button_e button_res;

#ifdef WIFI
  //wm.process();
   dnsServer.processNextRequest();
#endif

  serial_info_request();
  process_state();


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
          if (state == STATE_LIVE)
            setState(STATE_PAUSED);
          else if (state == STATE_PAUSED)  
            setState(STATE_LIVE);
        }  
        else if (button_res == LONG_PRESS)
        {
          setState(STATE_CAL_GYRO);
        }
        else if (button_res == VERY_LONG_PRESS)
        {   
          setState(STATE_CAL_MAG);
        }
        else if (button_res == VERY_VERY_LONG_PRESS)
        {
          setState(STATE_CAL_BUTTONS);
        }


        if (touchRead(BUT_A) < settings.th_but_a)
          but_a_state = false;
        else
          but_a_state = true; 
        if (touchRead(BUT_B) < settings.th_but_b)
          but_b_state = false;
        else
          but_b_state = true; 

      }
  }

  //Heartbeat
  EVERY_N_SECONDS(1)
  {
    led_state = !led_state;
    digitalWrite(STATUS_LED, led_state);
  }
}

