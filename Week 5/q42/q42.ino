#include <ESP8266WiFi.h>
#include <ESP8266HTTPClient.h>
#include <WiFiClientSecure.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi Network Credentials
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// IFTTT Webhook Web Service Configuration
// Replace with your Webhook Key. Event Name: "saffron_irrigation"
const char* iftttWebhookUrl = "http://maker.ifttt.com/trigger/saffron_irrigation/with/key/YOUR_IFTTT_SECRET_KEY";

// Weather API Configuration (OpenWeatherMap)
// Saffron cultivation is centered around Pampore, J&K, India
const char* apiHost = "http://api.openweathermap.org/data/2.5/weather?q=Pampore,IN&appid=YOUR_OPENWEATHER_API_KEY&units=metric";

// Pin Configurations
#define SOIL_MOISTURE_PIN A0   // ESP8266 only has 1 analog pin (A0)
#define MULTIPLEX_SELECT  D0   // Controls an external switching transistor/relay if sharing A0, or general digital control
#define LDR_DIGITAL_PIN   D3   // Configured as digital input using an adjustable LDR comparator module
#define RELAY_PUMP_PIN    D5   // Pin tied to 5V relay module / Simulated by an LED
#define BUTTON_OVERRIDE   D6   // Tactile push button for farmer manual override

// System Threshold Zones (Calibrated for 10-bit ADC: 0 - 1023)
const int DRY_ZONE    = 750;   // Raw ADC value above this means soil is bone-dry
const int OPTIMAL_ZONE = 450;  // Raw ADC value between 450 and 750 is ideal moisture

// Timing Constants (Millis-based tracking)
const unsigned long SENSOR_INTERVAL = 1800000; // 30 minutes in milliseconds
const unsigned long LOCKOUT_INTERVAL = 300000;  // 5 minutes manual lockout in milliseconds
const unsigned long PUMP_RUN_DURATION = 15000;  // Run pump for 15 seconds when triggered

// State Variable Mappings
unsigned long lastSensorReadTime = 0;
unsigned long manualLockoutStartTime = 0;
unsigned long pumpStartTime = 0;
unsigned long lastWateredTimeMillis = 0;

bool isPumpActive = false;
bool inManualLockout = false;
String currentSystemMode = "AUTO_IDLE";
int currentMoisturePercent = 0;

// Initialize 16x2 LCD display at I2C address 0x27
LiquidCrystal_I2C lcd(0x27, 16, 2);

void setup() {
  Serial.begin(115200);
  
  // Peripheral Pin Layout Design
  pinMode(LDR_DIGITAL_PIN, INPUT);
  pinMode(BUTTON_OVERRIDE, INPUT_PULLUP);
  pinMode(RELAY_PUMP_PIN, OUTPUT);
  digitalWrite(RELAY_PUMP_PIN, LOW); // Ensure pump is OFF on boot

  // Initialize LCD Screen
  lcd.init();
  lcd.backlight();
  lcd.setCursor(0, 0);
  lcd.print("Saffron System");
  lcd.setCursor(0, 1);
  lcd.print("Initializing...");

  // Establish Wi-Fi Network Connectivity
  WiFi.begin(ssid, password);
  Serial.print("Connecting to network");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nConnected to Wi-Fi successfully!");
  
  lcd.clear();
  lcd.print("System Ready");
  delay(1500);

  // Initial read to populate screen variables instantly on startup
  evaluateIrrigationSystem();
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Core Routine: Check soil metrics every 30 minutes
  if (currentMillis - lastSensorReadTime >= SENSOR_INTERVAL || lastSensorReadTime == 0) {
    lastSensorReadTime = currentMillis;
    evaluateIrrigationSystem();
  }

  // 2. Button Routine: Continuously monitor Manual Override Button with 5-minute lockout check
  if (inManualLockout) {
    if (currentMillis - manualLockoutStartTime >= LOCKOUT_INTERVAL) {
      inManualLockout = false;
      currentSystemMode = "AUTO_IDLE";
      updateLCDDisplay();
    }
  } else {
    // Button is pressed when line pulled LOW (Internal Pullup configuration)
    if (digitalWrite(BUTTON_OVERRIDE) == LOW) {
      delay(50); // Software debounce
      if (digitalWrite(BUTTON_OVERRIDE) == LOW) {
        Serial.println("Farmer manual override activated!");
        triggerWaterPump();
        inManualLockout = true;
        manualLockoutStartTime = currentMillis;
        currentSystemMode = "MANUAL_LOCK";
        updateLCDDisplay();
      }
    }
  }

  // 3. Pump Runtime Routine: Handle non-blocking auto shutoff of water pump
  if (isPumpActive) {
    if (currentMillis - pumpStartTime >= PUMP_RUN_DURATION) {
      isPumpActive = false;
      digitalWrite(RELAY_PUMP_PIN, LOW);
      Serial.println("Watering cycle complete. Pump turned off.");
      updateLCDDisplay();
    }
  }
}

