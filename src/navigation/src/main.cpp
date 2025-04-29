#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include "webserver.h"
#include "distance_sensor.h"

#define LED_PIN    LED_BUILTIN

#define TRIG1 D1
#define ECHO1 D2
#define TRIG2 D5
#define ECHO2 D6
#define S0  D3
#define S1  D4
#define S2  D7
#define S3  D8
#define OUT D0


unsigned int readColorChannel(byte s2State, byte s3State) {
    digitalWrite(S2, s2State);
    digitalWrite(S3, s3State);
    delay(10); // Allow sensor to settle
    return pulseIn(OUT, LOW, 25000); // 25ms timeout for ESP8266
}


AsyncWebServer server(80);
AsyncEventSource events("/events");

static bool ledState = false; // LED state
static bool mainBooted = false; // Main board boot status
static bool navigationBooted = false; // Navigation board boot status
static float lastDistance = -1.0; // Last received distance 1
static float lastDistance2 = -1.0; // Last received distance 2
static String lastColor = ""; // Last received color
unsigned long lastReportTime = 0;

static DistanceSensor sensor1(TRIG1, ECHO1);
static DistanceSensor sensor2(TRIG2, ECHO2);

void setupColorSensor() {
    pinMode(S0, OUTPUT);
    pinMode(S1, OUTPUT);
    pinMode(S2, OUTPUT);
    pinMode(S3, OUTPUT);
    pinMode(OUT, INPUT);

    digitalWrite(S0, HIGH);
    digitalWrite(S1, LOW);
}

// Helper to measure frequency on OUT pin for a given filter
String readColor() {
    unsigned int red = readColorChannel(LOW, LOW);
    unsigned int green = readColorChannel(HIGH, HIGH);
    unsigned int blue = readColorChannel(LOW, HIGH);
    return String(red) + "," + String(green) + "," + String(blue);
}

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // LED on (inverted logic)

    Serial.begin(115200); // UART for button status
    Serial.println("Setup complete. Waiting for button status via UART.");

    // Mark navigation board as booted and turn on LED
    navigationBooted = true;
    digitalWrite(LED_PIN, LOW); // LED on

    sensor1.begin();
    sensor2.begin();
    setupColorSensor();

    setupWebServer(server, events, ledState, mainBooted, navigationBooted, lastDistance, lastDistance2, lastColor);
    server.begin();
}

void loop() {
    // Read both ultrasonic sensors
    float distance1 = sensor1.read();
    float distance2 = sensor2.read();
    lastDistance = (distance1 > 0) ? distance1 : -1.0;
    lastDistance2 = (distance2 > 0) ? distance2 : -1.0;

    // Read color sensor (placeholder)
    lastColor = readColor();

    // Output as CSV: BOOTED,LED_STATE,DISTANCE1,DISTANCE2,COLOR
    // BOOTED: 1 if navigation board booted, 0 otherwise
    // LED_STATE: 1 if LED is ON, 0 if OFF
    // DISTANCE1, DISTANCE2: floating-point in cm or 'timeout'
    // COLOR: R,G,B as string
    int booted = navigationBooted ? 1 : 0;
    int led = ledState ? 1 : 0;
    String dist1Str = (lastDistance >= 0) ? String(lastDistance, 2) : "timeout";
    String dist2Str = (lastDistance2 >= 0) ? String(lastDistance2, 2) : "timeout";
    Serial.print(booted);
    Serial.print(",");
    Serial.print(led);
    Serial.print(",");
    Serial.print(dist1Str);
    Serial.print(",");
    Serial.print(dist2Str);
    Serial.print(",");
    Serial.println(lastColor);

    // Trigger SSE updates
    handleUartAndSSE(events, mainBooted, ledState, lastDistance, lastDistance2, lastColor);
    delay(100); // Small delay for loop stability
}