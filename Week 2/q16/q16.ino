/*
 * Project: Serial Command Interface for LED Control
 * Question: Q16
 * Author: [Shashwat Singh]
 * Commands List:
 *              LED_ON   - turn LED on
 *              LED_OFF  - turn LED off
 *              BLINK_X  - blink X times (X = 1-9)
 *              STATUS   - print current pin state
 *              RESET    - reset blink counter
 *              Includes input validation for malformed commands.
 */

#include <Arduino.h>

// Pin definitions
const int LED_PIN = 13;

// Global state variables
bool led_state = false;
int blink_counter = 0;

void setup() {
  // Initialize LED pin as output
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  // Initialize Serial at 9600 baud
  Serial.begin(9600);
  Serial.println("===== Serial Command Interface Started =====");
  Serial.println("Available Commands:");
  Serial.println("  LED_ON   - Turn LED on");
  Serial.println("  LED_OFF  - Turn LED off");
  Serial.println("  BLINK_X  - Blink X times (X = 1-9)");
  Serial.println("  STATUS   - Print current LED state");
  Serial.println("  RESET    - Reset blink counter");
  Serial.println("===========================================");
  Serial.println("Enter a command and press Enter:");
}

void loop() {
  // Check if data is available on Serial port
  if (Serial.available() > 0) {
    // Read the entire command line (until newline)
    String command = Serial.readStringUntil('\n');
    
    // Remove leading/trailing whitespace
    command.trim();
    
    // Convert to uppercase for case-insensitive matching
    command.toUpperCase();
    
    Serial.print("[CMD] Received: ");
    Serial.println(command);
    
    // Parse and execute commands
    if (command == "LED_ON") {
      digitalWrite(LED_PIN, HIGH);
      led_state = true;
      Serial.println("[SUCCESS] LED is ON");
    }
    else if (command == "LED_OFF") {
      digitalWrite(LED_PIN, LOW);
      led_state = false;
      Serial.println("[SUCCESS] LED is OFF");
    }
    else if (command.startsWith("BLINK_")) {
      // Extract the digit after "BLINK_"
      String digit_str = command.substring(6); // Get everything after "BLINK_"
      
      // Validate: must be a single digit 1-9
      if (digit_str.length() == 1 && isDigit(digit_str[0])) {
        int blink_count = digit_str.toInt();
        
        if (blink_count >= 1 && blink_count <= 9) {
          Serial.print("[BLINK] Blinking ");
          Serial.print(blink_count);
          Serial.println(" times...");
          
          // Perform blink action
          for (int i = 0; i < blink_count; i++) {
            digitalWrite(LED_PIN, HIGH);
            delay(200);
            digitalWrite(LED_PIN, LOW);
            delay(200);
            Serial.print(".");
          }
          Serial.println(" [DONE]");
          blink_counter += blink_count;
          led_state = false; // LED ends in OFF state
        } else {
          Serial.println("[ERROR] Blink count must be 1-9");
        }
      } else {
        Serial.println("[ERROR] Malformed BLINK_ command. Use BLINK_X where X=1-9");
      }
    }
    else if (command == "STATUS") {
      Serial.print("[STATUS] LED State: ");
      Serial.println(led_state ? "ON" : "OFF");
      Serial.print("[STATUS] Total Blinks: ");
      Serial.println(blink_counter);
    }
    else if (command == "RESET") {
      blink_counter = 0;
      Serial.println("[RESET] Blink counter reset to 0");
    }
    else {
      // Unknown command
      Serial.print("[ERROR] Unknown command: ");
      Serial.println(command);
      Serial.println("Type LED_ON, LED_OFF, BLINK_X (X=1-9), STATUS, or RESET");
    }
    
    Serial.println(); // Blank line for readability
  }
}
