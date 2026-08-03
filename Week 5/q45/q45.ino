#include <WiFi.h>
#include <PubSubClient.h>
#include <DHT.h>
#include <LiquidCrystal_I2C.h>

// Wi-Fi Network & MQTT Broker Configurations
const char* ssid         = "YOUR_WIFI_SSID";
const char* password     = "YOUR_WIFI_PASSWORD";
const char* mqtt_server  = "broker.hivemq.com"; // Open public broker for testing
const int mqtt_port      = 1883;
const char* mqtt_topic   = "kashmir/greenhouse/telemetry";

// Peripheral Pin Layout Definitions
#define DHTPIN 4
#define DHTTYPE DHT11
#define LDR_ANALOG_PIN 34

#define RELAY_HEATER    14 // Simulated via Red LED
#define RELAY_FAN       27 // Simulated via Blue LED
#define RELAY_GROWLIGHT 26 // Simulated via White/Yellow LED

// Environmental Setpoints & Hysteresis Deadbands
const float TEMP_LOW        = 18.0;
const float TEMP_HIGH       = 22.0;
const float HUMID_LOW       = 60.0;
const float HUMID_HIGH      = 70.0;
const float HYSTERESIS_TEMP  = 0.5; // Prevent relay jitter around thresholds
const float HYSTERESIS_HUMID = 1.0;

const int LIGHT_THRESHOLD   = 600;  // Analog baseline value indicating "insufficient light"

// LDR Running Average Window Configuration
const int FILTER_SAMPLES = 10;
int ldrSamples[FILTER_SAMPLES];
int ldrSampleIndex = 0;

// LCD Sequencer State Mapping
enum LcdDisplayScreen { DISPLAY_TEMP, DISPLAY_HUMID, DISPLAY_LIGHT };
LcdDisplayScreen activeScreen = DISPLAY_TEMP;

// Global Sensor Values
float currentTemp = 0.0;
float currentHumid = 0.0;
int averagedLight = 0;

// Relay Output States
bool heaterState = false;
bool fanState = false;
bool growLightState = false;

// Timing Variables (Non-blocking execution)
unsigned long lastSensorPoll = 0;
unsigned long lastScreenCycle = 0;
unsigned long lastMqttSync = 0;

const unsigned long SENSOR_INTERVAL = 2000; // Poll data every 2 seconds
const unsigned long SCREEN_INTERVAL = 3000; // Cycle LCD text screen every 3 seconds
const unsigned long MQTT_INTERVAL   = 5000; // Publish cloud telemetry every 5 seconds

DHT dht(DHTPIN, DHTTYPE);
LiquidCrystal_I2C lcd(0x27, 16, 2);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void setup() {
  Serial.begin(115200);
  
  dht.begin();
  lcd.init();
  lcd.backlight();

  pinMode(LDR_ANALOG_PIN, INPUT);
  pinMode(RELAY_HEATER, OUTPUT);
  pinMode(RELAY_FAN, OUTPUT);
  pinMode(RELAY_GROWLIGHT, OUTPUT);

  // Set default initial safe state (OFF)
  digitalWrite(RELAY_HEATER, LOW);
  digitalWrite(RELAY_FAN, LOW);
  digitalWrite(RELAY_GROWLIGHT, LOW);

  // Initialize rolling filter array buffer with baseline read values
  int baselineRead = analogRead(LDR_ANALOG_PIN);
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    ldrSamples[i] = baselineRead;
  }
  averagedLight = baselineRead;

  // Establish Wireless Connection Network Stack
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Network");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi Connected successfully.");

  mqttClient.setServer(mqtt_server, mqtt_port);
}

void loop() {
  unsigned long currentMillis = millis();

  // Maintain cloud connectivity channel
  if (!mqttClient.connected()) {
    reconnectMqttBroker();
  }
  mqttClient.loop();

  // 1. Non-Blocking Routine: Read Sensors & Run Bang-Bang Hysteresis Control Loop
  if (currentMillis - lastSensorPoll >= SENSOR_INTERVAL) {
    lastSensorPoll = currentMillis;
    
    // Process Climate Metrics
    float t = dht.readTemperature();
    float h = dht.readHumidity();
    if (!isnan(t) && !isnan(h)) {
      currentTemp = t;
      currentHumid = h;
    }

    // Process Rolling LDR Light Filter Window
    ldrSamples[ldrSampleIndex] = analogRead(LDR_ANALOG_PIN);
    ldrSampleIndex = (ldrSampleIndex + 1) % FILTER_SAMPLES;
    
    long totalSum = 0;
    for (int i = 0; i < FILTER_SAMPLES; i++) {
      totalSum += ldrSamples[i];
    }
    averagedLight = totalSum / FILTER_SAMPLES;

    executeClimateControlLogic();
  }

  // 2. Non-Blocking Routine: Sequential LCD Display Screen Engine
  if (currentMillis - lastScreenCycle >= SCREEN_INTERVAL) {
    lastScreenCycle = currentMillis;
    cycleLcdDashboard();
  }

  // 3. Non-Blocking Routine: Cloud MQTT Telemetry Publisher Sync
  if (currentMillis - lastMqttSync >= MQTT_INTERVAL) {
    lastMqttSync = currentMillis;
    publishTelemetryToCloud();
  }
}

