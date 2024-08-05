#include "baboi_webserver.h"
#include "baboi_sensors.h"
#include "baboi_led.h"
#include <AsyncElegantOTA.h>
#include "main.h"


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
  response->printf("<h1>BABOI FW V%d.%d Webserver</h1><br><p>",maj_ver,min_ver);
  response->print("This is our captive portal front page.");
  response->printf("You were trying to reach: http://%s%s", request->host().c_str(), request->url().c_str());
  response->printf("Try opening <a href='http://%s'>this link</a> instead<br><br>", WiFi.softAPIP().toString().c_str());

  response->print("<a href=\"./update\">Update</a><br><br>");
  response->print("<a href=\"./settings\">Settings</a><br><br>");
  response->print("<a href=\"./setup\">Setup</a><br><br>");
  response->print("<a href=\"./reset_q\">Reset Settings</a><br><br>");
  response->print("<a href=\"./cal_q\">Calibration</a><br><br>");    
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
  server.on("/settings", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Settings Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Settings</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->print("<h1>Settings</h1><br><p>");
    response->printf("Has Glove: %d <br>",checkForGlove());
    response->printf("Has Gyro: %d <br>",checkForGyro());
          
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

    response->printf("Tension Ch1 Min: %d <br>",settings.tension_min[0]); 
    response->printf("Tension Ch1 Max: %d <br>",settings.tension_max[0]); 
    response->printf("Tension Ch2 Min: %d <br>",settings.tension_min[1]); 
    response->printf("Tension Ch2 Max: %d <br>",settings.tension_max[1]); 
    response->printf("TENSION CH1 OUT: %d <br>",tension_get_ch(0));      
    response->printf("TENSION CH2 OUT: %d <br>",tension_get_ch(1));    
    response->printf("</p>");  
    response->print("<h1>Debug Data</h1><br><p>");
    response->printf("Free Heap: %d <br>",ESP.getFreeHeap());   

    response->printf("BUTTON CTRL VAL: %d <br>",digitalRead(BUT_CTRL));  
    response->printf("BUTTON A VAL: %d <br>",digitalRead(BUT_A));   

    response->printf("TENSION CH1 VAL: %d <br>",adc_get_data(0));      
    response->printf("TENSION CH2 VAL: %d <br>",adc_get_data(1));             
    response->printf("Current Pitch: %f <br>",mpu_GetCurrentPitch());  
    response->printf("Current Roll: %f <br>",mpu_GetCurrentRoll());  
    response->printf("Current Yaw: %f <br>",mpu_GetCurrentYaw());      
    response->print("<a href=\"./\">Back</a><br><br>");       
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);
  });
}



void setup_main_webpage()
{
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Main Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Main Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>BABOI FW V%d.%d Webserver</h1><br><p>",maj_ver,min_ver);
    response->print("<a href=\"./update\">Update</a><br><br>");
    response->print("<a href=\"./settings\">Settings</a><br><br>");
    response->print("<a href=\"./reset_q\">Reset Settings</a><br><br>");
    response->print("<a href=\"./setup\">Setup</a><br><br>");    
    response->print("<a href=\"./cal_q\">Calibration</a><br><br>");           
    response->print("<a href=\"./wmen\">Enable Wifi Manager</a><br><br>");       
    response->print("<a href=\"./wmdis\">Disable Wifi Manager</a><br><br>");         
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);
  });
}

void setup_reset_question_webpage()
{
  server.on("/reset_q", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Reset Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Reset Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>Reset All Settings</h1><br><p>");
    response->printf("Are you sure you want to reset all Settings?<br>");
    response->print("<a href=\"./reset\">Yes, Continue</a><br><br>");
    response->print("<a href=\"./\">Cancel</a><br><br>");       
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);
  });
}

void setup_reset_webpage()
{

   server.on("/reset", HTTP_GET, [](AsyncWebServerRequest *request)
   {
    init_settings_acc_gyro();
    init_settings_mag();
    init_settings_other();
    save_settings();
    setLED(1,64,64,64);
    delay(250);
    setLED(0,0,0,0);

    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Reset Done Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Reset Done Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>Reset Done</h1><br><p>");
    response->printf("All Settings have been reset to default. Cycle power to restart.<br>");
    response->print("<a href=\"./\">Back</a><br><br>");       
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);      
  });
}

