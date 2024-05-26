#include "main.h"
#include "ButtonClass.h"
#include "settings.h"
#include "baboi_protocol.h"
#include "baboi_sensors.h"
#include "baboi_led.h"

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
int maj_ver = 3;
int min_ver = 0;

extern setup_t settings;

bool but_a_state = false;
bool but_b_state = false;
bool but_c_state = false;

bool hasTouchpads = false;
bool touchpadsForStrips = false;   //Use Touchpads A/B/C in Analog Mode   as input for strips....

ButtonClass but_ctrl(BUT_CTRL,false,false);
ButtonClass but_a(BUT_A,false,false);
ButtonClass but_b(BUT_B,false,false);
ButtonClass but_c(BUT_C,false,false);

t_state state = STATE_STARTUP;
t_state lastState = STATE_STARTUP;


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
    #if BABOI_HW_VER == 2
      setCpuFrequencyMhz(240);
    #endif
    #if BABOI_HW_VER == 3
      setCpuFrequencyMhz(160);
    #endif
    
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
    else
    {
      hasTouchpads = false;
 #ifdef DEBUG      
      Serial.println("Buttons Enabled");
#endif
    }

    delay(10);
    settings_init();

    led_init();
    led_test();
#ifdef DEBUG
  Serial.println("Sensor Calib Load Done...");
