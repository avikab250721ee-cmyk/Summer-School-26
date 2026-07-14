/*
 * Project: Digital Piano with Tone Generation
 * Question: Q15
 * Author: [Shashwat Singh]
 */

#include <Arduino.h>

// Pin definitions
const int BUTTON_C = 8;  // C note
const int BUTTON_D = 9;  // D note
const int BUTTON_E = 10; // E note
const int BUTTON_F = 11; // F note
const int BUZZER = 12;   // Piezo buzzer/speaker

// Frequency constants (Hz) - Musical notes
const int FREQ_C = 262;   // Middle C
const int FREQ_D = 294;   // D
const int FREQ_E = 330;   // E
const int FREQ_F = 349;   // F
const int FREQ_SOL = 392; // G/Sol (bonus: 2-button simultaneous)

// Mode tracking
bool major_mode = true;  // Toggle between Major and Minor modes

void setup() {
  // Initialize button pins as inputs
  pinMode(BUTTON_C, INPUT);
  pinMode(BUTTON_D, INPUT);
  pinMode(BUTTON_E, INPUT);
  pinMode(BUTTON_F, INPUT);
  
  // Initialize buzzer pin as output
  pinMode(BUZZER, OUTPUT);
  
  // Initialize Serial for debugging
  Serial.begin(9600);
  Serial.println("===== Digital Piano Started =====");
  Serial.println("Button C (pin 8)  → 262 Hz (C)");
  Serial.println("Button D (pin 9)  → 294 Hz (D)");
  Serial.println("Button E (pin 10) → 330 Hz (E)");
  Serial.println("Button F (pin 11) → 349 Hz (F)");
  Serial.println("2-Button Press    → 392 Hz (Sol/G)");
  Serial.println("Mode Toggle       → Major/Minor");
  Serial.println("====================================");
}

void loop() {
  // Read all button states (HIGH = pressed)
  bool c_pressed = digitalRead(BUTTON_C) == HIGH;
  bool d_pressed = digitalRead(BUTTON_D) == HIGH;
  bool e_pressed = digitalRead(BUTTON_E) == HIGH;
  bool f_pressed = digitalRead(BUTTON_F) == HIGH;
  
  // Count number of buttons pressed simultaneously
  int buttons_pressed = c_pressed + d_pressed + e_pressed + f_pressed;
  
  // Determine tone to play
  int frequency = 0;
  const char* note_name = "NONE";
  
  // Bonus: Check for simultaneous 2-button presses
  if (buttons_pressed >= 2) {
    // Play Sol (392Hz) for any 2+ button combination
    frequency = FREQ_SOL;
    note_name = "SOL (2-Button Bonus)";
    Serial.println("[BONUS] Simultaneous 2+ buttons → Playing SOL (392 Hz)");
  }
  // Single button presses
  else if (c_pressed) {
    frequency = FREQ_C;
    note_name = "C";
    Serial.println("[BUTTON_C] Playing C (262 Hz)");
  }
  else if (d_pressed) {
    frequency = FREQ_D;
    note_name = "D";
    Serial.println("[BUTTON_D] Playing D (294 Hz)");
  }
  else if (e_pressed) {
    frequency = FREQ_E;
    note_name = "E";
    Serial.println("[BUTTON_E] Playing E (330 Hz)");
  }
  else if (f_pressed) {
    frequency = FREQ_F;
    note_name = "F";
    Serial.println("[BUTTON_F] Playing F (349 Hz)");
  }
  else {
    // No buttons pressed, silence the buzzer
    noTone(BUZZER);
  }
  
  // Play tone if frequency is set
  if (frequency > 0) {
    tone(BUZZER, frequency);
  }
  
  // Mode toggle (Major vs Minor frequency adjustment)
  if (major_mode) {
    // Major mode uses standard frequencies (unchanged)
    // In Major mode, intervals follow the major scale pattern
  } else {
    // Minor mode would apply frequency shifts
    if (frequency == FREQ_E) frequency = 311; // E minor would be ~311Hz
  }
  
  // Small delay to debounce button readings
  delay(50);
}
