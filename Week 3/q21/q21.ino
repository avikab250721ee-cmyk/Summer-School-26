/*
 * Project: DHT11 Weather Station
 * Question: Q21
 * Author: [Shashwat Singh]
 * Requires: DHT library by Adafruit (install via Arduino IDE Library Manager)
 */

#include "DHT.h"
#include <Arduino.h>

// DHT sensor configuration
#define DHTPIN 2           // Data pin for DHT11
#define DHTTYPE DHT11      // Sensor type
DHT dht(DHTPIN, DHTTYPE);

// LED pins
const int RED_LED = 5;
const int GREEN_LED = 6;

// Timing
const unsigned long READ_INTERVAL = 2000; // Read every 2 seconds
unsigned long last_read_time = 0;

void setup() {
  // Initialize LED pins
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Initialize DHT sensor
  dht.begin();
  
  // Initialize Serial
  Serial.begin(9600);
  Serial.println("===== DHT11 Weather Station Started =====");
  Serial.println("Library: DHT by Adafruit (version 1.4.4+)");
  Serial.println("Sensor: DHT11 (Temperature & Humidity)");
  Serial.println("Reading interval: 2 seconds");
  Serial.println("CSV Format: timestamp,temp_C,temp_F,humidity");
  Serial.println("==========================================");
  Serial.println("timestamp,temp_C,temp_F,humidity");
  
  // Initial read
  last_read_time = millis();
}

void loop() {
  unsigned long current_time = millis();
  
  // Non-blocking timing check
  if ((current_time - last_read_time) >= READ_INTERVAL) {
    last_read_time = current_time;
    
    // Read humidity and temperature
    float humidity = dht.readHumidity();
    float temp_c = dht.readTemperature();     // Celsius
    float temp_f = dht.readTemperature(true); // Fahrenheit
    
    // Check if readings are valid
    if (isnan(humidity) || isnan(temp_c) || isnan(temp_f)) {
      Serial.println("ERROR: Failed to read from DHT11 sensor!");
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
      return;
    }
    
    // Output CSV format
    Serial.print(current_time);
    Serial.print(",");
    Serial.print(temp_c, 1); // 1 decimal place
    Serial.print(",");
    Serial.print(temp_f, 1);
    Serial.print(",");
    Serial.println(humidity, 0); // No decimal for humidity %
    
    // LED control: Red if extreme conditions, Green otherwise
    bool extreme = (temp_c > 35.0) || (humidity > 80.0);
    
    if (extreme) {
      digitalWrite(RED_LED, HIGH);
      digitalWrite(GREEN_LED, LOW);
    } else {
      digitalWrite(RED_LED, LOW);
      digitalWrite(GREEN_LED, HIGH);
    }
  }
}
