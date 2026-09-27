# WeatherBoard

An ESP32-based weather monitor that measures ambient temperature and humidity using a **DHT22** sensor, displays real-time readings on an **SSD1306 128x64 I2C OLED** screen, and hosts a **Wi-Fi Access Point with an interactive Web Dashboard**.

This repository is configured for both physical deployment and simulation using [Wokwi](https://wokwi.com/).

---

## Features

- **Sensors & Display:** Real-time temperature (°C/°F) and relative humidity monitoring on both the OLED screen and web dashboard.
- **Wi-Fi Access Point (AP Mode):** Broadcasts its own local Wi-Fi network (`WeatherBoard-AP`). Connect directly with any phone or computer without needing an external router.
- **Responsive Web Dashboard:** Modern, mobile-first web interface served directly from ESP32 flash memory (`http://192.168.4.1`), auto-refreshing readings every 2 seconds without full-page reloads.
- **Onboard LED Control:** Interactive toggle button on the web page to activate or stop blinking the ESP32 onboard LED (GPIO 2).
- **Captive Portal Support:** Built-in DNS server automatically redirects mobile devices to the dashboard upon connection.

---

## Hardware & Pinout

### Components
- **Microcontroller:** ESP32 DevKit V1
- **Display:** 0.96" SSD1306 I2C OLED (128x64, Address `0x3C`)
- **Sensor:** DHT22 (AM2302) Temperature & Humidity Sensor
- **Status LED:** Onboard Blue LED (GPIO 2)

### Wiring Diagram

| Component | Pin | ESP32 Pin | Notes |
| :--- | :--- | :--- | :--- |
| **SSD1306 OLED** | GND | GND | Ground |
| | VCC | 3V3 | 3.3V Power |
| | SCL | GPIO 22 | Default I2C Clock |
| | SDA | GPIO 21 | Default I2C Data |
| **DHT22 Sensor** | VCC | 3V3 | 3.3V Power |
| | GND | GND | Ground |
| | SDA / DATA | GPIO 4 | Digital Signal Pin |
| **Status LED** | Anode (+) | GPIO 2 | Built-in LED on ESP32 DevKit V1 |
| | Cathode (-) | GND | Ground |

The complete hardware layout and simulation wiring can be found in [diagram.json](diagram.json).

---

## Wi-Fi & Web Dashboard Usage

1. Power on the ESP32 (or start the Wokwi simulation).
2. The OLED screen and Serial Monitor will show:
   - **SSID:** `WeatherBoard-AP`
   - **IP:** `192.168.4.1`
3. Connect your phone or laptop Wi-Fi to **`WeatherBoard-AP`** (open network, no password required by default).
4. Open your browser and navigate to:
   ```
   http://192.168.4.1
   ```
5. View live temperature and humidity metrics, and click **Start LED Blink** to toggle the onboard LED blinking.

---

## Software Dependencies

The sketch is built with the **Arduino framework** and requires the following libraries:

- `Adafruit SSD1306`
- `Adafruit GFX Library`
- `Adafruit Unified Sensor`
- `DHT sensor library`

Install them via `arduino-cli`:

```bash
arduino-cli lib install "Adafruit SSD1306"
arduino-cli lib install "Adafruit GFX Library"
arduino-cli lib install "DHT sensor library"
arduino-cli lib install "Adafruit Unified Sensor"
```

---

## Build & Flash (Arduino CLI)

Per project guidelines in [AGENTS.md](AGENTS.md), always compile using an explicit build path pointing to `./build`.

### 1. Compile the Sketch

```bash
arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 --build-path ./build ./WeatherBoard.ino
```

### 2. Upload to Physical ESP32

Identify your board's serial port (e.g. `/dev/ttyUSB0` on Linux, or `COMx` on Windows) and flash:

```bash
arduino-cli upload -p /dev/ttyUSB0 --fqbn esp32:esp32:esp32doit-devkit-v1 --input-dir ./build
```

---

## Simulation with Wokwi

This repository includes a [wokwi.toml](wokwi.toml) configuration referencing `./build/WeatherBoard.ino.bin` and `./build/WeatherBoard.ino.elf`.

1. Compile the sketch to generate the `./build` binaries:
   ```bash
   arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 --build-path ./build ./WeatherBoard.ino
   ```
2. Open the project in VS Code with the **Wokwi for VS Code** extension installed.
3. Open [diagram.json](diagram.json) and start the simulation (`F1` -> `Wokwi: Start Simulator`).

---

## Project Structure

```text
WeatherBoard/
├── .gitignore          # Ignores ./build/ and temporary build caches
├── AGENTS.md           # Guidelines for AI agents and build tooling
├── diagram.json        # Wokwi simulation parts and connection schema
├── README.md           # Project documentation and setup guide
├── WeatherBoard.ino    # Main Arduino application sketch
└── wokwi.toml          # Wokwi simulation configuration
```
