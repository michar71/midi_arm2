#include "main.h"
#include "ButtonClass.h"
#include "settings.h"
#include "baboi_protocol.h"
#include "baboi_sensors.h"


#ifdef CORE_DEBUG_LEVEL
#undef CORE_DEBUG_LEVEL
#endif

#define CORE_DEBUG_LEVEL 3
#define LOG_LOCAL_LEVEL ESP_LOG_DEBUG

#ifdef WIFI


  #include <WiFi.h>
  #include <WiFiManager.h> // https://github.com/tzapu/WiFiManager 
  #include <AsyncTCP.h>
  #include "baboi_webserver.h"
 // #include <DNSServer.h>  //Do not include as it clashes with WifiManager
  #include <ESPmDNS.h>

  DNSServer dnsServer;
  WiFiManager wm;

  unsigned int  timeout   = 120; // seconds to run for
  unsigned int  startTime = millis();
  bool portalRunning      = false;
  bool startAP            = false; // start AP and webserver if true, else start only webserver
#endif


const char* devicename = "BABOI";
int maj_ver = 2;
int min_ver = 0;

extern setup_t settings;



bool but_a_state = false;
bool but_b_state = false;
bool but_c_state = false;

bool hasTouchpads = false;

// Define the array of leds
CRGB leds[NUM_LEDS];

ButtonClass but_ctrl(BUT_CTRL,false);

t_state state = STATE_STARTUP;
t_state lastState = STATE_STARTUP;


void setLED(uint8_t led,uint8_t r, uint8_t g, uint8_t b)
{
    // Turn the LED on, then pause
  leds[led].r= r/6;
  leds[led].g= g/6;
  leds[led].b= b/6;
  FastLED.show();
}


void toggle_status_led(void)
{
    static int led_state = LOW;    // the current state of LED
    led_state = !led_state;
    digitalWrite(STATUS_LED, led_state);
}

void setState(t_state newState)
{
  lastState = state;
  state = newState;
}


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
    pinMode(STATUS_LED, OUTPUT);     
    pinMode(ID_PIN,INPUT_PULLUP);
        

    
  #ifdef DEBUG
    Serial.println("Pin Setup Done...");
  #endif


    if (digitalRead(ID_PIN) == LOW)
    {
      hasTouchpads = true;
 #ifdef DEBUG      
      Serial.println("Touchpads Enabled");
#endif
    }

    delay(10);
    settings_init();
/*
    pinMode(GLOVE_SCL,OUTPUT);
    pinMode(GLOVE_SDA,OUTPUT);
    while(1)
    {
      digitalWrite(GLOVE_SCL,HIGH);
      digitalWrite(GLOVE_SDA,HIGH);
      delay(50);
      digitalWrite(GLOVE_SCL,LOW);
      digitalWrite(GLOVE_SDA,LOW);
      delay(50);
      toggle_status_led();
    }
*/

    //Init and test LED's
   FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS);  // GRB ordering is assumed


    setLED(0,64,0,0);
    setLED(1,64,0,0);
    delay(100);
    setLED(0,0,64,0);
    setLED(1,0,64,0);
    delay(100);    
    setLED(0,0,0,64);
    setLED(1,0,0,64);
    delay(100);
    setLED(0,0,0,0); 
    setLED(1,0,0,0); 
    
  #ifdef DEBUG
    Serial.println("LED Setup Done...");
  #endif

    init_sensors();


  if (load_settings())
  {
    mpu_set_settings();

    if (checkForTouchpad())
    {
      but_ctrl.setTouchMode(true);
      but_ctrl.setTouchThreshold(settings.th_but_ctrl,(uint16_t)TH_CUTOFF);

      if ((settings.th_but_ctrl == 0) || (settings.th_but_a == 0) || (settings.th_but_b == 0)|| (settings.th_but_c == 0) || (settings.th_but_ctrl == 65535) || (settings.th_but_a == 65535) || (settings.th_but_b == 65535) || (settings.th_but_c == 65535))
      {
  #ifdef DEBUG
        Serial.println("orced Touchpad Calibration!");
  #endif      
        setLED(1,255,0,255);
        calibrate_buttons();
        delay(1000);          
        save_settings();  
      }

    }
    else
    {
      but_ctrl.setTouchMode(false);
      pinMode(BUT_CTRL,INPUT_PULLUP);
      pinMode(BUT_A,INPUT_PULLUP);
      pinMode(BUT_B,INPUT_PULLUP);
      pinMode(BUT_C,INPUT_PULLUP);

    }

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
    mpu_init_settings();
    init_settings_but();
    init_settings_other();
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


  #ifdef WIFI
    
    //Hmmmm Conceptually not sure how WifiMasnsger would work....
    //After it connects to a network how do we know the IP adress? 
    //How would we get to the config page afterwards?
    
    bool res = false;
    if (settings.autoConnect)
    {
      res = wm.autoConnect(devicename); // anonymous ap
      if(!res) 
      {
          Serial.println("Failed to connect");
      } 
      else 
      {
          //if you get here you have connected to the WiFi    
          Serial.println("connected...yeey :)");
      }   
    }


    if (!res)
    {
      WiFi.softAP(devicename,NULL,7);
      IPAddress IP = WiFi.softAPIP();
      delay(100);
      Serial.println("Setting the AP");
      IPAddress Ip(192, 168, 1, 1);    //set to IP Access Point same as gateway
      IPAddress NMask(255, 255, 255, 0);
      WiFi.softAPConfig(Ip, Ip, NMask);
      Serial.print("AP IP address: ");
      Serial.println(IP);
    }


    if (!MDNS.begin(devicename)) 
    {
        Serial.println("Error setting up MDNS responder!");
    }
    // Add service to MDNS-SD
    MDNS.addService("http", "tcp", 80);

    dnsServer.start(53, "*", WiFi.softAPIP()); 

    init_webserver();
    init_protocol();

  #endif


  #ifdef DEBUG
    Serial.println("Setup Done...");
  #endif    

  setState(STATE_WAITCONN);
}
 


