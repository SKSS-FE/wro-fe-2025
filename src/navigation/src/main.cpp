#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include "webserver.h"

#define LED_PIN    LED_BUILTIN

AsyncWebServer server(80);
AsyncEventSource events("/events");

static bool ledState = false; // LED state
static bool mainBooted = false; // Main board boot status
static bool navigationBooted = false; // Navigation board boot status
static float lastDistance = -1.0; // Last received distance
unsigned long lastReportTime = 0;

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // LED on (inverted logic)

    Serial.begin(115200); // UART for button status
    Serial.println("Setup complete. Waiting for button status via UART.");

    // Mark navigation board as booted and turn on LED
    navigationBooted = true;
    digitalWrite(LED_PIN, LOW); // LED on

    setupWebServer(server, events, ledState, mainBooted, navigationBooted, lastDistance);
    server.begin();
}

void loop() {
    handleUartAndSSE(events, mainBooted, ledState, lastDistance);
    delay(10); // Small delay for loop stability
}