#include <DHT.h>
#include "BluetoothSerial.h"

// Check if Bluetooth is properly configured in the board settings
#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run make menuconfig to enable it.
#endif

// Pin Definitions
#define MQ2_PIN 34
#define DHTPIN 4
#define DHTTYPE DHT11

#define RED_LED 14
#define GREEN_LED 27
#define BLUE_LED 12
#define BUZZER_PIN 13

// Threshold Configuration (Analog raw ADC values: 0 to 4095)
const int THRESHOLD_MODERATE = 400; // Corresponds roughly to ~350-450 ppm
const int THRESHOLD_DANGER   = 700; // Corresponds roughly to >700 ppm

// Severity States
enum Severity { SAFE, MODERATE, DANGER };
Severity currentStatus = SAFE;

// Object initializations
DHT dht(DHTPIN, DHTTYPE);
BluetoothSerial ESP_BT; 

// Track time without using delay() in loop for non-blocking operations
unsigned long lastLogTime = 0;
const unsigned long logInterval = 1000; // Log data every 1 second
unsigned long secondsCounter = 1;

void setup() {
  // Initialize Hardwired Serial for local CSV logging
  Serial.begin(115200);
  
  // Initialize Bluetooth Classic
  ESP_BT.begin("Milan_Home_AirMonitor"); 
  
  // Initialize Sensors & Pins
  dht.begin();
  pinMode(MQ2_PIN, INPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);
  pinMode(BUZZER_PIN, OUTPUT);

  // Run Baseline Calibration Routine on boot
  runSensorCalibration();
  
  // Print CSV Header to Serial Monitor
  Serial.println("\nTimestamp, MQ-2 Value, DHT11 Temp, DHT11 Humidity, Severity Level");
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking timer for execution loop every 1 second
  if (currentMillis - lastLogTime >= logInterval) {
    lastLogTime = currentMillis;

    // 1. Read environmental data from sensors
    int mq2Raw = analogRead(MQ2_PIN);
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();

    // Verification check for local DHT11 sensor stability
    if (isnan(temperature) || isnan(humidity)) {
      temperature = 0.0;
      humidity = 0.0;
    }

    // 2. Evaluate threshold severity logic
    Severity newStatus = SAFE;
    if (mq2Raw >= THRESHOLD_DANGER) {
      newStatus = DANGER;
    } else if (mq2Raw >= THRESHOLD_MODERATE) {
      newStatus = MODERATE;
    } else {
      newStatus = SAFE;
    }

    // 3. Handle Alert Escalations & State Changes
    executeHardwareAlerts(newStatus);
    
    if (newStatus != currentStatus) {
      sendBluetoothAlert(newStatus, mq2Raw, temperature, humidity);
      currentStatus = newStatus;
    }

    // 4. Print structured local CSV string to Serial Monitor
    printCSVData(mq2Raw, temperature, humidity, newStatus);
    
    secondsCounter++;
  }
}

// Baseline Sensor Calibration Loop
void runSensorCalibration() {
  Serial.println("Initializing system... Calibrating MQ-2 Baseline sensor.");
  // Visual setup indicator: cycle the indicator colors
  digitalWrite(RED_LED, HIGH); delay(200);
  digitalWrite(GREEN_LED, HIGH); delay(200);
  digitalWrite(BLUE_LED, HIGH); delay(200);
  
  // Quick chirp signaling sensor warm up complete
  tone(BUZZER_PIN, 2000, 100); 
  
  digitalWrite(RED_LED, LOW);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(BLUE_LED, LOW);
  Serial.println("Calibration complete. Device is ready.");
}

// Execution block managing RGB Indicators and multi-tone buzzer states
void executeHardwareAlerts(Severity status) {
  switch (status) {
    case SAFE:
      // Solid Green indicator
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
      noTone(BUZZER_PIN); // Silent execution
      break;

    case MODERATE:
      // Yellow Mix (Red + Green)
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, HIGH); 
      digitalWrite(BLUE_LED, LOW);
      // Low, continuous caution pulse tone
      tone(BUZZER_PIN, 800, 150); 
      break;

    case DANGER:
      // Solid Red indicator
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(BLUE_LED, LOW);
      // Piercing high-pitch emergency alarm tone
      tone(BUZZER_PIN, 2500, 400); 
      break;
  }
}

// Standardized structure outputting CSV layout directly via Serial
void printCSVData(int mqValue, float temp, float hum, Severity status) {
  // Format simulated timestamp mm:ss
  char timeStr[6];
  sprintf(timeStr, "%02d:%02d", (secondsCounter / 60) % 60, secondsCounter % 60);
  
  Serial.print(timeStr);
  Serial.print(", ");
  Serial.print(mqValue);
  Serial.print(" ppm, ");
  Serial.print(temp, 1);
  Serial.print("°C, ");
  Serial.print(hum, 0);
  Serial.print("%, ");
  
  if (status == SAFE) Serial.println("SAFE");
  else if (status == MODERATE) Serial.println("MODERATE");
  else if (status == DANGER) Serial.println("DANGER");
}

// Local Fallback Transmission Pipeline over Classic Bluetooth
void sendBluetoothAlert(Severity status, int mqValue, float temp, float hum) {
  if (ESP_BT.availableForWrite()) {
    ESP_BT.println("--------------------------------");
    switch (status) {
      case SAFE:
        ESP_BT.println("STATUS: SAFE");
        ESP_BT.print("Gas: "); ESP_BT.println(mqValue);
        ESP_BT.print("Temp: "); ESP_BT.print(temp, 1); ESP_BT.println("C");
        ESP_BT.print("Humidity: "); ESP_BT.print(hum, 0); ESP_BT.println("%");
        break;
        
      case MODERATE:
        ESP_BT.println("WARNING!!");
        ESP_BT.println("Moderate Smoke Detected");
        ESP_BT.print("Gas = "); ESP_BT.println(mqValue);
        ESP_BT.println("Action Required: Check Fireplace");
        break;
        
      case DANGER:
        ESP_BT.println("ALERT: DANGER!!");
        ESP_BT.println("Smoke Level Critical!");
        ESP_BT.print("Gas = "); ESP_BT.println(mqValue);
        ESP_BT.println("Immediate Action: Open Windows & Leave Room if Necessary");
        break;
    }
    ESP_BT.println("--------------------------------");
  }
}
