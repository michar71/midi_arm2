#include "baboi_webserver.h"
#include "baboi_sensors.h"
#include <AsyncElegantOTA.h>


extern setup_t settings;
extern String devicename;
extern int maj_ver;
extern int min_ver;

AsyncWebServer server(80);

CaptiveRequestHandler::CaptiveRequestHandler() 
{    
}

CaptiveRequestHandler::~CaptiveRequestHandler() 
{
}

bool CaptiveRequestHandler::canHandle(AsyncWebServerRequest *request){
//request->addInterestingHeader("ANY");
return true;
}

void CaptiveRequestHandler::handleRequest(AsyncWebServerRequest *request) 
{
AsyncResponseStream *response = request->beginResponseStream("text/html");
response->addHeader("Server","ESP BABOI Setup Page");
response->printf("<!DOCTYPE html><html><head><title>BABOI Main Page</title><style>");
response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
response->printf("</style></head><body>");  
response->printf("<h1>BABOI FW V%d.%d Webserver running.</h1><br><p>",maj_ver,min_ver);
response->print("This is our captive portal front page.");
response->printf("You were trying to reach: http://%s%s", request->host().c_str(), request->url().c_str());
response->printf("Try opening <a href='http://%s'>this link</a> instead<br><br>", WiFi.softAPIP().toString().c_str());

response->print("<a href=\"baboi.local/update\">Update</a><br><br>");
response->print("<a href=\"baboi.local/settings\">Settings</a><br><br>");
response->print("<a href=\"baboi.local/reset\">Reset Settings</a><br><br>");
response->print("<a href=\"baboi.local/calgyroacc\">Calibrate Gyro/Accelerometer</a><br><br>");    
response->print("<a href=\"baboi.local/calmag\">Calibrate Magnetometer</a><br><br>");       
response->print("<a href=\"baboi.local/calbuttons\">Calibrate Buttons</a><br><br>");     
response->print("<a href=\"baboi.local/caltension\">Calibrate Glove</a><br><br>");      
response->print("<a href=\"./wmen\">Enable Wifi Manager</a><br><br>");       
response->print("<a href=\"./wmdis\">Disable Wifi Manager</a><br><br>");    
response->printf("</p>");    
response->print("</body></html>");
request->send(response);
}



void notFound(AsyncWebServerRequest *request) {
    request->send(404, "text/plain", "Page Not found");
}

void setup_settings_webpage()
{
    server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Settings Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Settings</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->print("<h1>Settings</h1><br><p>");
          
    response->printf("Acc Bias X: %f <br>",settings.main_acc_bias_x);
    response->printf("Acc Bias Y: %f <br>",settings.main_acc_bias_y);
    response->printf("Acc Bias Z: %f <br>",settings.main_acc_bias_z);

    response->printf("Gyro Bias X: %f <br>",settings.main_gyro_bias_x);
    response->printf("Gyro Bias Y: %f <br>",settings.main_gyro_bias_y);
    response->printf("Gyro Bias Z: %f <br>",settings.main_gyro_bias_z);   

    response->printf("Offset Pitch: %f <br>",settings.offset_pitch);  
    response->printf("Offset Roll: %f <br>",settings.offset_roll);  
    response->printf("Offset Yaw: %f <br>",settings.offset_yaw);  

    response->printf("Mag Bias X: %f <br>",settings.main_mag_bias_x);    
    response->printf("Mag Bias Y: %f <br>",settings.main_mag_bias_y);    
    response->printf("Mag Bias Z: %f <br>",settings.main_mag_bias_z);    

    response->printf("Mag Scale X: %f <br>",settings.main_mag_scale_x);   
    response->printf("Mag Scale Y: %f <br>",settings.main_mag_scale_y);   
    response->printf("Mag Scale Z: %f <br>",settings.main_mag_scale_z);   

    response->printf("Touch TH CTRL: %d <br>",settings.th_but_ctrl); 
    response->printf("Touch TH A: %d <br>",settings.th_but_a); 
    response->printf("Touch TH B: %d <br>",settings.th_but_b);    

    response->printf("Tension Ch1 Min: %d <br>",settings.tension_ch1_min); 
    response->printf("Tension Ch1 Max: %d <br>",settings.tension_ch1_max); 
    response->printf("Tension Ch2 Min: %d <br>",settings.tension_ch2_min); 
    response->printf("Tension Ch2 Max: %d <br>",settings.tension_ch2_max);         
    response->printf("</p>");  
    response->print("<h1>Debug Data</h1><br><p>");
    response->printf("Free Heap: %d <br>",ESP.getFreeHeap());     
    response->printf("BUTTON CTRL VAL: %d <br>",touchRead(BUT_CTRL));  
    response->printf("BUTTON A VAL: %d <br>",touchRead(BUT_A));  
    response->printf("BUTTON B VAL: %d <br>",touchRead(BUT_B));  
    response->printf("TENSION CH1 VAL: %d <br>",analogRead(ANALOG_CH1));      
    response->printf("TENSION CH2 VAL: %d <br>",analogRead(ANALOG_CH2));      
    response->printf("Current Pitch: %f <br>",mpu_GetCurrentPitch());  
    response->printf("Current Roll: %f <br>",mpu_GetCurrentRoll());  
    response->printf("Current Yaw: %f <br>",mpu_GetCurrentYaw());      
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);
  });
}

