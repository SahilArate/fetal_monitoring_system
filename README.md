# Fetal Movement Monitoring System

This project implements a Fetal Movement Monitoring System using an ESP32 microcontroller, MPU6050 (accelerometer/gyroscope), and an FSR (Force Sensitive Resistor) sensor. The system monitors and tracks fetal movements and kicks, displaying real-time data on a web-based dashboard with an interactive graph.

## Features

* **Real-time Monitoring**: Continuously monitors fetal movements and kicks using MPU6050 and FSR sensors.
* **Web Dashboard**: Provides a user-friendly web interface to visualize real-time data.
* **Accelerometer Data**: Displays acceleration data (X, Y, Z axes) from the MPU6050.
* **Movement Counter**: Tracks and displays the total number of detected movements.
* **Kick Counter**: Tracks and displays the total number of detected kicks.
* **Interactive Graph**: Visualizes acceleration data over time, with markers for detected kicks.
* **CSV Data Download**: Allows users to download recorded movement and kick data as a CSV file for further analysis.
* **Wireless Connectivity**: Utilizes ESP8266 (ESP32 in your case, though the code provided uses ESP8266 libraries) Wi-Fi capabilities to host the web server.

## Hardware Requirements

* ESP32 Development Board (e.g., ESP32-WROOM-32)
* MPU6050 Accelerometer and Gyroscope Module
* FSR (Force Sensitive Resistor) Sensor
* Jumper Wires
* Breadboard (optional, for prototyping)
* Micro-USB Cable for ESP32

## Software Requirements

* Arduino IDE
* ESP32 Board Package for Arduino IDE
* MPU6050 Library (e.g., `Adafruit_MPU6050` or `MPU6050` by Jeff Rowberg)
* Libraries for ESP8266WebServer and ESP8266WiFi (already present in ESP32 core for web server functionalities)

## Installation and Setup

### 1. Arduino IDE Setup

1.  **Install Arduino IDE**: If you don't have it, download and install the Arduino IDE from the official website.
2.  **Add ESP32 Board Manager URL**:
    * Go to `File > Preferences`.
    * In "Additional Boards Manager URLs", add: `https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json`
    * Click "OK".
3.  **Install ESP32 Boards**:
    * Go to `Tools > Board > Boards Manager...`.
    * Search for "esp32" and install the "esp32 by Espressif Systems" package.
4.  **Install Libraries**:
    * Go to `Sketch > Include Library > Manage Libraries...`.
    * Search for "MPU6050" and install a suitable library (e.g., "MPU6050 by Jeff Rowberg" or "Adafruit MPU6050").
    * The `ESP8266WiFi.h` and `ESP8266WebServer.h` libraries are part of the ESP32 core and should be available automatically once the ESP32 board package is installed.

### 2. Wiring

Connect the components as follows:

**MPU6050 to ESP32:**

| MPU6050 Pin | ESP32 Pin (Default I2C) |
| :---------- | :---------------------- |
| VCC         | 3.3V                    |
| GND         | GND                     |
| SCL         | GPIO22 (SCL)            |
| SDA         | GPIO21 (SDA)            |

**FSR Sensor to ESP32:**

Connect the FSR in a voltage divider configuration with a pull-down resistor (e.g., 10k Ohm).

| FSR Pin     | ESP32 Pin               |
| :---------- | :---------------------- |
| One side    | 3.3V                    |
| Other side  | Analog Pin (e.g., GPIO34) & 10k Ohm Resistor to GND |

* **Note**: The provided `FMS_v2.3.2.ino` code doesn't explicitly show FSR sensor integration, only MPU6050. You will need to add the FSR reading logic and incorporate its data into the web dashboard. [cite_start]For instance, `kickCount` is currently derived from MPU6050 data (`totalDelta > kickThreshold`)[cite: 20]. You'd modify this to read from the FSR.

### 3. Uploading the Code

1.  Open `FMS_v2.3.2.ino` in Arduino IDE.
2.  [cite_start]**Update Wi-Fi Credentials**: Modify the `ssid` and `password` variables in the code to match your Wi-Fi network credentials. [cite: 1, 2]
    ```cpp
    const char* ssid = "YOUR_WIFI_SSID";
    const char* password = "YOUR_WIFI_PASSWORD";
    ```
