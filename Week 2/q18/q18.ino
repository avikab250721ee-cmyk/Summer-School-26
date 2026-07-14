/*
 * Project: Vending Machine Finite State Machine
 * Question: Q18
 * Author: [Shashwat Singh]
 */

#include <Arduino.h>

// Pin definitions
const int BUTTON_INSERT = 2;   // Insert Coin button
const int BUTTON_SELECT = 3;   // Select Item button
const int BUTTON_CANCEL = 4;   // Cancel button

const int LED_IDLE = 5;        // LED for IDLE state
const int LED_INSERTING = 6;   // LED for COIN_INSERTED state
const int LED_DISPENSING = 7;  // LED for DISPENSING state

// FSM states
enum State {
  IDLE,
  COIN_INSERTED,
  ITEM_SELECTED,
  DISPENSING
};

// State variables
State current_state = IDLE;
unsigned long state_start_time = 0;
unsigned long button_press_times[3] = {0, 0, 0};
bool button_pressed[3] = {false, false, false};

// Dispensing timing
const unsigned long DISPENSING_DURATION = 2000; // 2 seconds to dispense

void setup() {
  // Initialize button pins as inputs
  pinMode(BUTTON_INSERT, INPUT);
  pinMode(BUTTON_SELECT, INPUT);
  pinMode(BUTTON_CANCEL, INPUT);
  
  // Initialize LED pins as outputs
  pinMode(LED_IDLE, OUTPUT);
  pinMode(LED_INSERTING, OUTPUT);
  pinMode(LED_DISPENSING, OUTPUT);
  
  // Initialize Serial for state tracking
  Serial.begin(9600);
  Serial.println("===== Vending Machine FSM Started =====");
  Serial.println("States: IDLE → COIN_INSERTED → ITEM_SELECTED → DISPENSING → IDLE");
  Serial.println("Buttons: INSERT_COIN (pin 2), SELECT_ITEM (pin 3), CANCEL (pin 4)");
  Serial.println("LEDs: IDLE (pin 5), INSERTING (pin 6), DISPENSING (pin 7)");
  Serial.println("========================================");
  
  // Set initial state
  current_state = IDLE;
  state_start_time = millis();
  update_leds();
  Serial.println("STATE: *** → IDLE");
}

void loop() {
  unsigned long current_time = millis();
  
  // Non-blocking button reading (3 buttons)
  read_buttons(current_time);
  
  // Check for CANCEL button press (can cancel from any state)
  if (button_pressed[2]) {
    button_pressed[2] = false;
    if (current_state != IDLE) {
      Serial.println("[CANCEL] Returning to IDLE");
      current_state = IDLE;
      state_start_time = current_time;
      update_leds();
    }
  }
  
  // FSM state machine
  switch (current_state) {
    case IDLE:
      if (button_pressed[0]) {
        // Insert Coin button pressed
        button_pressed[0] = false;
        current_state = COIN_INSERTED;
        state_start_time = current_time;
        update_leds();
        Serial.print("STATE: IDLE → COIN_INSERTED | Time: ");
        Serial.print(current_time);
        Serial.println(" ms");
      }
      break;
    
    case COIN_INSERTED:
      if (button_pressed[1]) {
        // Select Item button pressed
        button_pressed[1] = false;
        current_state = ITEM_SELECTED;
        state_start_time = current_time;
        update_leds();
        Serial.print("STATE: COIN_INSERTED → ITEM_SELECTED | Time: ");
        Serial.print(current_time);
        Serial.println(" ms");
      }
      break;
    
    case ITEM_SELECTED:
      // Automatically transition to DISPENSING
      current_state = DISPENSING;
      state_start_time = current_time;
      update_leds();
      Serial.print("STATE: ITEM_SELECTED → DISPENSING (auto) | Time: ");
      Serial.print(current_time);
      Serial.println(" ms");
      break;
    
    case DISPENSING:
      // Check if dispensing is complete (2 seconds elapsed)
      if ((current_time - state_start_time) >= DISPENSING_DURATION) {
        current_state = IDLE;
        state_start_time = current_time;
        update_leds();
        Serial.print("STATE: DISPENSING → IDLE (complete) | Time: ");
        Serial.print(current_time);
        Serial.println(" ms");
        Serial.println("[ITEM DISPENSED]");
      } else {
        // Show dispensing progress
        unsigned long elapsed = current_time - state_start_time;
        if (elapsed % 500 == 0) {
          Serial.print(".");
        }
      }
      break;
  }
}

// Non-blocking button reading with debounce
void read_buttons(unsigned long current_time) {
  // Button pins: 0=INSERT, 1=SELECT, 2=CANCEL
  int button_pins[3] = {BUTTON_INSERT, BUTTON_SELECT, BUTTON_CANCEL};
  
  for (int i = 0; i < 3; i++) {
    if (digitalRead(button_pins[i]) == HIGH && !button_pressed[i]) {
      // Button pressed, start debounce timer
      button_press_times[i] = current_time;
      button_pressed[i] = true;
    }
    else if (button_pressed[i] && (current_time - button_press_times[i] > 20)) {
      // Debounce complete, button press confirmed
      if (digitalRead(button_pins[i]) == LOW) {
        // Button released
        button_pressed[i] = false;
      }
    }
  }
}

// Update LEDs based on current state
void update_leds() {
  // Turn off all LEDs first
  digitalWrite(LED_IDLE, LOW);
  digitalWrite(LED_INSERTING, LOW);
  digitalWrite(LED_DISPENSING, LOW);
  
  // Turn on LED corresponding to current state
  switch (current_state) {
    case IDLE:
      digitalWrite(LED_IDLE, HIGH);
      Serial.println("[LED] IDLE light ON");
      break;
    case COIN_INSERTED:
      digitalWrite(LED_INSERTING, HIGH);
      Serial.println("[LED] INSERTING light ON");
      break;
    case ITEM_SELECTED:
      digitalWrite(LED_INSERTING, HIGH);
      Serial.println("[LED] INSERTING light ON");
      break;
    case DISPENSING:
      digitalWrite(LED_DISPENSING, HIGH);
      Serial.println("[LED] DISPENSING light ON");
      break;
  }
}
