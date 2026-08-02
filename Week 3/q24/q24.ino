/*
 * Project: Multi-Sensor Logger (DHT11 + LDR + HC-SR04)
 * Question: Q24
 * Author: [Shashwat Singh]
 * Description: Reads temperature/humidity (DHT11), light level (LDR), and distance (HC-SR04)
 *              every 5 seconds. Outputs in standardized block format with proper labels.
 *              Temperature labels: "Extreme" (>35°C), "Normal" (15-35°C), "Cold" (<15°C)
 *              Light labels: "Bright" (>75%), "Dim" (25-75%), "Dark" (<25%)
 *              Requires: DHT library by Adafruit
 */

#include "DHT.h"
#include <Arduino.h>

// DHT sensor configuration
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pin definitions
const int LDR_PIN = A0;
const int HC_TRIG = 8;
const int HC_ECHO = 9;

// Timing
const unsigned long READ_INTERVAL = 5000;
unsigned long last_read_time = 0;

void setup() {
  // Initialize pins
  pinMode(LDR_PIN, INPUT);
  pinMode(HC_TRIG, OUTPUT);
  pinMode(HC_ECHO, INPUT);
  
  // Initialize DHT
  dht.begin();
  
  // Initialize Serial
  Serial.begin(9600);
  Serial.println("===== Multi-Sensor Logger Started =====");
  Serial.println("Sensors: DHT11 (temp/humidity) + LDR (light) + HC-SR04 (distance)");
  Serial.println("Interval: 5 seconds");
  Serial.println("======================================");
  
  last_read_time = millis();
}

void loop() {
  unsigned long current_time = millis();
  
  if ((current_time - last_read_time) >= READ_INTERVAL) {
    last_read_time = current_time;
    
    // === READ DHT11 ===
    float humidity = dht.readHumidity();
    float temp_c = dht.readTemperature();
    
    // === READ LDR ===
    int ldr_raw = analogRead(LDR_PIN);
    int light_percent = map(ldr_raw, 0, 1023, 0, 100);
    const char* light_label;
    if (light_percent > 75) {
      light_label = "Bright";
    } else if (light_percent >= 25) {
      light_label = "Dim";
    } else {
      light_label = "Dark";
    }
    
    // === READ HC-SR04 ===
    float distance = measure_distance();
    
    // === OUTPUT FORMATTED BLOCK ===
    Serial.println("=== SENSOR LOG ===");
    Serial.print("Time : ");
    Serial.print(current_time);
    Serial.println(" ms");
    
    if (!isnan(temp_c) && !isnan(humidity)) {
      Serial.print("Temp : ");
      Serial.print(temp_c, 1);
      Serial.print(" C | Humidity: ");
      Serial.print(humidity, 0);
      Serial.println("%");
    } else {
      Serial.println("Temp : ERROR");
    }
    
    Serial.print("Light : ");
    Serial.print(light_percent);
    Serial.print("% (");
    Serial.print(light_label);
    Serial.println(")");
    
    if (distance > 0) {
      Serial.print("Distance : ");
      Serial.print(distance, 1);
      Serial.println(" cm");
    } else {
      Serial.println("Distance : ERROR");
    }
    
    Serial.println("==================");
  }
}

// Helper function: Measure HC-SR04 distance
float measure_distance() {
  // Send 10µs pulse to trigger
  digitalWrite(HC_TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(HC_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(HC_TRIG, LOW);
  
  // Measure echo pulse duration
  unsigned long duration = pulseIn(HC_ECHO, HIGH, 30000);
  
  if (duration == 0) {
    return -1;  // Timeout/error
  }
  
  // Distance = (duration * 0.034) / 2
  return (duration * 0.034) / 2.0;
}