void setup_main_webpage()
{
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Main Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Main Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>BABOI FW V%d.%d Webserver running.</h1><br><p>",maj_ver,min_ver);
    response->print("<a href=\"./update\">Update</a><br><br>");
    response->print("<a href=\"./settings\">Settings</a><br><br>");
    response->print("<a href=\"./reset\">Reset Settings</a><br><br>");
    response->print("<a href=\"./calgyroacc\">Calibrate Gyro/Accelerometer</a><br><br>");    
    response->print("<a href=\"./calmag\">Calibrate Magnetometer</a><br><br>");       
    response->print("<a href=\"./calbuttons\">Calibrate Buttons</a><br><br>");     
    response->print("<a href=\"./caltension\">Calibrate Glove</a><br><br>");          
    response->print("<a href=\"./wmen\">Enable Wifi Manager</a><br><br>");       
    response->print("<a href=\"./wmdis\">Disable Wifi Manager</a><br><br>");         
    response->printf("</p>");    
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
      init_settings_other();
      save_settings();
      setLED(1,64,64,64);
      delay(250);
      setLED(0,0,0,0);
    });
}

void setup_wm_webpage()
{
  server.on("/wmen", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Wifi Manager Enabled. Restarting...");
    settings.autoConnect = true;
    save_settings();
    delay(1000);
    ESP.restart();
  });
  server.on("/wmdis", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Wifi Manager Disabled. Restarting");
    settings.autoConnect = false;
    save_settings();
    delay(1000);
    ESP.restart();
  });  
}

void setup_cal_gyro_acc_webpage()
{
  server.on("/calgyroacc", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Calibrating Gyro/Accelerometer. But BABOI on flat surface and don't move during calibration.");
    delay(1000);
    setState(STATE_CAL_GYRO);
  });
}

void setup_cal_mag_webpage()
{
  server.on("/calmag", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Calibrating Magnetometer. Move BABOI in a figure-8.");
    delay(1000);
    setState(STATE_CAL_MAG);   
  });
}

void setup_cal_buttons_webpage()
{
  server.on("/calbuttons", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Calibrating Buttons. Touch all 3 buttons repeatatly.");
    setState(STATE_CAL_BUTTONS); 
  });
}

void setup_cal_tension_webpage()
{
  server.on("/caltension", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/plain", "Calibrating Glove. Bend Fingers repeatedly.");
    setState(STATE_CAL_TENSION); 
  });
}

void setup_captive_webpage()
{
  server.addHandler(new CaptiveRequestHandler()).setFilter(ON_AP_FILTER);//only when requested from AP
}

void init_webserver()
{
    //setup_captive_webpage();
    server.onNotFound(notFound);
    setup_reset_webpage();
    setup_settings_webpage();
    setup_main_webpage();
    setup_cal_gyro_acc_webpage();
    setup_cal_mag_webpage();
    setup_cal_buttons_webpage();
    setup_cal_tension_webpage();
    setup_wm_webpage();

    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    server.begin();
    Serial.println("HTTP server started");    
}
