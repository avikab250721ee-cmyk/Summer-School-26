#include <Keypad.h>
#include <LiquidCrystal_I2C.h>
#include <ESP32Servo.h>
#include "BluetoothSerial.h"

// System States
enum LockState { ENTER_ID, ENTER_OTP, ACCESS_GRANTED, ACCESS_DENIED };
LockState currentState = ENTER_ID;

// Keypad Configuration
const byte ROWS = 4; 
const byte COLS = 4; 
char keys[ROWS][COLS] = {
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
};
byte rowPins[ROWS] = {13, 12, 14, 27}; // ESP32 GPIOs connected to row pins of the keypad
byte colPins[COLS] = {26, 25, 33, 32}; // ESP32 GPIOs connected to column pins of the keypad

Keypad customKeypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

// Peripherals Setup
LiquidCrystal_I2C lcd(0x27, 16, 2);
Servo lockServo;
BluetoothSerial SerialBT;

#define SERVO_PIN 15

// Buffers and Variables
String inputBuffer = "";
String studentID = "";
String generatedOTP = "";

unsigned long otpTimestamp = 0;
const unsigned long OTP_TIMEOUT = 30000; // 30 seconds limit
int lastRemainingSeconds = -1;

void setup() {
  Serial.begin(115200);
  
  // Initialize Bluetooth Classic
  SerialBT.begin("Hostel_Gate_Lock"); 
  Serial.println("Bluetooth Device Initialized: 'Hostel_Gate_Lock'");

  // Initialize LCD Screen
  lcd.init();
  lcd.backlight();
  
  // Initialize Servo Motor
  ESP32PWM::allocateTimer(0);
  lockServo.setPeriodHertz(50);
  lockServo.attach(SERVO_PIN, 500, 2400);
  lockServo.write(0); // Door Locked at 0 degrees

  resetToDefaultState();
}

void loop() {
  char key = customKeypad.getKey();
  unsigned long currentMillis = millis();

  // Handle Timeout Countdown if waiting for OTP entry
  if (currentState == ENTER_OTP) {
    if (currentMillis - otpTimestamp >= OTP_TIMEOUT) {
      Serial.println("LOG [FAILURE]: OTP Entry Timed Out for Student ID: " + studentID);
      currentState = ACCESS_DENIED;
      handleAccessDenied("Time Out!");
    } else {
      // Calculate and refresh countdown text on LCD without blocking loop execution
      int remainingSeconds = (OTP_TIMEOUT - (currentMillis - otpTimestamp)) / 1000;
      if (remainingSeconds != lastRemainingSeconds) {
        lastRemainingSeconds = remainingSeconds;
        lcd.setCursor(11, 0);
        lcd.print("T-");
        if(remainingSeconds < 10) lcd.print("0");
        lcd.print(remainingSeconds);
      }
    }
  }

  // Handle Keypad Character Intakes
  if (key) {
    if (key == '*') { 
      // Clear current typing line buffer
      inputBuffer = "";
      lcd.setCursor(0, 1);
      lcd.print("                ");
      lcd.setCursor(0, 1);
    } 
    else if (key == '#') { 
      // Confirm submission
      processBufferSubmission();
    } 
    else if (key != 'A' && key != 'B' && key != 'C' && key != 'D') {
      // Add digits to string variable buffers
      if (currentState == ENTER_ID && inputBuffer.length() < 4) {
        inputBuffer += key;
        lcd.setCursor(inputBuffer.length() - 1, 1);
        lcd.print(key); // Display raw ID digit
      } 
      else if (currentState == ENTER_OTP && inputBuffer.length() < 6) {
        inputBuffer += key;
        lcd.setCursor(inputBuffer.length() - 1, 1);
        lcd.print('*'); // Masked visual input confirmation
      }
    }
  }
}

void resetToDefaultState() {
  inputBuffer = "";
  studentID = "";
  generatedOTP = "";
  currentState = ENTER_ID;
  lastRemainingSeconds = -1;
  
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Enter 4-Digit ID");
  lcd.setCursor(0, 1);
}

void processBufferSubmission() {
  if (currentState == ENTER_ID) {
    if (inputBuffer.length() == 4) {
      studentID = inputBuffer;
      inputBuffer = ""; // Flush buffer for next stage
      
      // Generate Secure 6-Digit Verification Token
      generatedOTP = "";
      for (int i = 0; i < 6; i++) {
        generatedOTP += String(random(0, 10));
      }
      
      // Transmit generated password via Bluetooth pipeline
      if (SerialBT.hasClient()) {
        SerialBT.println("\n==========================");
        SerialBT.println("HOSTEL AUTOMATED GATE LOCK");
        SerialBT.print("Your 6-Digit OTP is: ");
        SerialBT.println(generatedOTP);
        SerialBT.println("Valid for exactly 30 seconds.");
        SerialBT.println("==========================");
      }
      
      // Output generation status locally to tracking port
      Serial.println("DEBUG: Generated OTP [" + generatedOTP + "] for Student ID: " + studentID);
      
      // Update State variables
      currentState = ENTER_OTP;
      otpTimestamp = millis();
      
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Enter 6-Digit OTP");
    }
  } 
  else if (currentState == ENTER_OTP) {
    if (inputBuffer.length() == 6) {
      if (inputBuffer == generatedOTP) {
        Serial.println("LOG [SUCCESS]: Access Granted to Student ID: " + studentID);
        currentState = ACCESS_GRANTED;
        handleAccessGranted();
      } else {
        Serial.println("LOG [FAILURE]: Invalid OTP Entered for Student ID: " + studentID);
        currentState = ACCESS_DENIED;
        handleAccessDenied("Wrong OTP!");
      }
    }
  }
}

void handleAccessGranted() {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Granted!");
  lcd.setCursor(0, 1);
  lcd.print("Welcome Home");
  
  // Pivot lock arm 90 degrees to simulate unlatching mechanics
  lockServo.write(90); 
  delay(4000); // Wait 4 seconds for the student to pass through
  
  lockServo.write(0); // Relatch lock
  resetToDefaultState();
}

void handleAccessDenied(String reason) {
  lcd.clear();
  lcd.setCursor(0, 0);
  lcd.print("Access Denied!");
  lcd.setCursor(0, 1);
  lcd.print(reason);
  
  delay(2500); // Keep message context readable
  resetToDefaultState();
}
