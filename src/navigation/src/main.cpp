#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

#define LED_PIN    LED_BUILTIN

const char* ssid = "ESP8266_AP";
const char* password = "12345678";

ESP8266WebServer server(80);
String ledStatus = "OFF";
String lastLine = "";

void handleRoot() {
  String html = "<html><head><title>LED Status</title></head><body>";
  html += "<h1>LED Status: ";
  html += ledStatus;
  html += "</h1></body></html>";
  server.send(200, "text/html", html);
}

void setup() {
  digitalWrite(LED_PIN, LOW);  // LED on (inverted logic)

  Serial.begin(9600);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.on("/", handleRoot);
  server.begin();
}

void loop() {
  server.handleClient();
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      if (lastLine.startsWith("LED:")) {
        String tempStatus = lastLine.substring(4);
        tempStatus.trim();
        ledStatus = tempStatus;
      }
      lastLine = "";
    } else if (c != '\r') {
      lastLine += c;
    }
  }
}
