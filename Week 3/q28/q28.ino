/*
 * Project: Keypad + LCD Password Entry System
 * Question: Q28
 * Author: [Shashwat Singh]
 */

#include <Keypad.h>
#include <LiquidCrystal.h>
#include <Arduino.h>

// LCD pins (16x2 display, 4-bit mode)
const int LCD_RS = 12;
const int LCD_EN = 11;
const int LCD_D4 = 6;
const int LCD_D5 = 5;
const int LCD_D6 = 4;
const int LCD_D7 = 3;
LiquidCrystal lcd(LCD_RS, LCD_EN, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

// 4x4 Keypad setup
const byte ROWS = 4;
const byte COLS = 4;
char keys[ROWS][COLS] = {
  {'1', '2', '3', 'A'},
  {'4', '5', '6', 'B'},
  {'7', '8', '9', 'C'},
  {'*', '0', '#', 'D'}
};

byte rowPins[ROWS] = {10, 9, 8, 7};        // Row pins
byte colPins[COLS] = {A3, A2, A1, A0};     // Column pins
Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Pin definitions
const int GREEN_LED = A5;  // Access granted LED
const int RED_LED = A4;    // Access denied LED
const int BUZZER = 2;      // Piezo buzzer

// Constants
const char CORRECT_PIN[5] = "1234";        // 4-digit password (changeable)
const unsigned long LOCKOUT_TIME = 10000;  // 10 seconds
const unsigned long ACCESS_LED_TIME = 2000; // 2 seconds to show access

// State variables
char entered_pin[5] = "";
int pin_index = 0;
int failed_attempts = 0;
bool locked_out = false;
unsigned long lockout_start_time = 0;
unsigned long access_time = 0;
bool access_granted = false;

void setup() {
  // Initialize LEDs and buzzer
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);
  pinMode(BUZZER, OUTPUT);
  digitalWrite(GREEN_LED, LOW);
  digitalWrite(RED_LED, LOW);
  digitalWrite(BUZZER, LOW);
  
  // Initialize LCD
  lcd.begin(16, 2);
  lcd.clear();
  lcd.print("ENTER PIN:");
  lcd.setCursor(0, 1);
  lcd.print("_ _ _ _");
  
  // Initialize Serial
  Serial.begin(9600);
  Serial.println("===== Keypad + LCD Access System Started =====");
  Serial.println("PIN: 1234 (4 digits)");
  Serial.println("Lockout: 10 seconds after 3 wrong attempts");
  Serial.println("==============================================");
}

void loop() {
  unsigned long current_time = millis();
  
  // === CHECK LOCKOUT STATUS ===
  if (locked_out && (current_time - lockout_start_time) >= LOCKOUT_TIME) {
    // Lockout expired, reset
    locked_out = false;
    failed_attempts = 0;
    pin_index = 0;
    entered_pin[0] = '\0';
    
    lcd.clear();
    lcd.print("ENTER PIN:");
    lcd.setCursor(0, 1);
    lcd.print("_ _ _ _");
    
    Serial.println("[LOCKOUT] Expired - system reset");
  }
  
  // === CHECK ACCESS GRANTED LED TIMEOUT ===
  if (access_granted && (current_time - access_time) >= ACCESS_LED_TIME) {
    access_granted = false;
    digitalWrite(GREEN_LED, LOW);
    
    // Reset for next entry
    pin_index = 0;
    entered_pin[0] = '\0';
    failed_attempts = 0;
    
    lcd.clear();
    lcd.print("ENTER PIN:");
    lcd.setCursor(0, 1);
    lcd.print("_ _ _ _");
  }
  
  // === READ KEYPAD (Non-blocking) ===
  if (!locked_out && !access_granted) {
    char key = keypad.getKey();
    
    if (key) {
      if (key == '*') {
        // Clear button
        pin_index = 0;
        entered_pin[0] = '\0';
        Serial.println("[CLEAR] PIN entry cleared");
        
        lcd.clear();
        lcd.print("ENTER PIN:");
        lcd.setCursor(0, 1);
        lcd.print("_ _ _ _");
      }
      else if (key == '#') {
        // Submit button
        if (pin_index == 4) {
          validate_pin();
        } else {
          Serial.println("[ERROR] Incomplete PIN (need 4 digits)");
          lcd.clear();
          lcd.print("INCOMPLETE!");
          lcd.setCursor(0, 1);
          lcd.print("Need 4 digits");
          delay(1500);
          lcd.clear();
          lcd.print("ENTER PIN:");
          lcd.setCursor(0, 1);
          lcd.print("_ _ _ _");
        }
      }
      else if (isDigit(key) && pin_index < 4) {
        // Digit entry
        entered_pin[pin_index] = key;
        entered_pin[pin_index + 1] = '\0';
        
        // Display asterisk instead of actual digit for security
        lcd.setCursor(pin_index * 2, 1);
        lcd.print("*");
        
        pin_index++;
        Serial.print("[KEY] Pressed: ");
        Serial.print(key);
        Serial.print(" (digits entered: ");
        Serial.print(pin_index);
        Serial.println("/4)");
      }
    }
  }
}

// Helper function: validate entered PIN
void validate_pin() {
  unsigned long current_time = millis();
  
  if (strcmp(entered_pin, CORRECT_PIN) == 0) {
    // Correct PIN
    Serial.println("[SUCCESS] Access GRANTED!");
    access_granted = true;
    access_time = current_time;
    digitalWrite(GREEN_LED, HIGH);
    digitalWrite(RED_LED, LOW);
    tone(BUZZER, 2000, 200);  // Success beep
    
    lcd.clear();
    lcd.print("ACCESS GRANTED!");
    lcd.setCursor(0, 1);
    lcd.print("Welcome!");
    
    failed_attempts = 0;  // Reset counter on success
  }
  else {
    // Incorrect PIN
    failed_attempts++;
    Serial.print("[FAIL] Incorrect PIN. Attempts: ");
    Serial.print(failed_attempts);
    Serial.println("/3");
    
    digitalWrite(RED_LED, HIGH);
    digitalWrite(GREEN_LED, LOW);
    tone(BUZZER, 800, 500);   // Error beep
    
    lcd.clear();
    lcd.print("ACCESS DENIED!");
    lcd.setCursor(0, 1);
    lcd.print("Attempts: ");
    lcd.print(failed_attempts);
    lcd.print("/3");
    
    delay(1500);
    
    if (failed_attempts >= 3) {
      // 3 strikes - lockout
      locked_out = true;
      lockout_start_time = current_time;
      pin_index = 0;
      entered_pin[0] = '\0';
      
      digitalWrite(RED_LED, HIGH);
      tone(BUZZER, 500, 1000);  // Long error beep
      
      lcd.clear();
      lcd.print("LOCKED OUT!");
      lcd.setCursor(0, 1);
      lcd.print("Wait 10 seconds");
      
      Serial.println("[LOCKOUT] System locked for 10 seconds");
    }
    else {
      // Reset for next attempt
      pin_index = 0;
      entered_pin[0] = '\0';
      
      delay(500);
      lcd.clear();
      lcd.print("ENTER PIN:");
      lcd.setCursor(0, 1);
      lcd.print("_ _ _ _");
    }
  }
}
