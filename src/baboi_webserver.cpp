#include "baboi_webserver.h"
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
response->printf("<!DOCTYPE html><html><head><title>BABOI Main Page</title></head><body>");
response->printf("<h1>%s FW V%d.%d Webserver running.</h1><br>",devicename,maj_ver,min_ver);
response->print("<p>This is out captive portal front page.</p>");
response->printf("<p>You were trying to reach: http://%s%s</p>", request->host().c_str(), request->url().c_str());
response->printf("<p>Try opening <a href='http://%s'>this link</a> instead</p>", WiFi.softAPIP().toString().c_str());

response->print("<a href=\"http://192.168.1.1/update\">Update</a><br><br>");
response->print("<a href=\"http://192.168.1.1/settings\">Settings</a><br><br>");
response->print("<a href=\"http://192.168.1.1/reset\">Reset Settings</a><br><br>");
response->print("<a href=\"http://192.168.1.1/calgyroacc\">Calibrate Gyro/Accelerometer</a><br><br>");    
response->print("<a href=\"http://192.168.1.1/calmag\">Calibrate Magnetometer</a><br><br>");       
response->print("<a href=\"http://192.168.1.1/calbuttons\">Calibrate Buttons</a><br><br>");       
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
    response->print("<a href=\"http://192.168.1.1/update\">Update</a><br><br>");
    response->print("<a href=\"http://192.168.1.1/settings\">Settings</a><br><br>");
    response->print("<a href=\"http://192.168.1.1/reset\">Reset Settings</a><br><br>");
    response->print("<a href=\"http://192.168.1.1/calgyroacc\">Calibrate Gyro/Accelerometer</a><br><br>");    
    response->print("<a href=\"http://192.168.1.1/calmag\">Calibrate Magnetometer</a><br><br>");       
    response->print("<a href=\"http://192.168.1.1/calbuttons\">Calibrate Buttons</a><br><br>");         
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

    AsyncElegantOTA.begin(&server);    // Start AsyncElegantOTA
    server.begin();
    Serial.println("HTTP server started");    
}
