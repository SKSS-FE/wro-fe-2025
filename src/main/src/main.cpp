#include <Arduino.h>

#define BUTTON_PIN D1
#define LED_PIN    LED_BUILTIN
void setup() {
    pinMode(BUTTON_PIN, INPUT_PULLUP); // Button active low
    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, HIGH); // LED off (inverted logic)

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
                Serial.println(ledState ? "0" : "1");
            }
        }
    }
    buttonReading = reading;
    delay(10); // Short delay
}