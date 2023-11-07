#include "main.h"
#include "ButtonClass.h"
#include "settings.h"
#include "baboi_protocol.h"
#include "baboi_sensors.h"

#ifdef WIFI
  //#include <WiFiManager.h> // https://github.com/tzapu/WiFiManager
  #include <DNSServer.h>
  #include <WiFi.h>
  #include <AsyncTCP.h>
  #include "baboi_webserver.h"

  //WiFiManager wm;
  DNSServer dnsServer;
#endif

String devicename = "BABOI";
int maj_ver = 1;
int min_ver = 1;

extern setup_t settings;


int led_state = LOW;    // the current state of LED
bool but_a_state = false;
bool but_b_state = false;
bool but_c_state = false;

// Define the array of leds
CRGB leds[NUM_LEDS];

ButtonClass but_ctrl(BUT_CTRL,true);

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


void toggle_led(void)
{
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

    init_sensors();

  #ifdef WIFI
    
    
    /*
    WiFi.mode(WIFI_STA); // explicitly set mode, esp defaults to STA+AP    
    wm.setConfigPortalBlocking(false);
    wm.setConfigPortalTimeout(60;)
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

    WiFi.softAP(ssid,NULL,7);
    IPAddress IP = WiFi.softAPIP();
    delay(100);
    Serial.println("Setting the AP");
    IPAddress Ip(192, 168, 1, 1);    //set to IP Access Point same as gateway
    IPAddress NMask(255, 255, 255, 0);
    WiFi.softAPConfig(Ip, Ip, NMask);

    dnsServer.start(53, "*", WiFi.softAPIP()); 

    Serial.print("AP IP address: ");
    Serial.println(IP);

    init_webserver();
    init_protocol();

  #endif

  if (load_settings())
  {
    mpu_set_settings();

    but_ctrl.setTouchThreshold(settings.th_but_ctrl,(uint16_t)TH_CUTOFF);
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
    mpu_init_settings();
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
 

//We need to illimate high outliers...

void calibrate_buttons()
{
  uint16_t min[3] = {0xFFFF,0xFFFF,0xFFFF};
  uint16_t max[3] = {0,0,0};
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
    val = touchRead(BUT_CTRL);
    if (val<TH_CUTOFF)
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
  settings.th_but_a = min[0] + ((max[0] - min[0])/3*2);
  settings.th_but_b = min[1] + ((max[1] - min[1])/3*2);
  settings.th_but_ctrl = min[2] + ((max[2] - min[2])/3*2);    
  but_ctrl.setTouchThreshold(settings.th_but_ctrl,TH_CUTOFF);
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
    tension_update();

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
    toggle_led();
  }
}

