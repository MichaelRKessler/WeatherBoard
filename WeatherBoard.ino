#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>
#include <WiFi.h>
#include <WebServer.h>
#include <DNSServer.h>

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Pin assignments (matching diagram.json)
#define DHTPIN   4
#define DHTTYPE  DHT22
#define I2C_SDA  21
#define I2C_SCL  22
#define LED_PIN  2  // Built-in LED on ESP32 DevKit V1

// Wi-Fi Access Point configuration
const char* apSSID = "WeatherBoard-AP";
const char* apPassword = "BlueJays"; // WPA2-PSK password (8 characters)

// Hardware instances
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);
WebServer server(80);
DNSServer dnsServer;

// Sensor reading cache and states
unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000; // DHT22 minimum read interval is 2 seconds
bool heartbeat = false;
float currentTempC = 0.0;
float currentTempF = 0.0;
float currentHumidity = 0.0;
bool sensorError = false;

// LED Blink control states
bool ledBlinking = false;
unsigned long lastBlinkTime = 0;
const unsigned long blinkInterval = 500; // Blink rate: 500ms
bool ledState = false;

// Embedded Webpage HTML
const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>WeatherBoard AP</title>
  <style>
    :root {
      --bg: #0f172a;
      --card-bg: #1e293b;
      --text: #f8fafc;
      --text-muted: #94a3b8;
      --accent: #38bdf8;
      --card-border: #334155;
      --status-on: #22c55e;
      --status-off: #64748b;
    }
    * { box-sizing: border-box; margin: 0; padding: 0; font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Roboto, sans-serif; }
    body {
      background: var(--bg);
      color: var(--text);
      display: flex;
      flex-direction: column;
      align-items: center;
      min-height: 100vh;
      padding: 1.5rem 1rem;
    }
    .container {
      width: 100%;
      max-width: 440px;
      display: flex;
      flex-direction: column;
      gap: 1.25rem;
    }
    header {
      text-align: center;
      padding-bottom: 0.25rem;
    }
    h1 {
      font-size: 1.6rem;
      font-weight: 700;
      color: var(--accent);
      letter-spacing: -0.02em;
    }
    .badge {
      display: inline-block;
      margin-top: 0.35rem;
      padding: 0.25rem 0.65rem;
      background: #0369a1;
      border-radius: 9999px;
      font-size: 0.75rem;
      font-weight: 600;
      letter-spacing: 0.05em;
      text-transform: uppercase;
    }
    .card {
      background: var(--card-bg);
      border: 1px solid var(--card-border);
      border-radius: 1rem;
      padding: 1.25rem;
      box-shadow: 0 4px 6px -1px rgba(0, 0, 0, 0.3);
    }
    .metric-row {
      display: grid;
      grid-template-columns: 1fr 1fr;
      gap: 1rem;
    }
    .metric-card {
      display: flex;
      flex-direction: column;
      align-items: center;
      text-align: center;
    }
    .metric-label {
      font-size: 0.8rem;
      font-weight: 600;
      color: var(--text-muted);
      text-transform: uppercase;
      letter-spacing: 0.05em;
      margin-bottom: 0.4rem;
    }
    .metric-value {
      font-size: 2rem;
      font-weight: 700;
      color: var(--text);
    }
    .metric-sub {
      font-size: 0.85rem;
      color: var(--text-muted);
      margin-top: 0.2rem;
    }
    .led-control {
      display: flex;
      flex-direction: column;
      gap: 1rem;
      align-items: center;
      text-align: center;
    }
    .led-status {
      display: flex;
      align-items: center;
      gap: 0.5rem;
      font-size: 0.95rem;
      font-weight: 500;
    }
    .led-dot {
      width: 12px;
      height: 12px;
      border-radius: 50%;
      background: var(--status-off);
      transition: background 0.3s;
    }
    .led-dot.active {
      background: var(--status-on);
      box-shadow: 0 0 10px var(--status-on);
      animation: pulse 1s infinite alternate;
    }
    @keyframes pulse {
      from { opacity: 0.6; }
      to { opacity: 1; }
    }
    .btn {
      width: 100%;
      padding: 0.85rem 1.25rem;
      font-size: 1rem;
      font-weight: 600;
      border-radius: 0.75rem;
      border: none;
      cursor: pointer;
      transition: all 0.2s ease;
      background: #2563eb;
      color: white;
    }
    .btn:hover {
      background: #1d4ed8;
      transform: translateY(-1px);
    }
    .btn:active {
      transform: translateY(0);
    }
    .btn.active {
      background: #dc2626;
    }
    .btn.active:hover {
      background: #b91c1c;
    }
    .footer {
      text-align: center;
      font-size: 0.75rem;
      color: var(--text-muted);
    }
    .error-banner {
      display: none;
      background: #7f1d1d;
      color: #fecaca;
      border: 1px solid #dc2626;
      border-radius: 0.5rem;
      padding: 0.5rem;
      font-size: 0.8rem;
      text-align: center;
    }
  </style>