#endif   

    if (checkForTouchpad())
    {
  #ifdef DEBUG
      Serial.println("Using Touch Pads");
  #endif          
      but_ctrl.setTouchMode(true);
      but_a.setTouchMode(true);
      but_b.setTouchMode(true);
      but_c.setTouchMode(true);            

      but_ctrl.setTouchThresholds(TOUCH_TH_CTRL-20000,TOUCH_TH_CTRL+20000);
      but_a.setTouchThresholds(TOUCH_TH_A-20000,TOUCH_TH_A+20000);
      but_b.setTouchThresholds(TOUCH_TH_B-20000,TOUCH_TH_B+20000);
      but_c.setTouchThresholds(TOUCH_TH_C-20000,TOUCH_TH_C+20000);


      //Reset everything if all buttons are pressed...
      if ((but_ctrl.check_button() == DOWN) && (but_a.check_button() == DOWN) && (but_b.check_button() == DOWN) && (but_c.check_button() == DOWN))
      {
        init_settings_acc_gyro();
        init_settings_mag();
        init_settings_but();
        init_settings_other();
        save_settings();
        setLED(1,64,64,64,true);
        delay(250);
        setLED(0,0,0,0,true);
  #ifdef DEBUG
        Serial.println("Full Reset Done");
  #endif        
      }    
    }
    else
    {
  #ifdef DEBUG
      Serial.println("Using Hard Buttons");
  #endif          
      but_ctrl.setTouchMode(false);
      pinMode(BUT_CTRL,INPUT_PULLUP);
      pinMode(BUT_A,INPUT_PULLUP);
      pinMode(BUT_B,INPUT_PULLUP);
      pinMode(BUT_C,INPUT_PULLUP);
      delay(1);
      //Reset everything if all buttons are down
      if ((digitalRead(BUT_A) == LOW) && (digitalRead(BUT_A) == LOW) && (digitalRead(BUT_A) == LOW) && (digitalRead(BUT_A) == LOW))
      {
        init_settings_acc_gyro();
        init_settings_mag();
        init_settings_but();
        init_settings_other();
        save_settings();
        setLED(1,64,64,64,true);
        delay(250);
        setLED(0,0,0,0,true);
  #ifdef DEBUG
        Serial.println("Full Reset Done");
  #endif           
      }      
    }

    //SPECIAL CASE TEST ONLY
    if (CheckTouchpadsForStrips())
    {
  #ifdef DEBUG
      Serial.println("Using Pressure-Sensitive Touchpads...");
  #endif       
      but_a.setTouchMode(true);
      but_b.setTouchMode(true);
      but_c.setTouchMode(true);            

      but_a.setTouchThresholds(0xffff,0);
      but_b.setTouchThresholds(0xffff,0);
      but_c.setTouchThresholds(0xffff,0);

      but_a.setAutocal(true);
      but_b.setAutocal(true);
      but_c.setAutocal(true);    
             
    }
    
  #ifdef DEBUG
    Serial.println("LED Setup Done...");
  #endif

    init_sensors();


  if (load_settings())
  {
  #ifdef DEBUG
    Serial.println("Settings Loaded");
  #endif    
    if (checkForGyro())
    {
      mpu_set_settings();
    }

    if (checkForTouchpad())
    {
      but_a.setTouchThresholds(settings.th_but_a_min,settings.th_but_a_max);
      but_b.setTouchThresholds(settings.th_but_b_min,settings.th_but_b_max);
      but_c.setTouchThresholds(settings.th_but_c_min,settings.th_but_c_max);        
      but_ctrl.setTouchThresholds(settings.th_but_ctrl_min,settings.th_but_ctrl_max);

      if ((settings.th_but_ctrl_max == 0) || (settings.th_but_a_max == 0) || (settings.th_but_b_max == 0)|| (settings.th_but_c_max == 0) || (settings.th_but_ctrl_min == 65535) || (settings.th_but_a_min == 65535) || (settings.th_but_b_min == 65535) || (settings.th_but_c_min == 65535))
      {
  #ifdef DEBUG
        Serial.println("Forced Touchpad Calibration!");
  #endif      
        setLED(1,255,0,255,true);
        calibrate_buttons();
        delay(1000);          
        save_settings();  
      }
    }
    setLED(0,0,64,0,true);
    delay(120);
    setLED(0,0,0,0,true);
  }
  else
  {
    mpu_init_settings();
    init_settings_but();
    init_settings_other();
    save_settings();
    setLED(0,64,0,0,true);
    delay(400);
    setLED(0,0,0,0,true);    

#ifdef DEBUG
  Serial.println("Sensor Calib Load Failed, resetting settings to defaults.");
  print_settings();
#endif
  }


  #ifdef WIFI
    
    //Hmmmm Conceptually not sure how WifiMansger would work....
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


    //Not configurede? Just create Access Point....
    if (!res)
    {
      Serial.print("Setting the AP:");
      Serial.println(devicename);
      IPAddress Ip(192, 168, 1, 1);    //set to IP Access Point same as gateway
      IPAddress NMask(255, 255, 255, 0);
      WiFi.softAPConfig(Ip, Ip, NMask);
      WiFi.softAP(devicename,NULL,7);
      delay(100);
      IPAddress IP = WiFi.softAPIP();   
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
 


//We need to eliminate high outliers...
void calibrate_buttons()
{
  uint16_t min[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};
  uint16_t max[4] = {0,0,0,0};
  uint16_t val = 0;

 #if BABOI_HW_VER == 2
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

  #endif
  //Take average of min/max
  //(Is this a good idea? Do we need to exclude outlieers?)
  settings.th_but_a_min = min[0];
  settings.th_but_a_max = max[0];
  settings.th_but_a_min = min[1];
  settings.th_but_a_max = max[1];
  settings.th_but_a_min = min[2];
  settings.th_but_a_max = max[2];
  settings.th_but_ctrl_min = min[3];    
  settings.th_but_ctrl_max = max[3];    
  but_a.setTouchThresholds(settings.th_but_a_min,settings.th_but_a_max);
  but_b.setTouchThresholds(settings.th_but_b_min,settings.th_but_b_max);
  but_c.setTouchThresholds(settings.th_but_c_min,settings.th_but_c_max);        
  but_ctrl.setTouchThresholds(settings.th_but_ctrl_min,settings.th_but_ctrl_max);
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
      setLED(1,255,0,255,true);
      init_settings_but();
      calibrate_buttons();        
      save_settings();    
    #ifdef DEBUG    
      Serial.println("Button Calib Done...");
      print_settings();
      #endif   
      setState(lastState);
      setLED(1,0,0,0,true);
    break;

    case STATE_CAL_TENSION:
      setLED(1,64,64,64,true);
      calibrate_tension();
      save_settings();    
    #ifdef DEBUG    
      Serial.println("Tension Calib Done...");
      print_settings();
      #endif   
      setState(lastState);
      setLED(1,0,0,0,true);
    break;

    case STATE_CAL_GYRO:
      setLED(1,0,0,255,true);
      delay(500);
      mpu_cal_gyro_accel();
      save_settings();
    #ifdef DEBUG    
      Serial.println("Sensor Calib Gyro/Acc Done...");
      print_settings();
    #endif  
      setState(lastState);
      setLED(1,0,0,0,true);
    break;

    case STATE_CAL_MAG:
      setLED(1,0,255,255,true);
      mpu_cal_mag();
      save_settings();
    #ifdef DEBUG    
      Serial.println("Sensor Calib Mag Done...");
      print_settings();
    #endif  
      setState(lastState);     
      setLED(1,0,0,0,true);     
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
      but_a_state = false;
      if (but_a.check_button() == DOWN)
        but_a_state = true; 

      but_b_state = false;
      if (but_b.check_button() == DOWN)
        but_b_state = true; 

      but_b_state = false;
      if (but_b.check_button() == DOWN)
        but_b_state = true; 
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


bool CheckTouchpadsForStrips(void)
{
  return touchpadsForStrips;
}

uint16_t getTouchAnalogValue(uint8_t pad)
{
  if (pad == 2)
    return but_c.getTouchAnalog(255);
  else if (pad == 1)
    return but_b.getTouchAnalog(255);
  else
    return but_a.getTouchAnalog(255);        
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
  if (checkForGyro())
  {
    if (mpu_update() == false) 
    {
      //MPU ERROR
      //Not sure what to do
    }
  }

  //Handle Tension Strips
  //EVERY_N_MILLIS(16)  //60 times/sec    
  if (checkForGlove())
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

    handle_led();
   
    //Check if we have a comm timeout
    if (checkCommTimeout())
      setLED(0,64,0,0);
    else
      setLED(0,0,64,0);  

    updateLED();
  }

  //Heartbeat
  EVERY_N_SECONDS(1)
  {
    toggle_status_led();
  }
}

