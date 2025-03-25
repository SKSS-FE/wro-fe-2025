# Codebase
All code is written in C++ with PlatformIO.

## Main

**Hardware Connections:**
- Button: Connected to pin D1 (GPIO5), configured as INPUT_PULLUP.
- LED: Onboard LED (LED_BUILTIN), used as an indicator.
- UART: TX pin sends data to the navigation board's RX pin. GND is shared.

**Functionality:**
- Detects button presses with debounce logic.
- Toggles the onboard LED each time the button is pressed.
- Sends the LED status (ON/OFF) over UART (Serial) in the format `LED:ON` or `LED:OFF`.

## Navigation

**Hardware Connections:**
- UART: RX pin receives data from the main board's TX pin. GND is shared.
- WiFi: ESP8266 runs in Access Point (AP) mode, creating a WiFi network named "ESP8266_AP".

**Functionality:**
- Receives the LED status from the main board via UART.
- Runs a web server on the ESP8266.
- Serves a web page (at the root URL) showing the current LED status (ON/OFF).
- Updates the displayed status in real time as new data is received over UART.
- Operates as a WiFi access point, allowing clients to connect and view the web page.

## Camera

(Describe camera module usage here if needed)
