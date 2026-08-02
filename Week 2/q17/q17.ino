/*
 * Project: PWM Night Light with Breathing Modes
 * Question: Q17
 * Author: [Shashwat Singh]
 */

#include <Arduino.h>

// Pin definitions
const int LED_PIN = 9;          // PWM pin 
const int MODE_BUTTON = 8;      // Button to cycle modes

// Enum for breathing modes
enum Mode {
  SLOW_BREATHING,
  FAST_PULSE,
  SOS_PATTERN
};

// State variables
Mode current_mode = SLOW_BREATHING;
unsigned long mode_start_time = 0;
unsigned long button_press_time = 0;
bool button_pressed = false;
int sos_index = 0;  // Index for SOS pattern
unsigned long sos_start_time = 0;

// Timing constants (milliseconds)
const unsigned long SLOW_BREATH_CYCLE = 3000;   // 3 seconds full cycle
const unsigned long FAST_PULSE_CYCLE = 500;     // 0.5 seconds full cycle
const unsigned long SOS_DOT_DURATION = 200;     // Dot duration
const unsigned long SOS_DASH_DURATION = 600;    // Dash duration (3x dot)
const unsigned long SOS_SPACE_DURATION = 200;   // Space between symbols
const unsigned long SOS_LETTER_SPACE = 600;     // Space between letters

// SOS pattern: 0=dot, 1=dash
const int SOS_PATTERN_LENGTH = 9;
const int sos_pattern[SOS_PATTERN_LENGTH] = {
  0, 0, 0,           // S (...)
  1, 1, 1,           // O (---)
  0, 0, 0            // S (...)
};

void setup() {
  // Initialize LED pin as PWM output
  pinMode(LED_PIN, OUTPUT);
  
  // Initialize mode button
  pinMode(MODE_BUTTON, INPUT);
  
  // Initialize Serial for debugging
  Serial.begin(9600);
  Serial.println("===== PWM Night Light Started =====");
  Serial.println("Mode 0: Slow Breathing (3s cycle)");
  Serial.println("Mode 1: Fast Pulse (0.5s cycle)");
  Serial.println("Mode 2: SOS Pattern");
  Serial.println("Press button to cycle modes");
  Serial.println("====================================");
  
  // Set initial time
  mode_start_time = millis();
  print_mode_change();
}

void loop() {
  unsigned long current_time = millis();
  
  // Non-blocking button reading
  if (digitalRead(MODE_BUTTON) == HIGH && !button_pressed) {
    button_pressed = true;
    button_press_time = current_time;
  }
  
  // Debounce and detect button release
  if (button_pressed && (current_time - button_press_time > 50)) {
    if (digitalRead(MODE_BUTTON) == LOW) {
      // Button released, cycle to next mode
      current_mode = (Mode)((current_mode + 1) % 3);
      mode_start_time = current_time;
      sos_index = 0;
      sos_start_time = current_time;
      button_pressed = false;
      print_mode_change();
    }
  }
  
  // Execute current mode
  switch (current_mode) {
    case SLOW_BREATHING:
      breathing_mode(current_time, SLOW_BREATH_CYCLE);
      break;
    
    case FAST_PULSE:
      breathing_mode(current_time, FAST_PULSE_CYCLE);
      break;
    
    case SOS_PATTERN:
      sos_mode(current_time);
      break;
  }
}

// Breathing animation using PWM
void breathing_mode(unsigned long current_time, unsigned long cycle_time) {
  // Calculate elapsed time in current cycle
  unsigned long elapsed = (current_time - mode_start_time) % cycle_time;
  
  // Normalize to 0-1 range
  float phase = (float)elapsed / cycle_time;
  
  // Simple triangular wave: 0→255→0 across the cycle
  int brightness;
  if (phase < 0.5) {
    // Fade in: 0 to 255
    brightness = (int)(phase * 2 * 255);
  } else {
    // Fade out: 255 to 0
    brightness = (int)((1 - phase) * 2 * 255);
  }
  
  // Apply PWM to LED
  analogWrite(LED_PIN, brightness);
}

// SOS pattern: ... --- ...
void sos_mode(unsigned long current_time) {
  unsigned long elapsed = current_time - sos_start_time;
  
  // Calculate position in SOS sequence
  unsigned long symbol_duration = SOS_DOT_DURATION;  // Will be updated based on dot/dash
  unsigned long cumulative_time = 0;
  
  // Iterate through SOS pattern
  for (int i = 0; i < SOS_PATTERN_LENGTH; i++) {
    symbol_duration = (sos_pattern[i] == 0) ? SOS_DOT_DURATION : SOS_DASH_DURATION;
    
    // Add symbol time + space after symbol
    unsigned long symbol_end = cumulative_time + symbol_duration;
    unsigned long space_end = symbol_end + SOS_SPACE_DURATION;
    
    if (elapsed < symbol_end) {
      // Inside a symbol (dot or dash)
      analogWrite(LED_PIN, 255);  // LED ON
      return;
    } else if (elapsed < space_end) {
      // Inside a space after symbol
      analogWrite(LED_PIN, 0);    // LED OFF
      return;
    }
    
    cumulative_time = space_end;
  }
  
  // Add letter space between SOS repeats
  if (elapsed < cumulative_time + SOS_LETTER_SPACE) {
    analogWrite(LED_PIN, 0);  // LED OFF during letter space
  } else {
    // Restart pattern
    sos_start_time = current_time;
  }
}

// Print mode change to Serial
void print_mode_change() {
  Serial.print("[MODE CHANGE] Current Mode: ");
  switch (current_mode) {
    case SLOW_BREATHING:
      Serial.println("SLOW_BREATHING (3s cycle)");
      break;
    case FAST_PULSE:
      Serial.println("FAST_PULSE (0.5s cycle)");
      break;
    case SOS_PATTERN:
      Serial.println("SOS_PATTERN (...---...)");
      break;
  }
  Serial.print("Time: ");
  Serial.print(millis());
  Serial.println(" ms");
}
