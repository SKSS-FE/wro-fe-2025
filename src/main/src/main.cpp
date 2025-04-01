#include <Arduino.h>

#define BUTTON_PIN D1
#define LED_PIN    LED_BUILTIN
#define TRIG_PIN D2
#define ECHO_PIN D3

void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Button active low
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED off (inverted logic)
    pinMode(TRIG_PIN, OUTPUT);
    pinMode(ECHO_PIN, INPUT);

    Serial.begin(9600);
    Serial.println("");
    Serial.println("BOOT_OK");
}

void loop() {
    static bool ledState = false;
    static bool buttonReading = HIGH;    // Last raw reading
    static bool buttonState = HIGH;      // Last debounced state
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;

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
                // Send LED status in parseable format
                Serial.print("LED:");
                Serial.println(ledState ? "ON" : "OFF");
            }
        }
    }
    buttonReading = reading;

    // HC-SR04 distance measurement
    long duration;
    float distance;
    digitalWrite(TRIG_PIN, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG_PIN, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG_PIN, LOW);
    duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
    if (duration > 0) {
        distance = duration * 0.0343 / 2.0;

        // Send distance over UART in a parseable format
        Serial.print("DIST:");
        Serial.println(distance, 2); // 2 decimal places
    } else {
        // Send timeout over UART
        Serial.println("DIST:timeout");
    }

    delay(10); // Short delay
}