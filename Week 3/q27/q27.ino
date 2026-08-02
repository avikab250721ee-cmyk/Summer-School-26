/*
 * Project: Relay Control
 * Question: Q27
 * Author: [Shashwat Singh]
 */

#include "DHT.h"
#include <Arduino.h>

// DHT sensor configuration
#define DHTPIN 2
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

// Pin definitions
const int RELAY_PIN = 5;           // Digital output for relay (simulated by LED)
const int OVERRIDE_BUTTON = 3;     // Manual override button

// Hysteresis thresholds
const float RELAY_ON_THRESHOLD = 32.0;   // Turn ON above this
const float RELAY_OFF_THRESHOLD = 28.0;  // Turn OFF below this
                                         // Dead zone: 28-32°C (prevents chatter)

// Timing
const unsigned long TEMP_READ_INTERVAL = 2000;  // Read every 2 seconds
const unsigned long LOG_INTERVAL = 5000;        // Log status every 5 seconds

unsigned long last_temp_read_time = 0;
unsigned long last_log_time = 0;

// State variables
bool relay_state = false;       // false=OFF, true=ON
bool manual_override = false;
float last_temp = 0;

void setup() {
  // Initialize pins
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(OVERRIDE_BUTTON, INPUT);
  
  // Initialize DHT
  dht.begin();
  
  // Initialize Serial
  Serial.begin(9600);
  Serial.println("===== Relay Control with Hysteresis Started =====");
  Serial.println("Hysteresis: ON>32°C, OFF<28°C (dead zone prevents chatter)");
  Serial.println("Button: Manual override");
  Serial.println("====================================================");
  
  // Initial state
  relay_state = false;
  digitalWrite(RELAY_PIN, LOW);
  
  last_temp_read_time = millis();
  last_log_time = millis();
}

void loop() {
  unsigned long current_time = millis();
  
  // === READ TEMPERATURE (Non-blocking) ===
  if ((current_time - last_temp_read_time) >= TEMP_READ_INTERVAL) {
    last_temp_read_time = current_time;
    
    float temp_c = dht.readTemperature();
    
    if (!isnan(temp_c)) {
      last_temp = temp_c;
      
      // Hysteresis logic
      if (!relay_state && temp_c > RELAY_ON_THRESHOLD) {
        // Temperature crossed ON threshold
        relay_state = true;
        digitalWrite(RELAY_PIN, HIGH);
        log_relay_change("ON", temp_c);
      }
      else if (relay_state && temp_c < RELAY_OFF_THRESHOLD) {
        // Temperature crossed OFF threshold
        relay_state = false;
        digitalWrite(RELAY_PIN, LOW);
        log_relay_change("OFF", temp_c);
      }
      // If between 28-32°C, relay stays in current state (hysteresis dead zone)
    }
  }
  
  // === READ OVERRIDE BUTTON (Non-blocking) ===
  static bool button_pressed = false;
  static unsigned long button_press_time = 0;
  
  if (digitalRead(OVERRIDE_BUTTON) == HIGH && !button_pressed) {
    button_pressed = true;
    button_press_time = current_time;
  }
  
  if (button_pressed && (current_time - button_press_time) > 50) {
    if (digitalRead(OVERRIDE_BUTTON) == LOW) {
      // Button released
      relay_state = !relay_state;
      digitalWrite(RELAY_PIN, relay_state ? HIGH : LOW);
      manual_override = true;
      log_relay_change(relay_state ? "ON (OVERRIDE)" : "OFF (OVERRIDE)", last_temp);
      button_pressed = false;
    }
  }
  
  // === PERIODIC STATUS LOG ===
  if ((current_time - last_log_time) >= LOG_INTERVAL) {
    last_log_time = current_time;
    
    Serial.print("[STATUS] Temp: ");
    Serial.print(last_temp, 1);
    Serial.print("°C | Relay: ");
    Serial.print(relay_state ? "ON" : "OFF");
    Serial.print(" | Manual Override: ");
    Serial.println(manual_override ? "YES" : "NO");
  }
}

// Helper function: log relay state change
void log_relay_change(const char* new_state, float temperature) {
  Serial.print("[RELAY CHANGE] State: ");
  Serial.print(new_state);
  Serial.print(" | Temperature: ");
  Serial.print(temperature, 1);
  Serial.println("°C");
}
