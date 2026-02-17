#ifndef WEBSERVER_HPP
#define WEBSERVER_HPP

#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>

extern AsyncWebServer server;

bool handleUpdate(AsyncWebServerRequest *request);

void sendGzipFile(AsyncWebServerRequest *request);

void sendGzipProgmem(AsyncWebServerRequest *request);

void setupWebserver();

#endif // WEBSERVER_HPP