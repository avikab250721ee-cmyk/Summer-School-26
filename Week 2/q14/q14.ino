# Q14. 3-LED Traffic Light with Pedestrian Button (FSM)

## Hardware Setup & Pin Mapping

| Component | Arduino Pin (Old) | Arduino Pin (Updated) | Type |
| :--- | :---: | :---: | :--- |
| **Red LED** | 5 | **12** | Output |
| **Yellow LED** | 6 | **11** | Output |
| **Green LED** | 7 | **10** | Output |
| **Pedestrian Button** | 2 | **3** | Input |

---

## Code Implementation

```cpp
/*
 * Project: 3-LED Traffic Light with Pedestrian Button
 * Question: Q14
 * Author: [Shashwat Singh]
 */

#include <Arduino.h>

// Pin definitions (Updated Configuration)
const int RED_LED = 12;
const int YELLOW_LED = 11;
const int GREEN_LED = 10;
const int PEDESTRIAN_BUTTON = 3;

// FSM states
enum State {
  RED,
  YELLOW,
  GREEN
};

// State variables
State current_state = RED;
unsigned long state_start_time = 0;
unsigned long pedestrian_press_time = 0;
bool pedestrian_pressed = false;

// Timing constants (milliseconds)
const unsigned long RED_DURATION = 5000;        // 5 seconds
const unsigned long YELLOW_DURATION = 2000;    // 2 seconds
const unsigned long GREEN_DURATION = 4000;     // 4 seconds
const unsigned long PEDESTRIAN_OVERRIDE = 8000; // 8 seconds forced red

void setup() {
  // Initialize LED pins as outputs
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  
  // Initialize pedestrian button as input
  pinMode(PEDESTRIAN_BUTTON, INPUT);
  
  // Initialize Serial for state tracking
  Serial.begin(9600);
  Serial.println("===== Traffic Light FSM Started =====");
  Serial.println("Red: 5s | Yellow: 2s | Green: 4s");
  Serial.println("Pedestrian button forces 8s red phase");
  Serial.println("=====================================");
  
  // Set initial state to RED
  current_state = RED;
  state_start_time = millis();
  digitalWrite(RED_LED, HIGH);
  Serial.println("STATE: IDLE → RED");
}

void loop() {
  // Non-blocking timing: get current time
  unsigned long current_time = millis();
  
  // Check pedestrian button (non-blocking)
  if (digitalRead(PEDESTRIAN_BUTTON) == HIGH && !pedestrian_pressed) {
    pedestrian_pressed = true;
    pedestrian_press_time = current_time;
    Serial.println("[PEDESTRIAN] Button pressed - forcing RED for 8s");
  }
  
  // Reset pedestrian flag after debounce
  if (pedestrian_pressed && (current_time - pedestrian_press_time > 100)) {
    pedestrian_pressed = false;
  }
  
  // FSM state machine with non-blocking timing
  switch (current_state) {
    case RED:
      // Check if pedestrian override is active
      if (pedestrian_pressed) {
        state_start_time = current_time; // Reset timer
      }
      
      // Determine next state based on elapsed time
      if ((current_time - state_start_time) >= PEDESTRIAN_OVERRIDE && pedestrian_pressed) {
        // Pedestrian override complete, go to GREEN
        current_state = GREEN;
        state_start_time = current_time;
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        Serial.print("STATE: RED → GREEN | Time: ");
        Serial.print(current_time);
        Serial.println(" ms");
        pedestrian_pressed = false;
      } else if ((current_time - state_start_time) >= RED_DURATION && !pedestrian_pressed) {
        // Normal RED duration complete
        current_state = YELLOW;
        state_start_time = current_time;
        digitalWrite(RED_LED, LOW);
        digitalWrite(YELLOW_LED, HIGH);
        Serial.print("STATE: RED → YELLOW | Time: ");
        Serial.print(current_time);
        Serial.println(" ms");
      }
      break;
    
    case YELLOW:
      // Yellow always transitions to GREEN after 2 seconds
      if ((current_time - state_start_time) >= YELLOW_DURATION) {
        current_state = GREEN;
        state_start_time = current_time;
        digitalWrite(YELLOW_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        Serial.print("STATE: YELLOW → GREEN | Time: ");
        Serial.print(current_time);
        Serial.println(" ms");
      }
      break;
    
    case GREEN:
      // Green transitions to RED after 4 seconds
      if ((current_time - state_start_time) >= GREEN_DURATION) {
        current_state = RED;
        state_start_time = current_time;
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(RED_LED, HIGH);
        Serial.print("STATE: GREEN → RED | Time: ");
        Serial.print(current_time);
        Serial.println(" ms");
      }
      break;
  }
  
  // Optional: Print current state every 5 seconds for monitoring
  static unsigned long last_log = 0;
  if ((current_time - last_log) >= 5000) {
    last_log = current_time;
    const char* state_name;
    switch (current_state) {
      case RED: state_name = "RED"; break;
      case YELLOW: state_name = "YELLOW"; break;
      case GREEN: state_name = "GREEN"; break;
    }
    Serial.print("[LOG] Current State: ");
    Serial.print(state_name);
    Serial.print(" | Elapsed: ");
    Serial.print(current_time - state_start_time);
    Serial.println(" ms");
  }
}
