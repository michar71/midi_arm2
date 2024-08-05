#ifndef baboi_webserver_h
#define baboi_webserver_h

#include "arduino.h"
#include "main.h"
#include "settings.h"


#include <ESPAsyncWebServer.h>



class CaptiveRequestHandler : public AsyncWebHandler {
public:
  CaptiveRequestHandler();
  virtual ~CaptiveRequestHandler();

  bool canHandle(AsyncWebServerRequest *request);

  void handleRequest(AsyncWebServerRequest *request);
};

void init_webserver(void);


#endif