void setup_setup_webpage()
{
   server.on("/setup", HTTP_GET, [](AsyncWebServerRequest *request)
   {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Setup Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Setup Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>Setup</h1><br><p>");
    response->printf("<form action=\"/setup\" method=\"POST\">");
    response->printf("<label for=\"ID\">BABOI ID (8 Characters):</label>");   
    response->printf("<input type=\"text\" name=\"ID\" maxlength=\"8\" size=\"8\" value=\"%s\" <br>",settings.ID);   


    response->printf("<br><br>Position:<br>");

    if (settings.pos = 0)
    {
      response->printf("<input type=\"radio\" name=\"pos\" value=\"Left\"/>");
      response->printf("<label for=\"Left\">Left</label/>");
      response->printf("<input type=\"radio\" name=\"pos\" value=\"Right\" checked />");
      response->printf("<label for=\"Right\">Right</label><br><br>");
    }
    else
    {
      response->printf("<input type=\"radio\" name=\"pos\" value=\"Left\" checked />");
      response->printf("<label for=\"Left\">Left</label>");
      response->printf("<input type=\"radio\" name=\"pos\" value=\"Right\" />");
      response->printf("<label for=\"Right\">Right</label><br><br>");
    }
    response->printf("<input type=\"submit\" value=\"SAVE\">");   
    response->printf("</form>");    
    response->print("<br><a href=\"./\">Back</a><br><br>");       
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);      
  });
}



void setup_handle_setup()
{
 server.on("/setup", HTTP_POST, [](AsyncWebServerRequest *request) 
 {
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter* p = request->getParam(i);
      if(p->isPost()){
        // HTTP POST input1 value (direction)
        if (p->name() == "ID") 
        {
          snprintf(settings.ID,8,p->value().c_str());
        }
        if (p->name() == "pos") {
          if(p->value().c_str() == "Left")
          {
            settings.pos = 1;
          }
          else 
          {
            settings.pos = 0;
          }
        }        
      }
    }
    save_settings();
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


void setup_cal_question_webpage()
{
  server.on("/cal_q", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Valibration Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Calibragtion Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>BABOI Calibration</h1><br><p>");
    response->printf("Calibrate the Gyro and Accelerometer by keeping your arm on a flat surface during calibration.<br>");
    response->print("<a href=\"./calgyroacc\">Calibrate Gyro/Accelerometer</a><br><br>");    
    response->printf("Calibrate the magnetometer by moving the baboi through all 3 axises in a figure-eight.<br>");
    response->print("<a href=\"./calmag\">Calibrate Magnetometer</a><br><br>");
    
    if (checkForGlove())
    {
      response->printf("Calibrate the Calibrate thew glove by repeatly flexing and stretching your fingers.<br>");
      response->print("<a href=\"./caltension\">Calibrate Glove</a><br><br>");          
    }
    response->print("<br><a href=\"./\">Cancel</a><br><br>");   
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);
  });
}

void setup_cal_gyro_acc_webpage()
{
  server.on("/calgyroacc", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    setState(STATE_CAL_GYRO);

    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Calibrate Gyro/Acc Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Gyro/Acc Calibration Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>Gyro/Accelerometer calibration in Progress</h1><br><p>");
    response->printf("Keep your arm on the flat surface until the calibration light turns off.<br>");
    response->print("<a href=\"./\">Back</a><br><br>");       
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);      
  });
}

void setup_cal_mag_webpage()
{
  server.on("/calmag", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    setState(STATE_CAL_MAG);   
  
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Calibrate Magnetometer Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Magnetometer Calibration Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>Magnetometer calibration in Progress</h1><br><p>");
    response->printf("Continue moving the BABOLI in a figure-eight motion through all three axises until the calibration light turns off.<br>");
    response->print("<a href=\"./\">Back</a><br><br>");       
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);        
  });
}


void setup_cal_tension_webpage()
{
  server.on("/caltension", HTTP_GET, [](AsyncWebServerRequest *request)
  {
    setState(STATE_CAL_TENSION); 
  
    AsyncResponseStream *response = request->beginResponseStream("text/html");
    response->addHeader("Server","ESP BABOI Calibrate Glove Page");
    response->printf("<!DOCTYPE html><html><head><title>BABOI Glove Calibration Page</title><style>");
    response->printf("h1 {color: maroon;margin-left: 40px;font-size: 40px;font-family: Arial,Helvetica, sans-serif;} ");
    response->printf("p {font-size: 22px;font-family: Arial, Helvetica, sans-serif;}"); 
    response->printf("</style></head><body>");  
    response->printf("<h1>Glove calibration in Progress</h1><br><p>");
    response->printf("Continue flexing and stretching your fingers until the calibration light turns off.<br>");
    response->print("<a href=\"./\">Back</a><br><br>");       
    response->printf("</p>");    
    response->print("</body></html>");
    request->send(response);     
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
    setup_reset_question_webpage();
    setup_reset_webpage();
    setup_settings_webpage();
    setup_main_webpage();
    setup_cal_question_webpage();
    setup_cal_gyro_acc_webpage();
    setup_cal_mag_webpage();
    setup_cal_tension_webpage();
    setup_wm_webpage();
    setup_setup_webpage();
    setup_handle_setup();

    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    server.begin();
    Serial.println("HTTP server started");    
}
