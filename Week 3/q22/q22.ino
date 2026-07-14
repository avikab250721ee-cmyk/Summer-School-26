/*
 * Project: HC-SR04 Ultrasonic Parking Sensor
 * Question: Q22
 * Author: [Shashwat Singh]
 * Description: Measures distance using HC-SR04 ultrasonic sensor.
 *              Distance = (duration * 0.034) / 2 cm
 *              Thresholds: >50cm SAFE, 20-50cm CAUTION (yellow + beep 500ms),
 *              10-20cm WARNING (red + beep 200ms), <10cm DANGER (flash + continuous beep).
 *              BONUS: Uses 100% non-blocking millis() timing (ZERO delay() calls).
 */

#include <Arduino.h>

// Pin definitions
const int TRIG_PIN = 8;    // Trigger pin (OUTPUT)
const int ECHO_PIN = 9;    // Echo pin (INPUT)
const int GREEN_LED = 10;  // Safe indicator
const int YELLOW_LED = 11; // Caution indicator
const int RED_LED = 12;    // Warning indicator
const int BUZZER = 7;      // Piezo buzzer

// Timing constants
const unsigned long MEASUREMENT_INTERVAL = 100;  // Read distance every 100ms
const unsigned long YELLOW_BEEP_INTERVAL = 500;  // Beep every 500ms in caution zone
const unsigned long RED_BEEP_INTERVAL = 200;     // Beep every 200ms in warning zone
const unsigned long DANGER_FLASH_INTERVAL = 100; // Flash every 100ms in danger zone

// Timing variables
unsigned long last_measurement_time = 0;
unsigned long last_yellow_beep_time = 0;
unsigned long last_red_beep_time = 0;
unsigned long last_danger_flash_time = 0;
unsigned long echo_start_time = 0;
bool measuring = false;
bool danger_led_state = false;

// Distance tracking
float current_distance = 0;

void setup() {
  // Initialize pins
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  
  // Initialize Serial
  Serial.begin(9600);
  Serial.println("===== HC-SR04 Ultrasonic Parking Sensor Started =====");
  Serial.println("Formula: distance = (duration * 0.034) / 2 cm");
  Serial.println("Thresholds:");
  Serial.println("  >50cm  : SAFE (Green LED)");
  Serial.println("  20-50cm: CAUTION (Yellow LED + beep every 500ms)");
  Serial.println("  10-20cm: WARNING (Red LED + beep every 200ms)");
  Serial.println("  <10cm  : DANGER (All LEDs flash + continuous beep)");
  Serial.println("Bonus: 100% non-blocking millis() timing (no delay() calls)");
  Serial.println("=====================================================");
}

void loop() {
  unsigned long current_time = millis();
  
  // === MEASUREMENT STAGE (Non-blocking trigger) ===
  if ((current_time - last_measurement_time) >= MEASUREMENT_INTERVAL) {
    last_measurement_time = current_time;
    
    if (!measuring) {
      // Start measurement: send 10µs pulse to trigger
      digitalWrite(TRIG_PIN, HIGH);
      delayMicroseconds(10);
      digitalWrite(TRIG_PIN, LOW);
      measuring = true;
      echo_start_time = current_time;
    } else {
      // Check if echo pulse returned (timeout after 100ms)
      if ((current_time - echo_start_time) > 100) {
        measuring = false;
        current_distance = -1; // Error code
      }
    }
  }
  
  // === READ ECHO (Interrupt-friendly check) ===
  if (measuring && digitalRead(ECHO_PIN) == LOW) {
    // Echo pulse completed, calculate distance
    unsigned long pulse_duration = current_time - echo_start_time;
    
    // Distance formula: (time * 0.034) / 2
    // 0.034 = speed of sound (34000 cm/s) / 1,000,000 µs
    current_distance = (pulse_duration * 0.034) / 2.0;
    
    measuring = false;
    
    // Debug output
    Serial.print("Distance: ");
    Serial.print(current_distance, 1);
    Serial.println(" cm");
  }
  
  // === LED & BUZZER CONTROL (State-based, non-blocking) ===
  if (current_distance > 0) {
    if (current_distance > 50) {
      // SAFE
      digitalWrite(GREEN_LED, HIGH);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, LOW);
      digitalWrite(BUZZER, LOW);
    }
    else if (current_distance >= 20 && current_distance <= 50) {
      // CAUTION
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, HIGH);
      digitalWrite(RED_LED, LOW);
      
      // Beep every 500ms
      if ((current_time - last_yellow_beep_time) >= YELLOW_BEEP_INTERVAL) {
        last_yellow_beep_time = current_time;
        tone(BUZZER, 1000, 100);  // 1000Hz for 100ms
      }
    }
    else if (current_distance >= 10 && current_distance < 20) {
      // WARNING
      digitalWrite(GREEN_LED, LOW);
      digitalWrite(YELLOW_LED, LOW);
      digitalWrite(RED_LED, HIGH);
      
      // Beep every 200ms
      if ((current_time - last_red_beep_time) >= RED_BEEP_INTERVAL) {
        last_red_beep_time = current_time;
        tone(BUZZER, 1500, 100);  // 1500Hz for 100ms
      }
    }
    else if (current_distance < 10) {
      // DANGER
      // All LEDs flash every 100ms
      if ((current_time - last_danger_flash_time) >= DANGER_FLASH_INTERVAL) {
        last_danger_flash_time = current_time;
        danger_led_state = !danger_led_state;
      }
      
      digitalWrite(GREEN_LED, danger_led_state);
      digitalWrite(YELLOW_LED, danger_led_state);
      digitalWrite(RED_LED, danger_led_state);
      
      // Continuous buzzer
      digitalWrite(BUZZER, HIGH);
    }
  }
}