// System Analysis Function: Reads sensors, parses cloud data, and determines irrigation needs
void evaluateIrrigationSystem() {
  Serial.println("\n--- Starting Irrigation Assessment Cycle ---");
  
  // A. Read Soil Moisture Sensor Data
  int rawMoisture = analogRead(SOIL_MOISTURE_PIN);
  // Map 10-bit sensor value to an intuitive 0-100% scale (invert values because wet = low raw score)
  currentMoisturePercent = map(rawMoisture, 1023, 200, 0, 100);
  if (currentMoisturePercent < 0) currentMoisturePercent = 0;
  if (currentMoisturePercent > 100) currentMoisturePercent = 100;

  // B. Assess Local Rain Proxy (LDR Module)
  // Low Light or Overcast sky pulls digital comparator pin HIGH
  bool isOvercastLocal = (digitalRead(LDR_DIGITAL_PIN) == HIGH);

  // C. Query Cloud Weather API for Rainfall Data
  bool isRainPredictedCloud = checkCloudWeatherAPI();

  Serial.print("Soil Moisture: "); Serial.print(currentMoisturePercent); Serial.println("%");
  Serial.print("Local Rain Proxy (LDR Overcast): "); Serial.println(isOvercastLocal ? "YES" : "NO");
  Serial.print("Cloud API Rain Indicator: "); Serial.println(isRainPredictedCloud ? "YES" : "NO");

  // D. Multi-Factor Decision Matrix for Saffron Safety
  if (rawMoisture >= DRY_ZONE) {
    // Soil is dry, check rain prevention proxies to avoid damaging saffron bulbs
    if (isOvercastLocal || isRainPredictedCloud) {
      currentSystemMode = "RAIN_HOLD";
      Serial.println("Decision: Soil is DRY, but watering was aborted due to recent rain or incoming overcast conditions.");
    } else {
      currentSystemMode = "WATERING";
      triggerWaterPump();
    }
  } else if (rawMoisture < DRY_ZONE && rawMoisture >= OPTIMAL_ZONE) {
    currentSystemMode = "OPTIMAL";
    Serial.println("Decision: Soil moisture zone is optimal. No watering required.");
  } else {
    currentSystemMode = "WET_ALERT";
    Serial.println("Decision Warning: Soil is excessively WET! Potential overwatering detected.");
  }

  // E. Push System Log Data up to cloud Google Sheets via IFTTT API
  logDataToGoogleSheets(currentMoisturePercent, currentSystemMode);

  // F. Update the physical 16x2 user interface screen
  updateLCDDisplay();
}

// Drives hardware lines to actuate the pump relay and log system timing
void triggerWaterPump() {
  Serial.println("Actuating water pump relay pin...");
  digitalWrite(RELAY_PUMP_PIN, HIGH);
  pumpStartTime = millis();
  lastWateredTimeMillis = millis();
  isPumpActive = true;
}

// Contacts OpenWeatherMap endpoint to check local atmospheric conditions
bool checkCloudWeatherAPI() {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, apiHost);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
      String payload = http.getString();
      http.end();
      
      // Look for indicators of rain in the weather description text field
      if (payload.indexOf("\"main\":\"Rain\"") != -1 || payload.indexOf("\"description\":\"rain\"") != -1) {
        return true;
      }
    } else {
      Serial.print("Weather API HTTP Error Code: ");
      Serial.println(httpResponseCode);
      http.end();
    }
  }
  return false; // Fail-safe default: assume no rain if API request times out or breaks
}

// Fires Webhook POST string packet containing JSON telemetry payloads to IFTTT
void logDataToGoogleSheets(int moisture, String mode) {
  if (WiFi.status() == WL_CONNECTED) {
    WiFiClient client;
    HTTPClient http;
    
    http.begin(client, iftttWebhookUrl);
    http.addHeader("Content-Type", "application/json");
    
    // Package parameters into values accepted by general IFTTT applet configurations
    String jsonPayload = "{\"value1\":\"" + String(moisture) + "%\",\"value2\":\"" + mode + "\",\"value3\":\"" + String(millis()) + "\"}";
    
    int httpResponseCode = http.POST(jsonPayload);
    Serial.print("Cloud Sync logging response code: ");
    Serial.println(httpResponseCode);
    
    http.end();
  } else {
    Serial.println("Cloud logging failed: Wi-Fi connection down.");
  }
}

// Rewrites character lines onto the physical 16x2 LCD panel
void updateLCDDisplay() {
  lcd.clear();
  
  // Line 1: Output Moisture and current operating system status mode
  lcd.setCursor(0, 0);
  lcd.print("M:");
  lcd.print(currentMoisturePercent);
  lcd.print("% ");
  lcd.print("[");
  lcd.print(currentSystemMode);
  lcd.print("]");

  // Line 2: Convert relative system execution millis counter into hours/minutes
  lcd.setCursor(0, 1);
  if (lastWateredTimeMillis == 0) {
    lcd.print("Last W: Never");
  } else {
    unsigned long totalSeconds = lastWateredTimeMillis / 1000;
    unsigned long minutes = totalSeconds / 60;
    unsigned long hours = minutes / 60;
    
    lcd.print("Last W: ");
    lcd.print(hours);
    lcd.print("h ");
    lcd.print(minutes % 60);
    lcd.print("m ago");
  }
}
