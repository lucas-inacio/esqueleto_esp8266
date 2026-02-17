#include "webserver.hpp"

#include <Arduino.h>
#include <FS.h>
#include <LittleFS.h>

#include "common.hpp"
#include "config.hpp"
#include "log.hpp"

#include "site_data.h"

AsyncWebServer server(80);

bool handleUpdate(AsyncWebServerRequest *request) {
    bool shouldRestart = false;
    for (const auto& it : mainConfig) {
        if (request->hasParam(it.first, true)) {
            String value = request->getParam(it.first, true)->value();
            Serial.println(it.first + " = " + value);
            if (value.length() > 0) {
                shouldRestart = true;
                mainConfig[it.first] = value;
            }
        }
    }

    return shouldRestart;
}

void sendGzipFile(AsyncWebServerRequest *request)
{
    String query = request->url();
    AsyncWebServerResponse *response = nullptr;
    String type{"text/html"};
    if (query.indexOf('.') >= 0) {
        if (query.indexOf(".css") >= 0) type = "text/css";
        else if (query.indexOf(".js") >= 0) type = "text/javascript";
        query += ".gz";
        response = request->beginResponse(SPIFFS, query, type);
    } else {
        response = request->beginResponse(SPIFFS, "/index.html.gz", type);
    }
    response->addHeader("Content-Encoding", "gzip");
    request->send(response);
}

void sendGzipProgmem(AsyncWebServerRequest *request)
{
    String query = request->url();
    String type{"text/html"};
    if (query.indexOf('.') >= 0) {
        if (query.indexOf(".css") >= 0) type = "text/css";
        else if (query.indexOf(".js") >= 0) type = "text/javascript";
    } else {
        query = "/index.html";
    }

    size_t i = 0;
    for (; i < gzipDataCount; ++i)
    {
        if (strcmp(query.c_str(), gzipDataMap[i].path) == 0)
        {
            AsyncWebServerResponse *response = request->beginResponse_P(
                200, type, gzipDataMap[i].data, gzipDataMap[i].dataSize);
            response->addHeader("Content-Encoding", "gzip");
            request->send(response);
            break;
        }
    }

    if (i == gzipDataCount)
    {
        request->send(404);
    }
}

void setupWebserver() {
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(mainConfig[KEY_ADMIN_LOGIN].c_str(), mainConfig[KEY_ADMIN_PASS].c_str()))
            return request->requestAuthentication();
        aplicaConfig = handleUpdate(request);
        if (aplicaConfig) request->send(200, "text/plain", "OK");
        else request->send(400, "text/plain", "Bad request");
    });

    server.on("/config", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(mainConfig[KEY_ADMIN_LOGIN].c_str(), mainConfig[KEY_ADMIN_PASS].c_str()))
            return request->requestAuthentication();
        if(LittleFS.exists(CONFIG_FILE)) {
            AsyncWebServerResponse *response = request->beginResponse(LittleFS, CONFIG_FILE, "text/plain");
            request->send(response);
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });

    /*server.on("/log", HTTP_GET, [](AsyncWebServerRequest *request) {
        if (!request->authenticate(mainConfig[KEY_ADMIN_LOGIN].c_str(), mainConfig[KEY_ADMIN_PASS].c_str()))
            return request->requestAuthentication();

        if(SPIFFS.exists(LOG_FILE)) {
            AsyncWebServerResponse *response = request->beginResponse(SPIFFS, LOG_FILE, "text/plain");
            request->send(response);
        } else {
            request->send(404, "text/plain", "Not found");
        }
    });*/

    server.onNotFound([](AsyncWebServerRequest *request) {
        Serial.println(request->url());
        if (!request->authenticate(mainConfig[KEY_ADMIN_LOGIN].c_str(), mainConfig[KEY_ADMIN_PASS].c_str()))
            return request->requestAuthentication();
        
        // sendGzipFile(request);
        sendGzipProgmem(request);
    });
    server.begin();
}
