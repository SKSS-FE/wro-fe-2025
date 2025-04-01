#include <Arduino.h>
#include <ESPAsyncWebServer.h>
#include <ESP8266WiFi.h>
#include <vector>

#define LED_PIN    LED_BUILTIN

const char* ssid = "ESP8266_AP";
const char* password = "12345678"; // Minimum 8 chars for WPA2

AsyncWebServer server(80);
AsyncEventSource events("/events");

static bool ledState = false; // LED state
static bool mainBooted = false; // Main board boot status
static bool navigationBooted = false; // Navigation board boot status

void setup() {
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW); // LED on (inverted logic)

    Serial.begin(9600); // UART for button status
    Serial.println("Setup complete. Waiting for button status via UART.");

    // Mark navigation board as booted and turn on LED
    navigationBooted = true;
    digitalWrite(LED_PIN, LOW); // LED on

    // Start WiFi in AP mode
    WiFi.mode(WIFI_AP);
    bool result = WiFi.softAP(ssid, password);
    if (result) {
        Serial.println("Access Point started");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("AP start failed!");
    }

    // Web server root: show LED state and both board boot status
    server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
        String html = "<html><body>";
        html += "<h1>LED is <span id='ledstate'>" + String(ledState ? "ON" : "OFF") + "</span></h1>";
        html += "<h2>Navigation Board: <span id='navboot'>" + String(navigationBooted ? "BOOTED" : "NOT READY") + "</span></h2>";
        html += "<h2>Main Board: <span id='mainboot'>" + String(mainBooted ? "BOOTED" : "NOT READY") + "</span></h2>";
        html += "<script>\n";
        html += "var es = new EventSource('/events');\n";
        html += "es.addEventListener('led', function(e) { document.getElementById('ledstate').textContent = e.data; });\n";
        html += "es.addEventListener('mainboot', function(e) { document.getElementById('mainboot').textContent = e.data; });\n";
        html += "</script>";
        html += "</body></html>";
        request->send(200, "text/html", html);
    });
    // SSE endpoint
    server.addHandler(&events);
    server.begin();
}

void loop() {
    // Listen for UART input: LED status ('0' for ON, '1' for OFF) and main board boot
    static String uartBuffer = "";
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (uartBuffer.length() > 0) {
                // Check for boot message
                if (uartBuffer == "BOOT_OK") {
                    if (!mainBooted) {
                        mainBooted = true;
                        events.send("BOOTED", "mainboot");
                    }
                }
                // LED status
                else if (uartBuffer == "0" && !ledState) {
                    ledState = true;
                    digitalWrite(LED_PIN, LOW);
                    events.send("ON", "led");
                } else if (uartBuffer == "1" && ledState) {
                    ledState = false;
                    digitalWrite(LED_PIN, HIGH);
                    events.send("OFF", "led");
                }
                uartBuffer = "";
            }
        } else if (isPrintable(c)) {
            uartBuffer += c;
        }
    }
    delay(10); // Short delay
}