// Bang-Bang Control Strategy implementing custom Hysteresis protection walls
void executeClimateControlLogic() {
  // Temperature Control (Target Profile: 18C to 22C)
  if (currentTemp < (TEMP_LOW - HYSTERESIS_TEMP)) {
    heaterState = true; // Actuate heating line if it drops too low
  } else if (currentTemp > (TEMP_LOW + HYSTERESIS_TEMP)) {
    // Keep off or turn off once it recovers safely into normal limits
    if(currentTemp >= TEMP_HIGH) heaterState = false; 
  }
  
  // Ventilation Fan Control (Target Profile: Under 70%)
  if (currentHumid > (HUMID_HIGH + HYSTERESIS_HUMID) || currentTemp > (TEMP_HIGH + HYSTERESIS_TEMP)) {
    fanState = true;   // Actuate exhaust lines if overly humid or hot
  } else if (currentHumid < (HUMID_HIGH - HYSTERESIS_HUMID) && currentTemp < (TEMP_HIGH - HYSTERESIS_TEMP)) {
    fanState = false;
  }

  // Supplemental Grow Light Control
  if (averagedLight < LIGHT_THRESHOLD) {
    growLightState = true;  // Sun is setting / cloud overcast present
  } else if (averagedLight > (LIGHT_THRESHOLD + 200)) {
    growLightState = false; // Sufficient daylight available
  }

  // Drive Hardware lines to matching output configurations
  digitalWrite(RELAY_HEATER, heaterState ? HIGH : LOW);
  digitalWrite(RELAY_FAN, fanState ? HIGH : LOW);
  digitalWrite(RELAY_GROWLIGHT, growLightState ? HIGH : LOW);
}

// Sequential 3-Second Dashboard Panel Rotator
void cycleLcdDashboard() {
  lcd.clear();
  lcd.setCursor(0, 0);
  
  switch (activeScreen) {
    case DISPLAY_TEMP:
      lcd.print("Greenhouse Temp");
      lcd.setCursor(0, 1);
      lcd.print("Value: "); lcd.print(currentTemp, 1); lcd.print(" C ");
      lcd.print(heaterState ? "[HTR]" : "[OFF]");
      activeScreen = DISPLAY_HUMID;
      break;

    case DISPLAY_HUMID:
      lcd.print("Greenhouse Humid");
      lcd.setCursor(0, 1);
      lcd.print("Value: "); lcd.print(currentHumid, 0); lcd.print(" % ");
      lcd.print(fanState ? "[FAN]" : "[OFF]");
      activeScreen = DISPLAY_LIGHT;
      break;

    case DISPLAY_LIGHT:
      lcd.print("Light RunningAvg");
      lcd.setCursor(0, 1);
      lcd.print("LDR Raw: "); lcd.print(averagedLight);
      lcd.print(growLightState ? " [LGT]" : " [OFF]");
      activeScreen = DISPLAY_TEMP;
      break;
  }
}

// Packages parameters into standard JSON format strings
void publishTelemetryToCloud() {
  if (mqttClient.connected()) {
    String payload = "{";
    payload += "\"temperature\":" + String(currentTemp, 1) + ",";
    payload += "\"humidity\":" + String(currentHumid, 0) + ",";
    payload += "\"ldr_avg\":" + String(averagedLight) + ",";
    payload += "\"heater\":" + String(heaterState ? 1 : 0) + ",";
    payload += "\"fan\":" + String(fanState ? 1 : 0) + ",";
    payload += "\"growlight\":" + String(growLightState ? 1 : 0);
    payload += "}";

    mqttClient.publish(mqtt_topic, payload.cString());
    Serial.print("MQTT Log [SYNC]: Sent data packet -> ");
    Serial.println(payload);
  }
}

void reconnectMqttBroker() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT Server Cloud Broker...");
    // Build an isolated random Client ID identifier
    String clientId = "ESP32GreenhouseClient-";
    clientId += String(random(0, 0xffff), HEX);
    
    if (mqttClient.connect(clientId.cString())) {
      Serial.println("Connected!");
    } else {
      Serial.print("Connection failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" Retrying sequence in 3 seconds...");
      delay(3000);
    }
  }
}