</head>
<body>
  <div class="container">
    <header>
      <h1>WeatherStation</h1>
      <span class="badge">Wi-Fi Access Point</span>
    </header>

    <div id="errorBanner" class="error-banner">Sensor read error. Checking DHT sensor...</div>

    <div class="card metric-row">
      <div class="metric-card">
        <span class="metric-label">Temperature</span>
        <div class="metric-value"><span id="tempC">--</span><span style="font-size:1.1rem;font-weight:500;">°C</span></div>
        <div class="metric-sub">(<span id="tempF">--</span>°F)</div>
      </div>
      <div class="metric-card">
        <span class="metric-label">Humidity</span>
        <div class="metric-value"><span id="humidity">--</span><span style="font-size:1.1rem;font-weight:500;">%</span></div>
        <div class="metric-sub">Relative</div>
      </div>
    </div>

    <div class="card led-control">
      <div class="led-status">
        <div id="ledDot" class="led-dot"></div>
        <span id="ledStatusText">LED Blinking: Disabled</span>
      </div>
      <button id="toggleBtn" class="btn" onclick="toggleBlink()">Start LED Blink</button>
    </div>

    <div class="footer">
      Connected to WeatherBoard AP (192.168.4.1)<br>Auto-updates every 2 seconds
    </div>
  </div>

  <script>
    let isBlinking = false;

    async function fetchData() {
      try {
        const res = await fetch('/data');
        if (!res.ok) return;
        const data = await res.json();

        const banner = document.getElementById('errorBanner');
        if (data.error) {
          banner.style.display = 'block';
        } else {
          banner.style.display = 'none';
          document.getElementById('tempC').textContent = data.temperatureC.toFixed(1);
          document.getElementById('tempF').textContent = data.temperatureF.toFixed(1);
          document.getElementById('humidity').textContent = data.humidity.toFixed(1);
        }
        updateBlinkUI(data.blinking);
      } catch (e) {
        console.error('Fetch error:', e);
      }
    }

    function updateBlinkUI(blinking) {
      isBlinking = blinking;
      const dot = document.getElementById('ledDot');
      const text = document.getElementById('ledStatusText');
      const btn = document.getElementById('toggleBtn');
      if (blinking) {
        dot.classList.add('active');
        text.textContent = 'LED Blinking: Active';
        btn.textContent = 'Stop LED Blink';
        btn.classList.add('active');
      } else {
        dot.classList.remove('active');
        text.textContent = 'LED Blinking: Disabled';
        btn.textContent = 'Start LED Blink';
        btn.classList.remove('active');
      }
    }

    async function toggleBlink() {
      const btn = document.getElementById('toggleBtn');
      btn.disabled = true;
      try {
        const res = await fetch('/toggle-blink', { method: 'POST' });
        const data = await res.json();
        updateBlinkUI(data.blinking);
      } catch (e) {
        console.error('Toggle error:', e);
      } finally {
        btn.disabled = false;
      }
    }

    fetchData();
    setInterval(fetchData, 2000);
  </script>
</body>
</html>
)rawliteral";

// Web server route handlers
void handleRoot() {
  server.send_P(200, "text/html", INDEX_HTML);
}

void handleData() {
  String json = "{";
  json += "\"temperatureC\":" + String(currentTempC, 1) + ",";
  json += "\"temperatureF\":" + String(currentTempF, 1) + ",";
  json += "\"humidity\":" + String(currentHumidity, 1) + ",";
  json += "\"blinking\":" + String(ledBlinking ? "true" : "false") + ",";
  json += "\"error\":" + String(sensorError ? "true" : "false");
  json += "}";
  server.send(200, "application/json", json);
}

void handleToggleBlink() {
  ledBlinking = !ledBlinking;
  Serial.printf("[WeatherBoard] LED Blinking toggled: %s\n", ledBlinking ? "ENABLED" : "DISABLED");
  String json = "{\"blinking\":" + String(ledBlinking ? "true" : "false") + "}";
  server.send(200, "application/json", json);
}

void handleNotFound() {
  // Redirect captive portal checks and unknown routes to root
  server.sendHeader("Location", "http://192.168.4.1/", true);
  server.send(302, "text/plain", "");
}

void updateLedBlink() {
  if (ledBlinking) {
    unsigned long currentMillis = millis();
    if (currentMillis - lastBlinkTime >= blinkInterval) {
      lastBlinkTime = currentMillis;
      ledState = !ledState;
      digitalWrite(LED_PIN, ledState ? HIGH : LOW);
    }
  } else {
    if (ledState) {
      ledState = false;
      digitalWrite(LED_PIN, LOW);
    }
  }
}

