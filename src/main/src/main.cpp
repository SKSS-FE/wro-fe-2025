#include <Arduino.h>
#include <Servo.h>
#include <SoftwareSerial.h>

// GY-25 definitions
#define GY25_SERIAL gy25Serial // Use software serial for GY-25
#define GY25_RX_PIN D2 
#define GY25_TX_PIN D3 
SoftwareSerial gy25Serial(GY25_RX_PIN, GY25_TX_PIN); // RX, TX

#define BUTTON_PIN D0 
#define LED_PIN    LED_BUILTIN
#define IN1_PIN D7
#define IN2_PIN D6
#define ENA_PIN D5
#define SERVO_PIN D1  // Servo control pin

Servo myServo;
// Forward declaration for servo test
void testServoSweep();

enum MotorDirection { MOTOR_STOP, MOTOR_FORWARD, MOTOR_BACKWARD };

void setMotor(MotorDirection dir, uint8_t speed) {
    switch (dir) {
        case MOTOR_FORWARD:
            digitalWrite(IN1_PIN, HIGH);
            digitalWrite(IN2_PIN, LOW);
            break;
        case MOTOR_BACKWARD:
            digitalWrite(IN1_PIN, LOW);
            digitalWrite(IN2_PIN, HIGH);
            break;
        case MOTOR_STOP:
        default:
            digitalWrite(IN1_PIN, LOW);
            digitalWrite(IN2_PIN, LOW);
            break;
    }
    analogWrite(ENA_PIN, speed); // 0-1023 for ESP8266
}

float gy25Roll = 0, gy25Pitch = 0, gy25Yaw = 0;
unsigned char gy25Buf[8];
uint8_t gy25Counter = 0;

void gy25SerialEvent() {
    while (GY25_SERIAL.available()) {
        gy25Buf[gy25Counter] = (unsigned char)GY25_SERIAL.read();
        if (gy25Counter == 0 && gy25Buf[0] != 0xAA) return;
        gy25Counter++;
        if (gy25Counter == 8) {
            gy25Counter = 0;
            if (gy25Buf[0] == 0xAA && gy25Buf[7] == 0x55) {
                gy25Yaw = (int16_t)(gy25Buf[1] << 8 | gy25Buf[2]) / 100.0;
                gy25Pitch = (int16_t)(gy25Buf[3] << 8 | gy25Buf[4]) / 100.0;
                gy25Roll = (int16_t)(gy25Buf[5] << 8 | gy25Buf[6]) / 100.0;
            }
        }
    }
}

void setup() {
    // LED off
    digitalWrite(LED_PIN, HIGH); // LED off (inverted logic)

    pinMode(BUTTON_PIN, INPUT_PULLUP); // Button active low
    pinMode(LED_PIN, OUTPUT);
    pinMode(IN1_PIN, OUTPUT);
    pinMode(IN2_PIN, OUTPUT);
    pinMode(ENA_PIN, OUTPUT);

    myServo.attach(SERVO_PIN);
    myServo.write(90); // Initialize servo to center (90°)
 
    Serial.begin(115200); // Add this line for debug output
    delay(100);
    Serial.println("[SETUP] Serial started");

    GY25_SERIAL.begin(9600); // GY-25 default baudrate
    Serial.println("[SETUP] GY-25 serial started");

    // Correction mode for GY-25
    GY25_SERIAL.write(0xA5);
    GY25_SERIAL.write(0x55);
    Serial.println("[SETUP] Sent 0xA5 0x55 to GY-25");

    // Wait for GY-25 to stabilize
    delay(4000);
    Serial.println("[SETUP] Waited 4s for GY-25");

    GY25_SERIAL.write(0xA5);
    GY25_SERIAL.write(0x52);
    Serial.println("[SETUP] Sent 0xA5 0x52 to GY-25");

    setMotor(MOTOR_STOP, 0);

    // Turn on LED to indicate setup complete
    digitalWrite(LED_PIN, LOW); // LED on
    Serial.println("[SETUP] Setup complete");
}


void loop() {
    // Read data from GY-25
    static bool click = false;
    static bool lastReading = HIGH;    // Last raw reading
    static bool buttonState = HIGH;    // Debounced button state
    static unsigned long lastDebounceTime = 0;
    const unsigned long debounceDelay = 50;
    static unsigned long lastUartReport = 0;
    static bool mainBooted = true;
    static bool ledState = false;
    static float uartYaw = 0.0; // Value received from UART
    static unsigned long lastGy25Print = 0;
    const unsigned long gy25PrintInterval = 100; // 100ms between prints

    static float navigationDistance1 = -1.0; // Value received from navigation board
    static float navigationDistance2 = -1.0;
    static int navigationBooted = 0;
    static int navigationLed = 0;
    static int navigationColorR = 0, navigationColorG = 0, navigationColorB = 0;

    int reading = digitalRead(BUTTON_PIN);
    if (reading != lastReading) {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay) {
        if (reading != buttonState) {
            buttonState = reading;
            if (buttonState == LOW) { // Button pressed
                click = !click;
                ledState = click;
                if (click) {
                    setMotor(MOTOR_BACKWARD, 255); // Start motor at 50% speed, negative direction
                } else {
                    setMotor(MOTOR_STOP, 0); // Stop motor
                }
            }
        }
    }
    lastReading = reading;

    // Read UART from Serial (USB) and parse CSV: yaw,booted,led\n
    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        float yawVal;
        int bootedVal, ledVal;
        int parsed = sscanf(line.c_str(), "%f,%d,%d", &yawVal, &bootedVal, &ledVal);
        if (parsed == 3) {
            uartYaw = yawVal;
            mainBooted = bootedVal;
            ledState = ledVal;
        }
    }

    // Read UART from navigation (USB Serial) and parse CSV: BOOTED,LED_STATE,DISTANCE1,DISTANCE2,COLOR\n
    if (Serial.available()) {
        String line = Serial.readStringUntil('\n');
        int bootedVal, ledVal;
        float dist1, dist2;
        int r, g, b;
        // Try to parse: booted,led,dist1,dist2,r,g,b
        int parsed = sscanf(line.c_str(), "%d,%d,%f,%f,%d,%d,%d", &bootedVal, &ledVal, &dist1, &dist2, &r, &g, &b);
        if (parsed == 7) {
            navigationBooted = bootedVal;
            navigationLed = ledVal;
            navigationDistance1 = dist1;
            navigationDistance2 = dist2;
            navigationColorR = r;
            navigationColorG = g;
            navigationColorB = b;
        }
    }

    // GY-25 UART CSV output (always print one line, even if no data)
    unsigned long now = millis();
    if (GY25_SERIAL.available()) {
        gy25SerialEvent();
    }
    if (now - lastGy25Print >= gy25PrintInterval) {
        lastGy25Print = now;
        Serial.print(gy25Yaw);
        Serial.print(",");
        Serial.print(mainBooted ? 1 : 0);
        Serial.print(",");
        Serial.print(ledState ? 1 : 0);
        Serial.println();
    }

    // UART reporting at 10 Hz (every 100 ms)
    if (now - lastUartReport >= 100) {
        lastUartReport = now;
        int booted = mainBooted ? 1 : 0;
        int led = ledState ? 1 : 0;
        Serial.printf("%d,%d,timeout\n", booted, led);
    }

    delay(10); // Small delay for loop stability
}