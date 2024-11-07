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


#define CTRL_LED_BRIGHTNESS 16
const char* devicename = "BABOI";
int maj_ver = 3;
int min_ver = 5;

extern setup_t settings;

bool but_a_state = false;



ButtonClass but_ctrl(BUT_CTRL,false,false);
ButtonClass but_a(BUT_A,false,false);


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
    Serial.begin(460800);	
    setCpuFrequencyMhz(160);

    
  #ifdef DEBUG
   
   //delay(5000);
    Serial.setDebugOutput(true);
    //for (int ii=0;ii<100;ii++)
    //{
    //  Serial.println("Startup...");
    //  delay(100);
    //}
    
  #endif
    pinMode(STATUS_LED, OUTPUT);     
        
    
  #ifdef DEBUG
    Serial.println("Pin Setup Done...");
  #endif

    settings_init();
    led_init();
    led_test();
#ifdef DEBUG
  Serial.println("Sensor Calib Load Done...");
#endif   

  #ifdef DEBUG
    Serial.println("Using Hard Buttons");
  #endif          
    but_ctrl.setTouchMode(false);
    pinMode(BUT_CTRL,INPUT_PULLUP);
    pinMode(BUT_A,INPUT_PULLUP);
    delay(1);
    //Reset everything if all buttons are down
    if (digitalRead(BUT_CTRL) == LOW)
    {
      init_settings_acc_gyro();
      init_settings_mag();
      init_settings_other();
      save_settings();
      setLED(1,CTRL_LED_BRIGHTNESS,CTRL_LED_BRIGHTNESS,CTRL_LED_BRIGHTNESS,true);
      delay(500);
      setLED(0,0,0,0,true);
  #ifdef DEBUG
        Serial.println("Full Reset Done");
  #endif           
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
    if (checkForMCU())
    {
      mpu_set_settings();
    }
  }
  else
  {
    mpu_init_settings();
    init_settings_other();
    save_settings();
    setLED(0,CTRL_LED_BRIGHTNESS,0,0,true);
    delay(400);
    setLED(0,0,0,0,true);    

#ifdef DEBUG
  Serial.println("Sensor Calib Load Failed, resetting settings to defaults.");
  print_settings();
#endif
  }


    
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



  #ifdef DEBUG
    Serial.println("Setup Done...");
  #endif    

  setState(STATE_WAITCONN);
}


void process_state(void)
{
  setLED(0,0,0,0);
  switch(state)
  {
    case STATE_STARTUP:
    break;

    case STATE_WAITCONN:
      setLED(0,255,255,0);
    break;

    case STATE_LIVE:
      send_processing_data(true);
      setLED(1,0,255,0);
    break;

    case STATE_PAUSED:

      send_processing_data(false);
      setLED(1,255,0,0);
      for(int ii=2;ii<NUM_LEDS;ii++)
      {
        setLED(ii,0,0,0);
      } 
    break;

    case STATE_CAL_BUTTONS:
      setLED(1,255,0,255,true);   
      save_settings();    
    #ifdef DEBUG    
      Serial.println("Button Calib Done...");
      print_settings();
      #endif   
      setState(lastState);
      setLED(1,0,0,0,true);
    break;

    case STATE_CAL_TENSION:
      setLED(1,255,255,255,true);
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
  else if (button_res == VERY_VERY_LONG_PRESS)
  {
    setState(STATE_CAL_MAG);
  }


    if (digitalRead(BUT_A) == LOW)
      but_a_state = true; 
    else
      but_a_state = false;       

}


void loop() 
{
  static uint16_t sampleCount = 0;

  dnsServer.processNextRequest();

  //Deal with incoming data
  incoming_protocol_request();
  //Handle Motion Data
  if (checkForMCU())
  {
    if (mpu_update() == false) 
    {
      //Gyro not avasilable yet... Just ignore...
    }
  }
  else
  {
    Serial.println("No MCU");
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
   
    //Check if we have a comm timeout
    if (checkCommTimeout())
    {
      setLED(0,255,0,0);
      for(int ii=2;ii<NUM_LEDS;ii++)
      {
        setLED(ii,0,0,0);
      } 
    }
    else
    {
      setLED(0,0,255,0);  
    }

    updateLED();   
  }
}