void drawDisplay(float tempC, float tempF, float humidity, bool hasError) {
  display.clearDisplay();

  // Header Banner with AP IP info
  display.fillRect(0, 0, SCREEN_WIDTH, 14, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(8, 3);
  display.print("AP: 192.168.4.1");

  display.setTextColor(SSD1306_WHITE);

  if (hasError) {
    display.setTextSize(1);
    display.setCursor(15, 28);
    display.print("Sensor Read Error!");
    display.setCursor(20, 42);
    display.print("Check DHT wiring");
  } else {
    // Temperature section
    display.setTextSize(1);
    display.setCursor(4, 20);
    display.print("TEMP:");

    display.setTextSize(2);
    display.setCursor(40, 18);
    display.print(tempC, 1);
    display.setTextSize(1);
    display.print(" C");

    display.setCursor(40, 34);
    display.print("(");
    display.print(tempF, 1);
    display.print(" F)");

    // Horizontal divider
    display.drawFastHLine(0, 46, SCREEN_WIDTH, SSD1306_WHITE);

    // Humidity section
    display.setTextSize(1);
    display.setCursor(4, 52);
    display.print("HUMID:");

    display.setTextSize(1);
    display.setCursor(48, 52);
    display.print(humidity, 1);
    display.print(" %");

    // LED Blinking indicator icon if active
    if (ledBlinking) {
      display.setCursor(95, 52);
      display.print("LED*");
    }

    // Heartbeat pulse indicator in lower right
    if (heartbeat) {
      display.fillCircle(123, 55, 2, SSD1306_WHITE);
    } else {
      display.drawCircle(123, 55, 2, SSD1306_WHITE);
    }
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[WeatherBoard] Starting up...");

  // Initialize LED
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  // Initialize I2C with defined pins
  Wire.begin(I2C_SDA, I2C_SCL);

  // Initialize OLED display
  if (!display.begin(SSD1306_SWITCHCAPVCC, SCREEN_ADDRESS)) {
    Serial.println("[WeatherBoard] ERROR: SSD1306 initialization failed!");
    for (;;); // Halt if OLED fails
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(6, 8);
  display.println("Starting WeatherBoard");
  display.setCursor(6, 22);
  display.println("SSID: WeatherBoard-AP");
  display.setCursor(6, 36);
  display.println("Pass: BlueJays");
  display.setCursor(6, 50);
  display.println("IP: 192.168.4.1");
  display.display();

  // Initialize DHT sensor
  dht.begin();

  // Initialize Wi-Fi in Access Point mode
  WiFi.mode(WIFI_AP);
  bool apStarted;
  if (strlen(apPassword) >= 8) {
    apStarted = WiFi.softAP(apSSID, apPassword);
  } else {
    apStarted = WiFi.softAP(apSSID);
  }

  if (apStarted) {
    Serial.println("[WeatherBoard] Wi-Fi AP started successfully.");
    Serial.print("[WeatherBoard] SSID: ");
    Serial.println(apSSID);
    Serial.print("[WeatherBoard] IP address: ");
    Serial.println(WiFi.softAPIP());
  } else {
    Serial.println("[WeatherBoard] ERROR: Failed to start Wi-Fi AP!");
  }

  // Start captive portal DNS server
  dnsServer.start(53, "*", WiFi.softAPIP());

  // Setup Web Server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/data", HTTP_GET, handleData);
  server.on("/toggle-blink", HTTP_ANY, handleToggleBlink);

  // Captive portal probes
  server.on("/generate_204", handleRoot);
  server.on("/fwlink", handleRoot);
  server.on("/hotspot-detect.html", handleRoot);
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("[WeatherBoard] Web server started.");

  delay(1000);
}

void loop() {
  dnsServer.processNextRequest();
  server.handleClient();
  updateLedBlink();

  unsigned long currentMillis = millis();
  if (currentMillis - lastReadTime >= readInterval || lastReadTime == 0) {
    lastReadTime = currentMillis;
    heartbeat = !heartbeat;

    float h = dht.readHumidity();
    float tC = dht.readTemperature();
    float tF = dht.readTemperature(true);

    if (isnan(h) || isnan(tC) || isnan(tF)) {
      Serial.println("[WeatherBoard] Warning: Failed to read from DHT sensor!");
      sensorError = true;
      drawDisplay(0, 0, 0, true);
    } else {
      sensorError = false;
      currentHumidity = h;
      currentTempC = tC;
      currentTempF = tF;
      Serial.printf("[WeatherBoard] Temp: %.1f C (%.1f F) | Humidity: %.1f %%\n",
                    currentTempC, currentTempF, currentHumidity);
      drawDisplay(currentTempC, currentTempF, currentHumidity, false);
    }
  }
}
