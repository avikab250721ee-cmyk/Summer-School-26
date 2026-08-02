/*
 * Project: Servo Control with Potentiometer Mapping
 * Question: Q25
 * Author: [Shashwat Singh]
 */

#include <Servo.h>
#include <Arduino.h>

// Pin definitions
const int POT_PIN = A0;     // Potentiometer analog input
const int SERVO_PIN = 9;    // Servo PWM output (pin 9 supports PWM)
const int BUTTON_PIN = 8;   // Button for sweep trigger

// Servo object
Servo servo;

// State variables
bool sweep_active = false;
int sweep_angle = 0;
bool sweep_direction = 1;   // 0=decreasing, 1=increasing
unsigned long sweep_start_time = 0;

// Timing
const unsigned long SWEEP_SPEED = 20;  // ms per degree

void setup() {
  // Initialize pins
  pinMode(POT_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT);
  
  // Attach servo to pin 9
  servo.attach(SERVO_PIN);
  
  // Initialize Serial
  Serial.begin(9600);
  Serial.println("===== Servo Control Started =====");
  Serial.println("Potentiometer (A0): 0-1023 → Servo: 0-180°");
  Serial.println("Button (pin 8): Trigger sweep 0→180→0");
  Serial.println("===================================");
  
  // Set initial servo position
  servo.write(0);
  sweep_start_time = millis();
}

void loop() {
  unsigned long current_time = millis();
  
  // === READ POTENTIOMETER AND CONTROL SERVO ===
  int pot_reading = analogRead(POT_PIN);
  int servo_angle = map(pot_reading, 0, 1023, 0, 180);
  
  if (!sweep_active) {
    // Normal mode: potentiometer controls servo
    servo.write(servo_angle);
    
    // Print angle every 500ms
    static unsigned long last_print = 0;
    if ((current_time - last_print) >= 500) {
      last_print = current_time;
      Serial.print("Pot: ");
      Serial.print(pot_reading);
      Serial.print(" (0-1023) | Servo Angle: ");
      Serial.print(servo_angle);
      Serial.println("°");
    }
  }
  
  // === BUTTON DETECTION (Non-blocking sweep trigger) ===
  static bool button_pressed = false;
  static unsigned long button_press_time = 0;
  
  if (digitalRead(BUTTON_PIN) == HIGH && !button_pressed) {
    button_pressed = true;
    button_press_time = current_time;
  }
  
  if (button_pressed && (current_time - button_press_time) > 20) {
    if (digitalRead(BUTTON_PIN) == LOW) {
      // Button released - start sweep
      sweep_active = true;
      sweep_angle = servo_angle;  // Start from current position
      sweep_direction = 1;         // Go toward 180
      sweep_start_time = current_time;
      button_pressed = false;
      Serial.println("[SWEEP] Starting 0→180→0 sequence...");
    }
  }
  
  // === SWEEP EXECUTION (Non-blocking) ===
  if (sweep_active) {
    unsigned long elapsed = current_time - sweep_start_time;
    int target_angle = elapsed / SWEEP_SPEED;  // Increment angle
    
    if (sweep_direction == 1) {
      // Moving toward 180
      if (target_angle >= 180) {
        target_angle = 180;
        sweep_direction = 0;  // Change direction
        sweep_start_time = current_time;  // Reset timer
      }
    } else {
      // Moving back toward 0
      target_angle = 180 - (elapsed / SWEEP_SPEED);
      if (target_angle <= 0) {
        target_angle = 0;
        sweep_active = false;  // Sweep complete
        Serial.println("[SWEEP] Complete - back to potentiometer control");
      }
    }
    
    servo.write(target_angle);
  }
}
