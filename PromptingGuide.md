# AI Agent Prompting Guide: Building WeatherBoard from Scratch

This guide provides a step-by-step sequence of prompts to instruct an AI coding assistant (such as Antigravity, Claude, or ChatGPT) to build, simulate, and flash the **WeatherBoard** project (or any similar ESP32 embedded IoT project) from an empty directory.

Rather than giving the AI one massive prompt—which often leads to hallucinated pinouts, skipped dependencies, or mismatched build paths—this guide breaks the build down into logical, testable milestones.

---

## Architecture & Project Overview

Before starting, keep in mind the stack being built:
- **Target Hardware:** ESP32 DevKit V1 (4MB Flash)
- **Framework & CLI:** Arduino Framework managed via `arduino-cli`
- **Sensors & Output:** DHT22 (Temp/Humidity) + SSD1306 I2C OLED (128x64) + Onboard LED (GPIO 2)
- **Connectivity:** Wi-Fi Access Point (`WeatherBoard-AP`) hosting an embedded HTTP web dashboard
- **Simulation:** Wokwi Simulator (`wokwi.toml` + `diagram.json`)

---

## Phase 1: Agent Ground Rules & Project Setup

Start by giving the agent its boundaries and guidelines so it doesn't default to OS temp directories or forget to sync simulation files.

### Prompt 1.1: Create `AGENTS.md` and `.gitignore`
```markdown
I am starting a new embedded ESP32 project called WeatherBoard.
First, create two foundation files:

1. AGENTS.md: Document rules for any AI agent working on this repo:
   - Framework: Arduino framework.
   - CLI & Build Tool: Use arduino-cli for compiling, library management, and flashing.
   - Build Path: Always compile using an explicit build path pointing to the local `./build` directory rather than relying on OS temp directories.
   - Version Control: Ensure `./build` is strictly excluded in `.gitignore`.
   - Simulation: Use Wokwi for simulation. Configure wokwi.toml at project root pointing to binary and ELF outputs in `./build`.
   - Hardware Synchronization: Whenever hardware components or pin assignments are added, removed, or modified, keep both the source code and diagram.json synchronized.

2. .gitignore: Exclude `./build/`, `.cache/`, and common IDE temporary files.
```

---

## Phase 2: Toolchain & Dependency Installation

Have the agent set up `arduino-cli`, the ESP32 platform core, and the required sensor/display libraries.

### Prompt 2.1: Toolchain & Core Setup
```markdown
Please check if arduino-cli is installed on this system.
If not, install it or guide me through installing it.
Once available:
1. Initialize the arduino-cli configuration if not already present.
2. Add the ESP32 board manager URL:
   https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json
3. Update the core index and install the `esp32:esp32` platform core.
```

### Prompt 2.2: Install Required Libraries
```markdown
Using arduino-cli, install the necessary libraries for our sensors and display:
- "Adafruit SSD1306"
- "Adafruit GFX Library"
- "DHT sensor library"
- "Adafruit Unified Sensor"
Verify that all four libraries install successfully.
```

---

## Phase 3: Initial Hardware Application (OLED + DHT22)

Build the foundational sketch that reads temperature and humidity and renders the data to the OLED screen.

### Prompt 3.1: Create `WeatherBoard.ino`
```markdown
Create the initial sketch `WeatherBoard.ino` for an ESP32 DevKit V1:
- Display: 128x64 SSD1306 I2C OLED (Address 0x3C, SDA: GPIO 21, SCL: GPIO 22).
- Sensor: DHT22 connected to GPIO 4.
- Functionality:
  - Initialize Serial at 115200 baud.
  - In setup(), show a welcome/initialization screen on the OLED.
  - In loop(), read temperature (°C and °F) and relative humidity every 2 seconds (non-blocking using millis()).
  - Draw a formatted UI on the OLED:
    - Top header: "WEATHER STATION"
    - Middle: Temperature in both Celsius and Fahrenheit
    - Bottom: Humidity percentage and a pulsing heartbeat indicator dot
  - Print readings to Serial.
  - Handle sensor read errors gracefully by displaying a warning message on the OLED.
```

---

## Phase 4: Wokwi Simulation Configuration

Before connecting physical hardware, configure the local simulation environment so you can test immediately.

### Prompt 4.1: Configure Wokwi Simulation
```markdown
Set up the Wokwi simulation configuration files:

1. wokwi.toml:
   - Point firmware to "build/WeatherBoard.ino.bin"
   - Point elf to "build/WeatherBoard.ino.elf"

2. diagram.json:
   - Part 1: "wokwi-esp32-devkit-v1"
   - Part 2: "board-ssd1306" (I2C address 0x3c)
   - Part 3: "wokwi-dht22"
   - Wiring connections:
     - OLED GND -> ESP32 GND
     - OLED VCC -> ESP32 3V3
     - OLED SCL -> ESP32 D22
     - OLED SDA -> ESP32 D21
     - DHT22 VCC -> ESP32 3V3
     - DHT22 GND -> ESP32 GND
     - DHT22 SDA -> ESP32 D4
     - Serial Monitor TX/RX connected to ESP32 TX/RX

Verify diagram.json matches the pin definitions in WeatherBoard.ino.
```

