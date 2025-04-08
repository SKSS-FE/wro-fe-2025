#pragma once
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>

void triggerSSE(AsyncEventSource &events, const String &type, const String &data);
void setupWebServer(AsyncWebServer &server, AsyncEventSource &events, bool &ledState, bool &mainBooted, bool &navigationBooted, float &lastDistance);
void handleUartAndSSE(AsyncEventSource &events, bool &mainBooted, bool &ledState, float &lastDistance);
