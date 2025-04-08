#include "webserver.h"
#include <Arduino.h>

// Define the pin number for the LED
#define LED_PIN    LED_BUILTIN

const char* ssid = "ESP8266_AP";
const char* password = "12345678"; // Minimum 8 chars for WPA2

void triggerSSE(AsyncEventSource &events, const String &type, const String &data) {
    events.send(data.c_str(), type.c_str());
}

void startWifiAP() {
    WiFi.mode(WIFI_AP);
    bool result = WiFi.softAP(ssid, password);
    if (result) {
        Serial.println("Access Point started");
        Serial.print("AP IP address: ");
        Serial.println(WiFi.softAPIP());
    } else {
        Serial.println("AP start failed!");
    }
}

void setupWebServer(AsyncWebServer &server, AsyncEventSource &events, bool &ledState, bool &mainBooted, bool &navigationBooted, float &lastDistance) {
    startWifiAP();
    server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request){
        String html = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Board Status</title>
    <style>
        body { font-family: 'Segoe UI', Arial, sans-serif; background: #f4f6fa; color: #222; margin: 0; padding: 0; }
        .container { max-width: 420px; margin: 40px auto; background: #fff; border-radius: 12px; box-shadow: 0 2px 12px #0001; padding: 32px 28px; }
        h1 { font-size: 2.1em; margin-bottom: 0.5em; }
        h2 { font-size: 1.2em; margin: 1.2em 0 0.5em 0; }
        .status { display: flex; align-items: center; gap: 0.7em; margin-bottom: 0.7em; }
        .led-indicator { width: 18px; height: 18px; border-radius: 50%; display: inline-block; border: 2px solid #bbb; }
        .led-on { background: #4caf50; border-color: #388e3c; }
        .led-off { background: #e53935; border-color: #b71c1c; }
        .chip { display: inline-block; padding: 0.2em 0.8em; border-radius: 1em; font-size: 1em; background: #e3e7ef; color: #333; margin-left: 0.5em; }
        .chip.booted { background: #c8e6c9; color: #256029; }
        .chip.notready { background: #ffcdd2; color: #b71c1c; }
        .distance { font-size: 1.5em; font-weight: 500; margin-top: 1.2em; }
        .footer { margin-top: 2em; color: #888; font-size: 0.95em; text-align: center; }
    </style>
</head>
<body>
    <div class='container'>
        <h1>Status</h1>
        <div class='status'>
            <span class='led-indicator' id='ledcircle'></span>
            <span>LED is <span id='ledstate'>)rawliteral";
        html += String(ledState ? "ON" : "OFF");
        html += R"rawliteral(</span></span>
        </div>
        <div class='status'>
            <span>Navigation Board:</span>
            <span id='navboot' class='chip )rawliteral";
        html += String(navigationBooted ? "booted" : "notready");
        html += R"rawliteral('>)rawliteral";
        html += String(navigationBooted ? "BOOTED" : "NOT READY");
        html += R"rawliteral(</span>
        </div>
        <div class='status'>
            <span>Main Board:</span>
            <span id='mainboot' class='chip )rawliteral";
        html += String(mainBooted ? "booted" : "notready");
        html += R"rawliteral('>)rawliteral";
        html += String(mainBooted ? "BOOTED" : "NOT READY");
        html += R"rawliteral(</span>
        </div>
        <div class='distance'>
            Distance: <span id='distance'>)rawliteral";
        if (lastDistance >= 0) html += String(lastDistance, 2) + " cm";
        else html += "N/A";
        html += R"rawliteral(</span>
        </div>
    </div>
    <script>
        function updateLedCircle(state) {
            var led = document.getElementById('ledcircle');
            if (state === 'ON') {
                led.classList.add('led-on');
                led.classList.remove('led-off');
            } else {
                led.classList.add('led-off');
                led.classList.remove('led-on');
            }
        }
        var es = new EventSource('/events');
        es.addEventListener('led', function(e) {
            document.getElementById('ledstate').textContent = e.data;
            updateLedCircle(e.data);
        });
        es.addEventListener('mainboot', function(e) {
            var el = document.getElementById('mainboot');
            el.textContent = e.data;
            el.className = 'chip ' + (e.data === 'BOOTED' ? 'booted' : 'notready');
        });
        es.addEventListener('distance', function(e) {
            document.getElementById('distance').textContent = (e.data === 'timeout' ? 'N/A' : e.data + ' cm');
        });
        // Initial LED circle state
        updateLedCircle(document.getElementById('ledstate').textContent);
    </script>
</body>
</html>
)rawliteral";
        request->send(200, "text/html", html);
    });
    server.addHandler(&events);
}

void handleUartAndSSE(AsyncEventSource &events, bool &mainBooted, bool &ledState, float &lastDistance) {
    static String uartBuffer = "";
    static unsigned long lastSseUpdate = 0;
    static bool prevMainBooted = false;
    static bool prevLedState = false;
    static float prevLastDistance = -9999.0;
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (uartBuffer.length() > 0) {
                int firstComma = uartBuffer.indexOf(',');
                int secondComma = uartBuffer.indexOf(',', firstComma + 1);
                if (firstComma > 0 && secondComma > firstComma) {
                    String bootedStr = uartBuffer.substring(0, firstComma);
                    String ledStr = uartBuffer.substring(firstComma + 1, secondComma);
                    String distStr = uartBuffer.substring(secondComma + 1);

                    // BOOTED
                    bool newMainBooted = (bootedStr == "1");
                    // LED_STATE
                    bool newLedState = (ledStr == "1");
                    // DISTANCE
                    float newLastDistance = -1.0;
                    bool isTimeout = (distStr == "timeout");
                    if (!isTimeout) newLastDistance = distStr.toFloat();

                    // Update cached values
                    mainBooted = newMainBooted;
                    ledState = newLedState;
                    lastDistance = isTimeout ? -1.0 : newLastDistance;
                }
                uartBuffer = "";
            }
        } else if (isPrintable(c)) {
            uartBuffer += c;
        }
    }
    // Force SSE update at 5 Hz (every 200 ms)
    unsigned long now = millis();
    if (now - lastSseUpdate >= 200) {
        lastSseUpdate = now;
        triggerSSE(events, "mainboot", mainBooted ? "BOOTED" : "NOT READY");
        digitalWrite(LED_PIN, ledState ? LOW : HIGH);
        triggerSSE(events, "led", ledState ? "ON" : "OFF");
        if (lastDistance == -1.0) {
            triggerSSE(events, "distance", "timeout");
        } else {
            triggerSSE(events, "distance", String(lastDistance, 2));
        }
        prevMainBooted = mainBooted;
        prevLedState = ledState;
        prevLastDistance = lastDistance;
    }
}
