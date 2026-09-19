# WeatherBoard

An ESP32-based weather monitor that measures ambient temperature and humidity using a **DHT22** sensor and displays real-time readings on an **SSD1306 128x64 I2C OLED** screen.

This repository is configured for both physical deployment and simulation using [Wokwi](https://wokwi.com/).

---

## Hardware & Pinout

### Components
- **Microcontroller:** ESP32 DevKit V1
- **Display:** 0.96" SSD1306 I2C OLED (128x64, Address `0x3C`)
- **Sensor:** DHT22 (AM2302) Temperature & Humidity Sensor

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

The complete hardware layout and simulation wiring can be found in [diagram.json](diagram.json).

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

```powershell
arduino-cli compile --fqbn esp32:esp32:esp32doit-devkit-v1 --build-path ./build ./WeatherBoard.ino
```

### 2. Upload to Physical ESP32

Identify your board's COM port (e.g. `COM6`) and flash:

```powershell
arduino-cli upload -p COM6 --fqbn esp32:esp32:esp32doit-devkit-v1 --input-dir ./build
```

---

## Simulation with Wokwi

This repository includes a [wokwi.toml](wokwi.toml) configuration referencing `./build/WeatherBoard.ino.bin` and `./build/WeatherBoard.ino.elf`.

1. Compile the sketch to generate the `./build` binaries:
   ```powershell
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

