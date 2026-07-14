#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

// Network & Secret Configurations
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
const char* botToken = "YOUR_TELEGRAM_BOT_TOKEN";
const char* chatId = "YOUR_TELEGRAM_CHAT_ID";

// Hardware Pin Definitions
#define PIR_PIN 13
#define POT_PIN 34  // Analog pin simulating PIR sensitivity threshold adjustment
#define BUZZER_PIN 12
#define LED1 14
#define LED2 27
#define LED3 26

// Global Configuration Variables (Default shop hours: 22:00 to 06:00)
int activeStartHour = 22;
int activeEndHour = 6;

// Simulated Internal Clock (Incremental tracking for standalone simulation)
int currentSimHour = 23; 
int currentSimMin = 0;

// System Escalation States
enum EscalationLevel { IDLE, WARNING, ALARM, URGENT };
EscalationLevel currentLevel = IDLE;

// Timing Variables (Non-blocking tracking)
unsigned long lastClockTick = 0;
unsigned long stateChangeTimestamp = 0;
unsigned long lastLedFlashTime = 0;
unsigned long lastBuzzerTime = 0;

bool isAlertActive = false;
bool ledToggleState = false;

WiFiClientSecure secured_client;
UniversalTelegramBot bot(botToken, secured_client);

void setup() {
  Serial.begin(115200);
  
  pinMode(PIR_PIN, INPUT);
  pinMode(POT_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(LED1, OUTPUT);
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);

  // Initialize Network Connectivity
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Network");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Connected successfully!");
  
  // Attach Root Certificate SSL requirement for Telegram API
  secured_client.setInsecure(); 

  Serial.println("\n--- System Operational ---");
  printConfiguredHours();
  Serial.println("Send Serial Commands to update time window. Format: SET_HOURS StartHour EndHour (e.g., SET_HOURS 21 05)");
}

void loop() {
  unsigned long currentMillis = millis();

  // 1. Maintain simulated software clock metrics (1 second = 1 virtual minute for simulation speed)
  if (currentMillis - lastClockTick >= 1000) {
    lastClockTick = currentMillis;
    currentSimMin++;
    if (currentSimMin >= 60) {
      currentSimMin = 0;
      currentSimHour = (currentSimHour + 1) % 24;
    }
  }

  // 2. Parse Incoming Runtime Configuration Commands via Serial
  if (Serial.available() > 0) {
    String commandStr = Serial.readStringUntil('\n');
    commandStr.trim();
    if (commandStr.startsWith("SET_HOURS")) {
      int firstSpace = commandStr.indexOf(' ');
      int secondSpace = commandStr.indexOf(' ', firstSpace + 1);
      if (firstSpace != -1 && secondSpace != -1) {
        activeStartHour = commandStr.substring(firstSpace + 1, secondSpace).toInt();
        activeEndHour = commandStr.substring(secondSpace + 1).toInt();
        Serial.println("LOG [CONFIG CHANGED]: Operational window successfully updated.");
        printConfiguredHours();
      }
    }
  }

  // 3. Process Sensor Scanning Loops
  bool isMotionDetected = (digitalRead(PIR_PIN) == HIGH);
  int sensitivityPot = analogRead(POT_PIN); // Max scale 4095

  // Map potentiometer threshold: lower pot setting means a lower analog wall to cross (high sensitivity)
  bool isSystemTriggered = isMotionDetected && (sensitivityPot > 500); 
  bool isWithinShopHours = checkIfActiveHours();

  // 4. Multi-Stage Security State Handling
  if (isSystemTriggered && isWithinShopHours) {
    if (!isAlertActive) {
      isAlertActive = true;
      stateChangeTimestamp = currentMillis;
      currentLevel = WARNING;
      logEventToSerial("MOTION DETECTED - Initial Warning Activated");
      sendTelegramAlert("⚠️ Security Alert: Motion detected in Rohtak shop during off-hours!");
    } else {
      // Escalation Timing logic over continuous presence
      unsigned long dynamicDelta = currentMillis - stateChangeTimestamp;
      if (dynamicDelta >= 15000) { // Over 15 seconds continuous motion
        if (currentLevel != URGENT) {
          currentLevel = URGENT;
          logEventToSerial("CRITICAL ALERT - System Escalated to URGENT");
          sendTelegramAlert("🚨 EMERGENCY: Continuous motion detected! Alarm level elevated to URGENT.");
        }
      } else if (dynamicDelta >= 7000) { // Over 7 seconds continuous motion
        if (currentLevel == WARNING) {
          currentLevel = ALARM;
          logEventToSerial("ALERT LEVEL ESCALATED - System in ALARM state");
        }
      }
    }
    executeAlertSignaling(currentLevel, currentMillis);
  } else {
    // Gracefully wind down alerts if motion halts
    if (isAlertActive) {
      isAlertActive = false;
      currentLevel = IDLE;
      logEventToSerial("Area Clear - Security Status Reset to IDLE");
      clearHardwareLines();
    }
  }
}

