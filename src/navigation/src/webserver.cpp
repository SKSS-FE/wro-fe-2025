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

void setupWebServer(AsyncWebServer &server, AsyncEventSource &events, bool &ledState, bool &mainBooted, bool &navigationBooted, float &lastDistance, float &lastDistance2, String &lastColor) {
    startWifiAP();
    server.on("/", HTTP_GET, [&](AsyncWebServerRequest *request){
        String html = R"rawliteral(
<!DOCTYPE html>
<html lang='en'>
<head>
    <meta charset='UTF-8'>
    <meta name='viewport' content='width=device-width, initial-scale=1.0'>
    <title>Board Status</title>
    <link rel='stylesheet' href='/tailwind.min.css'>
</head>
<body class='bg-gray-100 text-gray-900 m-0 p-0'>
    <div class='max-w-md mx-auto mt-10 bg-white rounded-xl shadow-lg p-8'>
        <h1 class='text-3xl font-bold mb-4 text-center'>Status</h1>
        <div class='flex items-center gap-3 mb-3'>
            <span id='ledcircle' class='w-5 h-5 rounded-full border-2 border-gray-400 inline-block'></span>
            <span>LED is <span id='ledstate' class='font-semibold'>)rawliteral";
        html += String(ledState ? "ON" : "OFF");
        html += R"rawliteral(</span></span>
        </div>
        <div class='flex items-center gap-3 mb-3'>
            <span>Navigation Board:</span>
            <span id='navboot' class='px-3 py-1 rounded-full text-sm font-medium ml-2 ' + String(navigationBooted ? "bg-green-100 text-green-800" : "bg-red-100 text-red-800") + '>)rawliteral";
        html += String(navigationBooted ? "BOOTED" : "NOT READY");
        html += R"rawliteral(</span>
        </div>
        <div class='flex items-center gap-3 mb-3'>
            <span>Main Board:</span>
            <span id='mainboot' class='px-3 py-1 rounded-full text-sm font-medium ml-2 ' + String(mainBooted ? "bg-green-100 text-green-800" : "bg-red-100 text-red-800") + '>)rawliteral";
        html += String(mainBooted ? "BOOTED" : "NOT READY");
        html += R"rawliteral(</span>
        </div>
        <div class='text-xl font-semibold mt-5'>
            Distance 1: <span id='distance'>)rawliteral";
        if (lastDistance >= 0) html += String(lastDistance, 2) + " cm";
        else html += "N/A";
        html += R"rawliteral(</span>
        </div>
        <div class='text-xl font-semibold mt-2'>
            Distance 2: <span id='distance2'>)rawliteral";
        if (lastDistance2 >= 0) html += String(lastDistance2, 2) + " cm";
        else html += "N/A";
        html += R"rawliteral(</span>
        </div>
        <div class='text-xl font-semibold mt-2'>
            Color: <span id='color'>)rawliteral";
        html += lastColor.length() > 0 ? lastColor : "N/A";
        html += R"rawliteral(</span>
        </div>
    </div>
    <script>
        function updateLedCircle(state) {
            var led = document.getElementById('ledcircle');
            if (state === 'ON') {
                led.classList.add('bg-green-500', 'border-green-700');
                led.classList.remove('bg-red-500', 'border-red-700');
            } else {
                led.classList.add('bg-red-500', 'border-red-700');
                led.classList.remove('bg-green-500', 'border-green-700');
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
            el.className = 'px-3 py-1 rounded-full text-sm font-medium ml-2 ' + (e.data === 'BOOTED' ? 'bg-green-100 text-green-800' : 'bg-red-100 text-red-800');
        });
        es.addEventListener('distance', function(e) {
            document.getElementById('distance').textContent = (e.data === 'timeout' ? 'N/A' : e.data + ' cm');
        });
        es.addEventListener('distance2', function(e) {
            document.getElementById('distance2').textContent = (e.data === 'timeout' ? 'N/A' : e.data + ' cm');
        });
        es.addEventListener('color', function(e) {
            document.getElementById('color').textContent = e.data;
        });
        // Initial LED circle state
        updateLedCircle(document.getElementById('ledstate').textContent);
    </script>
