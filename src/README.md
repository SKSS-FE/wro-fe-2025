# Codebase
All code is written in C++ with PlatformIO. The project is split into three main components: Main (sensor/logic board), Navigation (WiFi/web dashboard), and Camera (TBD).

## UART CSV Reporting Schema (Single-Line, Periodic Reporting)

The main board sends a single CSV line over UART at 10 Hz, reporting all relevant status variables:

### CSV Schema
- **Format:**
  `BOOTED,LED_STATE,DISTANCE`
- **Fields:**
  - `BOOTED`: `1` if main board is booted, `0` otherwise
  - `LED_STATE`: `1` if LED is ON, `0` if OFF
  - `DISTANCE`: Floating-point value in centimeters (from ultrasonic sensor), or `timeout` if unavailable
- **Example Messages:**
  - `1,1,15.67`
  - `1,0,timeout`
  - `0,0,timeout`
- **Note:**
  - The main board caches the current values of all status variables and reports them together as a single CSV line at 10 Hz.
  - Each field is separated by a comma, and each line ends with a newline character (`\n`).

## Main Board (src/main)

**Hardware Connections:**
- Button: Connected to pin D1 (GPIO5), configured as INPUT_PULLUP.
- LED: Onboard LED (LED_BUILTIN), used as an indicator (active LOW).
- Distance Sensor: Ultrasonic sensor (e.g., HC-SR04) on pins D0 (Trig) and D2 (Echo).
- UART: TX pin sends data to the navigation board's RX pin. GND is shared.

**Functionality:**
- Detects button presses with debounce logic.
- Toggles the onboard LED each time the button is pressed.
- Measures distance using the ultrasonic sensor.
- Sends a CSV line over UART at 10 Hz with boot status, LED state, and distance (or 'timeout').

## Navigation Board (src/navigation)

**Hardware Connections:**
- UART: RX pin receives data from the main board's TX pin. GND is shared.
- WiFi: ESP8266 runs in Access Point (AP) mode, creating a WiFi network named "ESP8266_AP".

**Functionality:**
- Receives and parses the CSV status line from the main board via UART.
- Runs a web server on the ESP8266 using ESPAsyncWebServer.
- Serves a modern web dashboard (at the root URL) showing:
  - Main board boot status
  - Navigation board boot status
  - LED state (with colored indicator)
  - Last measured distance (in cm, or N/A if unavailable)
- Uses Server-Sent Events (SSE) to update the web page in real time as new data is received over UART.
- Operates as a WiFi access point, allowing clients to connect and view the web page.

## Camera

(Describe camera module usage here if needed)

---

**Build/Development:**
- Each board has its own PlatformIO project and `platformio.ini`.
- Main dependencies: Arduino framework, ESPAsyncWebServer (for navigation board).
- See each subfolder for board-specific code and configuration.