// Computes wrapping time windows across midnight boundaries
bool checkIfActiveHours() {
  if (activeStartHour > activeEndHour) {
    return (currentSimHour >= activeStartHour || currentSimHour < activeEndHour);
  } else {
    return (currentSimHour >= activeStartHour && currentSimHour < activeEndHour);
  }
}

// Drives non-blocking visual sequential flash updates and multi-frequency audio warnings
void executeAlertSignaling(EscalationLevel level, unsigned long currentMillis) {
  switch (level) {
    case WARNING:
      // Slow pulsing tone, single diagnostic LED flip
      if (currentMillis - lastBuzzerTime >= 600) {
        lastBuzzerTime = currentMillis;
        tone(BUZZER_PIN, 1000, 100);
      }
      if (currentMillis - lastLedFlashTime >= 300) {
        lastLedFlashTime = currentMillis;
        ledToggleState = !ledToggleState;
        digitalWrite(LED1, ledToggleState ? HIGH : LOW);
        digitalWrite(LED2, LOW);
        digitalWrite(LED3, LOW);
      }
      break;

    case ALARM:
      // Faster high/low warning frequencies, twin alternating LEDs
      if (currentMillis - lastBuzzerTime >= 300) {
        lastBuzzerTime = currentMillis;
        tone(BUZZER_PIN, (currentMillis % 2000 > 1000) ? 1500 : 1200, 150);
      }
      if (currentMillis - lastLedFlashTime >= 150) {
        lastLedFlashTime = currentMillis;
        ledToggleState = !ledToggleState;
        digitalWrite(LED1, ledToggleState ? HIGH : LOW);
        digitalWrite(LED2, ledToggleState ? LOW : HIGH);
        digitalWrite(LED3, LOW);
      }
      break;

    case URGENT:
      // Piercing constant sweeping acoustic alarms, sequential hyper-speed strobing
      if (currentMillis - lastBuzzerTime >= 100) {
        lastBuzzerTime = currentMillis;
        tone(BUZZER_PIN, 2800, 80);
      }
      if (currentMillis - lastLedFlashTime >= 60) {
        lastLedFlashTime = currentMillis;
        int activeLedIndex = (currentMillis / 60) % 3;
        digitalWrite(LED1, (activeLedIndex == 0) ? HIGH : LOW);
        digitalWrite(LED2, (activeLedIndex == 1) ? HIGH : LOW);
        digitalWrite(LED3, (activeLedIndex == 2) ? HIGH : LOW);
      }
      break;

    default:
      clearHardwareLines();
      break;
  }
}

void clearHardwareLines() {
  noTone(BUZZER_PIN);
  digitalWrite(LED1, LOW);
  digitalWrite(LED2, LOW);
  digitalWrite(LED3, LOW);
}

void logEventToSerial(String message) {
  char timeBuffer[16];
  sprintf(timeBuffer, "[%02d:%02d]", currentSimHour, currentSimMin);
  Serial.print(timeBuffer);
  Serial.print(" ");
  Serial.println(message);
}

void printConfiguredHours() {
  Serial.printf("Current Security Active Window: [%02d:00] to [%02d:00]\n", activeStartHour, activeEndHour);
}

void sendTelegramAlert(String txt) {
  if (WiFi.status() == WL_CONNECTED) {
    if (bot.sendMessage(chatId, txt, "")) {
      Serial.println("LOG [NETWORK]: Telegram notification dispatched successfully.");
    } else {
      Serial.println("LOG [ERROR]: Telegram delivery failed.");
    }
  }
}