</body>
</html>
)rawliteral";
        request->send(200, "text/html", html);
    });
    server.on("/tailwind.min.css", HTTP_GET, [](AsyncWebServerRequest *request){
        const char* css = "body{background-color:#f3f4f6;color:#111827;margin:0;padding:0}.max-w-md{max-width:28rem}.mx-auto{margin-left:auto;margin-right:auto}.mt-10{margin-top:2.5rem}.bg-white{background-color:#fff}.rounded-xl{border-radius:0.75rem}.shadow-lg{box-shadow:0 10px 15px -3px rgba(0,0,0,.1),0 4px 6px -4px rgba(0,0,0,.1)}.p-8{padding:2rem}.text-3xl{font-size:1.875rem;line-height:2.25rem}.font-bold{font-weight:700}.mb-4{margin-bottom:1rem}.text-center{text-align:center}.flex{display:flex}.items-center{align-items:center}.gap-3{gap:0.75rem}.mb-3{margin-bottom:0.75rem}.w-5{width:1.25rem}.h-5{height:1.25rem}.rounded-full{border-radius:9999px}.border-2{border-width:2px}.border-gray-400{border-color:#9ca3af}.inline-block{display:inline-block}.font-semibold{font-weight:600}.px-3{padding-left:0.75rem;padding-right:0.75rem}.py-1{padding-top:0.25rem;padding-bottom:0.25rem}.text-sm{font-size:0.875rem;line-height:1.25rem}.font-medium{font-weight:500}.ml-2{margin-left:0.5rem}.bg-green-100{background-color:#d1fae5}.text-green-800{color:#065f46}.bg-red-100{background-color:#fee2e2}.text-red-800{color:#991b1b}.text-xl{font-size:1.25rem;line-height:1.75rem}.mt-5{margin-top:1.25rem}.bg-green-500{background-color:#22c55e}.border-green-700{border-color:#15803d}.bg-red-500{background-color:#ef4444}.border-red-700{border-color:#b91c1c}";
        request->send(200, "text/css", css);
    });
    server.addHandler(&events);
}

void handleUartAndSSE(AsyncEventSource &events, bool &mainBooted, bool &ledState, float &lastDistance, float &lastDistance2, String &lastColor) {
    static String uartBuffer = "";
    static unsigned long lastSseUpdate = 0;
    static bool prevMainBooted = false;
    static bool prevLedState = false;
    static float prevLastDistance = -9999.0;
    static float prevLastDistance2 = -9999.0;
    static String prevLastColor = "";
    while (Serial.available()) {
        char c = Serial.read();
        if (c == '\n' || c == '\r') {
            if (uartBuffer.length() > 0) {
                // Split CSV: BOOTED,LED_STATE,DIST1,DIST2,COLOR
                int firstComma = uartBuffer.indexOf(',');
                int secondComma = uartBuffer.indexOf(',', firstComma + 1);
                int thirdComma = uartBuffer.indexOf(',', secondComma + 1);
                int fourthComma = uartBuffer.indexOf(',', thirdComma + 1);
                if (firstComma > 0 && secondComma > firstComma && thirdComma > secondComma && fourthComma > thirdComma) {
                    String bootedStr = uartBuffer.substring(0, firstComma);
                    String ledStr = uartBuffer.substring(firstComma + 1, secondComma);
                    String dist1Str = uartBuffer.substring(secondComma + 1, thirdComma);
                    String dist2Str = uartBuffer.substring(thirdComma + 1, fourthComma);
                    String colorStr = uartBuffer.substring(fourthComma + 1);

                    // BOOTED
                    bool newMainBooted = (bootedStr == "1");
                    // LED_STATE
                    bool newLedState = (ledStr == "1");
                    // DISTANCE 1
                    float newLastDistance = -1.0;
                    bool isTimeout1 = (dist1Str == "timeout");
                    if (!isTimeout1) newLastDistance = dist1Str.toFloat();
                    // DISTANCE 2
                    float newLastDistance2 = -1.0;
                    bool isTimeout2 = (dist2Str == "timeout");
                    if (!isTimeout2) newLastDistance2 = dist2Str.toFloat();
                    // COLOR
                    String newLastColor = colorStr;

                    // Update cached values
                    mainBooted = newMainBooted;
                    ledState = newLedState;
                    lastDistance = isTimeout1 ? -1.0 : newLastDistance;
                    lastDistance2 = isTimeout2 ? -1.0 : newLastDistance2;
                    lastColor = newLastColor;
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
        if (lastDistance2 == -1.0) {
            triggerSSE(events, "distance2", "timeout");
        } else {
            triggerSSE(events, "distance2", String(lastDistance2, 2));
        }
        triggerSSE(events, "color", lastColor.length() > 0 ? lastColor : "N/A");
        prevMainBooted = mainBooted;
        prevLedState = ledState;
        prevLastDistance = lastDistance;
        prevLastDistance2 = lastDistance2;
        prevLastColor = lastColor;
    }
}