3.  **Select Board**: Go to `Tools > Board` and select your ESP32 board (e.g., "ESP32 Dev Module").
4.  **Select Port**: Go to `Tools > Port` and select the serial port connected to your ESP32.
5.  **Upload**: Click the "Upload" button (right arrow icon) to compile and upload the code to your ESP32.

## Usage

1.  [cite_start]After uploading, open the Serial Monitor (Tools > Serial Monitor) at a baud rate of 115200. [cite: 8]
2.  [cite_start]The ESP32 will attempt to connect to your Wi-Fi network. [cite: 9]
3.  [cite_start]Once connected, it will print its local IP address. [cite: 10]
4.  Open a web browser on a device connected to the *same* Wi-Fi network and navigate to the IP address displayed in the Serial Monitor (e.g., `http://192.168.1.100`).
5.  The web dashboard will load, displaying real-time acceleration data, movement count, and kick count. You will also see an interactive graph of the Z-axis acceleration.
6.  You can click the "Download Report" button to get a CSV file of the recorded data.

## Code Overview

* [cite_start]**MPU6050 Initialization**: The `setup()` function initializes the MPU6050 sensor, setting its full-scale accelerometer range and digital low-pass filter. [cite: 8]
* [cite_start]**Calibration**: Upon startup, the MPU6050 calibrates itself by taking 50 samples to establish baseline accelerometer values ( `baseAx`, `baseAy`, `baseAz`). [cite: 3, 4, 11, 12, 13, 14]
* [cite_start]**Movement Detection**: Movements are detected based on significant deviation or change in relative accelerometer values (`relativeAx`, `relativeAy`, `relativeAz`) from the calibrated baseline, with a `movementThreshold` and `movementCooldown` to prevent multiple counts for a single event. [cite: 16, 17, 18, 19]
* [cite_start]**Kick Detection**: Kicks are identified when the `totalDelta` (sum of absolute changes in X, Y, and Z accelerations, with Z-axis weighted higher) exceeds a `kickThreshold`, also with a `kickCooldown`. [cite: 20, 21]
* [cite_start]**Web Server**: An `ESP8266WebServer` instance serves an HTML page (`handleRoot()`) and provides a JSON API endpoint (`/data`) for real-time sensor values. [cite: 10, 22, 23, 24, 25, 26]
* **Front-end (HTML, CSS, JavaScript)**: The `handleRoot()` function serves an HTML page that includes:
    * [cite_start]**Styling**: Modern CSS for a clean and responsive design. [cite: 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63, 64, 65, 66, 67, 68, 69, 70, 71, 72, 73, 74, 75, 76, 77, 78, 79, 80, 81, 82, 83, 84, 85, 86, 87, 88, 89, 90, 91, 92, 93, 94]
    * [cite_start]**Chart.js**: For plotting real-time acceleration data. [cite: 106]
    * [cite_start]**JavaScript**: Fetches data from the ESP32's `/data` endpoint every 2 seconds and updates the dashboard and graph. [cite: 118, 124] [cite_start]It also handles CSV data export. [cite: 125, 126, 127, 128, 129, 130, 131]
* [cite_start]**Data Handling**: The `handleData()` function prepares a JSON string containing current acceleration values (relative to baseline), movement count, and kick count. [cite: 22, 23, 24, 25, 26]

## Future Enhancements

* **FSR Integration**: Fully integrate FSR sensor readings into the `kickCount` logic.
* **Data Logging to SD Card**: Store historical data on an SD card for long-term analysis.
* **Cloud Integration**: Send data to a cloud platform (e.g., Firebase, AWS IoT, Thingspeak) for remote monitoring and advanced analytics.
* **Notifications**: Implement push notifications or SMS alerts for significant events.
* **Battery Monitoring**: Add battery level monitoring for portable usage.
* **Improved Kick Detection Algorithm**: Refine the kick detection algorithm to differentiate between various types of movements and true kicks more accurately, possibly using machine learning.
* **Mobile Application**: Develop a dedicated mobile application for a better user experience.

## Contributing

Contributions are welcome! Please feel free to open issues or submit pull requests.

## License

This project is open-source and available under the MIT License.