---

## Phase 5: Local Compilation & Simulation Testing

Validate that the project builds cleanly into the local `./build` directory and matches Wokwi's target paths.

### Prompt 5.1: Build the Project
```markdown
Compile the sketch using arduino-cli:
- Board FQBN: esp32:esp32:esp32doit-devkit-v1
- Build Path: ./build
- Target sketch: ./WeatherBoard.ino

Verify that `./build/WeatherBoard.ino.bin` and `./build/WeatherBoard.ino.elf` are generated and match wokwi.toml.
Explain how I can start the simulator in VS Code.
```

---

## Phase 6: Physical Hardware Detection & Flashing

Connect the physical board, ensure port permissions, and flash the firmware.

### Prompt 6.1: Port Detection & Permission Check
```markdown
I have plugged an ESP32 board into my computer via USB.
1. Scan for connected serial devices using arduino-cli and list the detected ports.
2. Check file permissions on the detected port (e.g. /dev/ttyUSB0).
3. If permissions are restricted (e.g. root:uucp or root:dialout), explain what command I should run to grant permanent user access.
4. Verify communication with the board using esptool to check its chip model and MAC address.
```

### Prompt 6.2: Flash the Firmware
```markdown
Flash the compiled binaries from `./build` to the connected ESP32 on /dev/ttyUSB0.
After flashing, read a few lines from the serial monitor at 115200 baud to verify that the DHT22 sensor readings are printing properly.
```

---

## Phase 7: Feature Upgrade (Wi-Fi AP + Web Dashboard + LED Toggle)

Now expand the project by adding Wi-Fi capabilities and a web dashboard.

### Prompt 7.1: Implement Wi-Fi AP, Web Server & LED Control
```markdown
Upgrade the WeatherBoard project with network and web capabilities:

1. Wi-Fi Access Point:
   - Configure the ESP32 to start a local Wi-Fi Access Point named "WeatherBoard-AP".
   - Start a captive portal DNS server on port 53 redirecting requests to the AP IP (192.168.4.1).

2. Web Dashboard:
   - Host an embedded, mobile-responsive HTML/CSS/JS page served directly from PROGMEM on port 80.
   - Display real-time Temperature (°C and °F) and Humidity cards.
   - Use non-blocking JavaScript fetch() polling to `/data` every 2 seconds so the metrics update live without refreshing the browser page.

3. Onboard LED Control:
   - On ESP32 DevKit V1, use the onboard blue LED (GPIO 2).
   - Add a button on the webpage that toggles blinking of the LED on/off.
   - Blinking should use a non-blocking millis() timer (500ms interval) so it does not block the web server or sensor reads.
   - The web page button and status indicator should update dynamically when toggled.

4. Synchronization & Build:
   - Update the OLED header to display "AP: 192.168.4.1" and show an indicator when LED blinking is active.
   - Update diagram.json to add a blue status LED on GPIO 2 so the blinking is visible in Wokwi simulation.
   - Recompile the sketch into `./build`.
```

---

## Phase 8: Wi-Fi Security & Documentation

Add network security and update the repository documentation.

### Prompt 8.1: Add Wi-Fi Password & Document
```markdown
Let's secure the Wi-Fi Access Point:
1. Set the WPA2 password for "WeatherBoard-AP" to "BlueJays".
2. Update the OLED startup screen to display the SSID, Password, and IP.
3. Update README.md with:
   - Project features and architecture
   - Full pinout and wiring table (including the GPIO 2 status LED)
   - Wi-Fi credentials (SSID: WeatherBoard-AP, Password: BlueJays, IP: 192.168.4.1)
   - Step-by-step instructions for Wokwi simulation and physical flashing
4. Recompile the project to `./build` and flash the updated firmware to the board.
```

---

## Phase 9: Git Commit & Push

Lock in your work with a clean commit history.

### Prompt 9.1: Commit and Push
```markdown
Review `git status` and `git diff` to make sure only source and documentation files are tracked (and `./build` remains ignored).
Stage the changes, create a descriptive commit message, and push to origin main.
```

---

## Best Practices for Prompting AI Agents on Embedded Projects

1. **Enforce Local Build Paths:**
   Always tell the agent to pass `--build-path ./build` to `arduino-cli`. Defaulting to temp directories breaks Wokwi integration and makes flashing reproducible binaries difficult.

2. **Insist on Non-Blocking Code:**
   Explicitly instruct the agent to avoid `delay()` in `loop()`. On an ESP32 running Wi-Fi AP, DNS, WebServer, and sensor readings simultaneously, blocking delays will cause dropped web requests and Wi-Fi disconnects.

3. **Cache Slow Sensor Reads:**
   DHT sensors take 20ms+ of timing-sensitive bit-banging. Never read the DHT inside an HTTP request handler. Have `loop()` cache the readings every 2 seconds, and let HTTP handlers serve the cached values instantly.

4. **Synchronize Hardware & Diagram:**
   Any time a GPIO pin is introduced (such as the LED on GPIO 2), instruct the agent in the same prompt to update `diagram.json`. This ensures your Wokwi simulation never drifts out of sync with your physical circuit.
