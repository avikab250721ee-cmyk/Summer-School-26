/*
 * Project: Smart Street Light with LDR and PIR Motion Detection
 * Question: Q23
 * Author: [Shashwat Singh]
 */

#include <Arduino.h>

// Pin definitions
const int LDR_PIN = A0;       // Analog input for light sensor
const int PIR_PIN = 3;        // Digital input for motion sensor
const int LED_PIN = 9;        // PWM output for brightness control (pin 9 supports PWM)

// Thresholds
const int LDR_DAY_THRESHOLD = 600;      // LDR reading above this = day
const int LDR_NIGHT_THRESHOLD = 400;    // LDR reading below this = night (hysteresis)

// Timing constants
const unsigned long MOTION_TIMEOUT = 30000;      // 30 seconds
const unsigned long PIR_DEBOUNCE = 100;          // 100ms debounce
const unsigned long LOG_INTERVAL = 5000;         // Log status every 5 seconds

// Brightness levels
const int FULL_BRIGHTNESS = 255;                 // 100%
const int DIM_BRIGHTNESS = 51;                   // 20% (255 * 0.2)
const int OFF_BRIGHTNESS = 0;                    // 0%

// State variables
bool is_night = false;
bool last_motion = false;
unsigned long motion_detected_time = 0;
unsigned long last_pir_read_time = 0;
unsigned long last_log_time = 0;
bool led_full_brightness = false;

void setup() {
  // Initialize pins
  pinMode(LDR_PIN, INPUT);
  pinMode(PIR_PIN, INPUT);
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize Serial
  Serial.begin(9600);
  Serial.println("===== Smart Street Light Started =====");
  Serial.println("LDR Thresholds: Day>600, Night<400");
  Serial.println("PIR Motion Detection with 30s timeout");
  Serial.println("Brightness: 100% (motion) -> 20% (idle) -> 0% (day)");
  Serial.println("Events logged with HH:MM:SS timestamp");
  Serial.println("======================================");
  
  last_log_time = millis();
}

void loop() {
  unsigned long current_time = millis();
  
  // === READ LDR (Day/Night Detection) ===
  int ldr_reading = analogRead(LDR_PIN);
  bool new_is_night = false;
  
  if (is_night && ldr_reading > LDR_DAY_THRESHOLD) {
    // Transition from night to day
    is_night = false;
    log_event("[DAY] Daylight detected - LED OFF", current_time);
  }
  else if (!is_night && ldr_reading < LDR_NIGHT_THRESHOLD) {
    // Transition from day to night
    is_night = true;
    log_event("[NIGHT] Darkness detected - LED ready for motion", current_time);
  }
  
  // === READ PIR MOTION SENSOR (with debounce) ===
  if ((current_time - last_pir_read_time) >= PIR_DEBOUNCE) {
    last_pir_read_time = current_time;
    
    bool motion = digitalRead(PIR_PIN) == HIGH;
    
    if (motion && !last_motion && is_night) {
      // Motion detected (transition from no motion to motion)
      motion_detected_time = current_time;
      led_full_brightness = true;
      log_event("[MOTION] Detected - LED 100% brightness", current_time);
    }
    
    last_motion = motion;
  }
  
  // === BRIGHTNESS CONTROL ===
  if (is_night) {
    // Night mode: brightness depends on motion timeout
    if (led_full_brightness && (current_time - motion_detected_time) < MOTION_TIMEOUT) {
      // Motion active: full brightness
      analogWrite(LED_PIN, FULL_BRIGHTNESS);
    }
    else if (led_full_brightness && (current_time - motion_detected_time) >= MOTION_TIMEOUT) {
      // Motion timeout: dim to 20%
      led_full_brightness = false;
      analogWrite(LED_PIN, DIM_BRIGHTNESS);
      log_event("[IDLE] No motion for 30s - LED dimmed to 20%", current_time);
    }
    else if (!led_full_brightness) {
      // Remain dimmed
      analogWrite(LED_PIN, DIM_BRIGHTNESS);
    }
  }
  else {
    // Day mode: LED off
    analogWrite(LED_PIN, OFF_BRIGHTNESS);
  }
  
  // === PERIODIC STATUS LOG ===
  if ((current_time - last_log_time) >= LOG_INTERVAL) {
    last_log_time = current_time;
    Serial.print("[STATUS] ");
    print_time(current_time);
    Serial.print(" | LDR: ");
    Serial.print(ldr_reading);
    Serial.print(" | Night: ");
    Serial.print(is_night ? "YES" : "NO");
    Serial.print(" | Motion: ");
    Serial.print(last_motion ? "YES" : "NO");
    Serial.print(" | Brightness: ");
    Serial.println(led_full_brightness ? "100%" : (is_night ? "20%" : "0%"));
  }
}

// Helper function: format millis() into HH:MM:SS
void print_time(unsigned long ms) {
  unsigned long total_seconds = ms / 1000;
  unsigned int hours = (total_seconds / 3600) % 24;
  unsigned int minutes = (total_seconds / 60) % 60;
  unsigned int seconds = total_seconds % 60;
  
  if (hours < 10) Serial.print("0");
  Serial.print(hours);
  Serial.print(":");
  if (minutes < 10) Serial.print("0");
  Serial.print(minutes);
  Serial.print(":");
  if (seconds < 10) Serial.print("0");
  Serial.print(seconds);
}

// Helper function: log event with timestamp
void log_event(const char* event, unsigned long current_time) {
  Serial.print("[");
  print_time(current_time);
  Serial.print("] ");
  Serial.println(event);
}