//We need to illimate high outliers...
void calibrate_buttons()
{
  uint16_t min[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};
  uint16_t max[4] = {0,0,0,0};
  uint16_t val = 0;

  //Do cal for 15 sec
  for (int ii=0;ii<400;ii++)
  {
    val = touchRead(BUT_A);
    if (val<TH_CUTOFF)
    {
      if (val > max[0])
        max[0] = val;
      if (val < min[0])
        min[0] = val;
    }
    val = touchRead(BUT_B);
    if (val<TH_CUTOFF)
    {
      if (val > max[1])
        max[1] = val;
      if (val < min[1])
        min[1] = val;
    }
    val = touchRead(BUT_C);
    if (val<TH_CUTOFF)
    {
      if (val > max[2])
        max[2] = val;
      if (val < min[2])
        min[2] = val;
    }    
    val = touchRead(BUT_CTRL);
    if (val<TH_CUTOFF)
    {
      if (val > max[3])
        max[3] = val;
      if (val < min[3])
        min[3] = val;
    }
    delay(20);
  }
  //Take average of min/max
  //(Is this a good idea? Do we need to exclude outlieers?)
  settings.th_but_a = min[0] + ((max[0] - min[0])/3*2);
  settings.th_but_b = min[1] + ((max[1] - min[1])/3*2);
  settings.th_but_c = min[2] + ((max[2] - min[2])/3*2);
  settings.th_but_ctrl = min[3] + ((max[2] - min[3])/3*2);    
  but_ctrl.setTouchThreshold(settings.th_but_ctrl,TH_CUTOFF);
}


void process_state(void)
{
  setLED(0,0,0,0);
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
      setLED(1,0,0,0);
    #ifdef DEBUG    
      Serial.println("Button Calib Done...");
      print_settings();
      #endif   
      setState(lastState);
    break;

    case STATE_CAL_TENSION:
      setLED(1,64,64,64);
      calibrate_tension();
      save_settings();    
      setLED(1,0,0,0);
    #ifdef DEBUG    
      Serial.println("Tension Calib Done...");
      print_settings();
      #endif   
      setState(lastState);
    break;

    case STATE_CAL_GYRO:
      setLED(1,0,0,255);
      delay(500);
      mpu_cal_gyro_accel();
      save_settings();
    #ifdef DEBUG    
      Serial.println("Sensor Calib Gyro/Acc Done...");
      print_settings();
    #endif  
      setState(lastState);
    break;

    case STATE_CAL_MAG:
      setLED(1,0,255,255);
      mpu_cal_mag();
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


void handle_buttons(void)
{
  mode_button_e  button_res = but_ctrl.check_button();
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

  if (checkForTouchpad())
  {
    int val = 0;
    val = touchRead(BUT_A);
    if ((val < settings.th_but_a) && (val > (settings.th_but_a/100*60)))
      but_a_state = false;
    else if (val < TH_CUTOFF)
      but_a_state = true; 

    val = touchRead(BUT_B);  
    if ((val < settings.th_but_b) && (val > (settings.th_but_b/100*60)))
      but_b_state = false;
    else if (val < TH_CUTOFF)
      but_b_state = true; 

    val = touchRead(BUT_C);  
    if ((val < settings.th_but_c) && (val > (settings.th_but_c/100*60)))
      but_c_state = false;
    else if (val < TH_CUTOFF)
      but_c_state = true; 
  }
  else
  {
    if (digitalRead(BUT_A) == LOW)
      but_a_state = true; 
    else
      but_a_state = false; 

    if (digitalRead(BUT_B) == LOW)
      but_b_state = true; 
    else
      but_b_state = false; 

    if (digitalRead(BUT_C) == LOW)
      but_c_state = true; 
    else
      but_c_state = false;           
  }
}

bool checkForTouchpad(void)
{
  return hasTouchpads;
}

void loop() 
{
  static uint16_t sampleCount = 0;
#ifdef WIFI
  //wm.process();
  dnsServer.processNextRequest();
#endif

  //Deal with incoming data
  incoming_protocol_request();

  //Handle Motion Data
  if (mpu_update() == false) 
  {
    //MPU ERROR
    //Not sure what to do
  }

  //Handle Tension Strips
  EVERY_N_MILLIS(16)  //60 times/sec    
  {
    glove_update();
  }

  sampleCount++;

  EVERY_N_MILLIS(33)  //30 times/sec
  {
    //Handle Buttons
    handle_buttons();
    //See If we have to deal with state-changes
    process_state();

    sampleCount = 0;
  }
  
  //Heartbeat
  EVERY_N_SECONDS(1)
  {
    toggle_status_led();
  }
}

