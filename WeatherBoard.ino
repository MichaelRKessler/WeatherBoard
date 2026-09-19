#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <DHT.h>

// Display settings
#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET    -1
#define SCREEN_ADDRESS 0x3C

// Pin assignments (matching diagram.json)
#define DHTPIN  4
#define DHTTYPE DHT22
#define I2C_SDA 21
#define I2C_SCL 22

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
DHT dht(DHTPIN, DHTTYPE);

unsigned long lastReadTime = 0;
const unsigned long readInterval = 2000; // DHT22 minimum read interval is 2 seconds
bool heartbeat = false;

void drawDisplay(float tempC, float tempF, float humidity, bool hasError) {
  display.clearDisplay();

  // Header Banner
  display.fillRect(0, 0, SCREEN_WIDTH, 14, SSD1306_WHITE);
  display.setTextColor(SSD1306_BLACK);
  display.setTextSize(1);
  display.setCursor(18, 3);
  display.print("WEATHER STATION");

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

    // Heartbeat pulse indicator in lower right
    if (heartbeat) {
      display.fillCircle(120, 55, 3, SSD1306_WHITE);
    } else {
      display.drawCircle(120, 55, 3, SSD1306_WHITE);
    }
  }

  display.display();
}

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("\n[WeatherBoard] Starting up...");

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
  display.setCursor(10, 20);
  display.println("Initializing...");
  display.setCursor(10, 34);
  display.println("Connecting sensor...");
  display.display();

  // Initialize DHT sensor
  dht.begin();
  Serial.println("[WeatherBoard] DHT22 & SSD1306 initialized successfully.");

  delay(1000);
}

void loop() {
  unsigned long currentMillis = millis();

  if (currentMillis - lastReadTime >= readInterval || lastReadTime == 0) {
    lastReadTime = currentMillis;
    heartbeat = !heartbeat;

    float humidity = dht.readHumidity();
    float tempC = dht.readTemperature();
    float tempF = dht.readTemperature(true);

    if (isnan(humidity) || isnan(tempC) || isnan(tempF)) {
      Serial.println("[WeatherBoard] Warning: Failed to read from DHT sensor!");
      drawDisplay(0, 0, 0, true);
    } else {
      Serial.printf("[WeatherBoard] Temp: %.1f C (%.1f F) | Humidity: %.1f %%\n",
                    tempC, tempF, humidity);
      drawDisplay(tempC, tempF, humidity, false);
    }
  }
}

