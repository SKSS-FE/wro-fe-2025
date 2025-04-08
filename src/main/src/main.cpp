#include <Arduino.h>
#include "distance_sensor.h"

#define BUTTON_PIN D1
#define LED_PIN    LED_BUILTIN

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Button active low
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED off (inverted logic)

    // Create and initialize distance sensor object
    static DistanceSensor sensor(D0, D2);
    sensor.begin();

    Serial.begin(115200);
    Serial.println("");
}

void loop() {
    static bool ledState = false;
    static bool buttonReading = HIGH;    // Last raw reading
    static bool buttonState = HIGH;      // Last debounced state
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;
    static bool mainBooted = true; // Always 1 after setup
    static float lastDistance = -1.0;
    static unsigned long lastUartReport = 0;
    static DistanceSensor sensor(D0, D2);

    bool reading = digitalRead(BUTTON_PIN);
    if (reading != buttonReading) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == LOW) {
                ledState = !ledState;
                digitalWrite(LED_PIN, ledState ? LOW : HIGH);
            }
        }
    }
    buttonReading = reading;

    // Distance measurement using DistanceSensor class
    float distance = sensor.read();
    if (distance > 0) {
        lastDistance = distance;
    } else {
        lastDistance = -1.0;
    }

    // UART reporting at 10 Hz (every 100 ms)
    unsigned long now = millis();
    if (now - lastUartReport >= 100) {
        lastUartReport = now;
        int booted = mainBooted ? 1 : 0;
        int led = ledState ? 1 : 0;
        if (lastDistance >= 0) {
            Serial.printf("%d,%d,%.2f\n", booted, led, lastDistance);
        } else {
            Serial.printf("%d,%d,timeout\n", booted, led);
        }
    }
    delay(10); // Small delay for loop stability
}