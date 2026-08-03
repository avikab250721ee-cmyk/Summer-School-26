#include <Wire.h>

// Hardware Pin Layout Configurations
#define PROXY_INPUT_PIN A0  // Analog pin acting as sensor proxy input (Potentiometer/LDR)
#define STATUS_RED_LED  14  // Red LED indicating vital warning thresholds
#define ALERT_BUZZER    12  // Audio buzzer indicator for immediate risk states

// Timing Configurations (Millis non-blocking tracking)
unsigned long lastUpdateTimestamp = 0;
const unsigned long UPDATE_INTERVAL = 2000; // Force update refresh every 2 seconds

// Rolling Filter Parameters
const int WINDOW_SIZE = 5;
int hrSamples[WINDOW_SIZE];
int spo2Samples[WINDOW_SIZE];
int filterIndex = 0;

void setup() {
  Serial.begin(115200);

  pinMode(PROXY_INPUT_PIN, INPUT);
  pinMode(STATUS_RED_LED, OUTPUT);
  pinMode(ALERT_BUZZER, OUTPUT);

  // Initial diagnostics checking flash pins
  digitalWrite(STATUS_RED_LED, HIGH);
  delay(200);
  digitalWrite(STATUS_RED_LED, LOW);

  // Pre-seed filter buffers to prevent radical startup drift spikes
  int seedVal = analogRead(PROXY_INPUT_PIN);
  int seedHR = map(seedVal, 0, 4095, 40, 140);
  int seedSpO2 = map(seedVal, 0, 4095, 88, 100);
  for (int i = 0; i < WINDOW_SIZE; i++) {
    hrSamples[i] = seedHR;
    spo2Samples[i] = seedSpO2;
  }

  // Print baseline structural layout header to match project file specifications
  Serial.println("\n--- HEALTH LOGGING CORE START ---");
  Serial.println("Timestamp_ms,Raw_HeartRate_BPM,Smoothed_HR_BPM,Raw_SpO2_Pct,Smoothed_SpO2_Pct,Health_Status");
}

void loop() {
  unsigned long currentMillis = millis();

  // Non-blocking timer tracking execution cycles every 2 seconds
  if (currentMillis - lastUpdateTimestamp >= UPDATE_INTERVAL) {
    lastUpdateTimestamp = currentMillis;

    // Read the physical simulation proxy wiper setting
    int inputVal = analogRead(PROXY_INPUT_PIN);

    // Generate simulated Heart Rate & SpO2 readings mapped from input
    int rawHR = map(inputVal, 0, 4095, 40, 140); // Standard human variance window
    int rawSpO2 = map(inputVal, 0, 4095, 88, 100); // Hypoxia through nominal saturation bounds

    // Inject parameters into the circular array index positions
    hrSamples[filterIndex] = rawHR;
    spo2Samples[filterIndex] = rawSpO2;
    filterIndex = (filterIndex + 1) % WINDOW_SIZE;

    // Calculate 5-Reading Rolling Averages
    int hrSum = 0;
    int spo2Sum = 0;
    for (int i = 0; i < WINDOW_SIZE; i++) {
      hrSum += hrSamples[i];
      spo2Sum += spo2Samples[i];
    }
    int smoothedHR = hrSum / WINDOW_SIZE;
    int smoothedSpO2 = spo2Sum / WINDOW_SIZE;

    // Evaluate Risk State Thresholds
    bool alertTriggered = false;
    String conditionFlag = "NORMAL";

    if (smoothedHR < 50 || smoothedHR > 120 || smoothedSpO2 < 94) {
      alertTriggered = true;
      conditionFlag = "ABNORMAL_CRITICAL";
    }

    // Actuate Local Alert Hardware Lines
    if (alertTriggered) {
      digitalWrite(STATUS_RED_LED, HIGH);
      tone(ALERT_BUZZER, 1500, 300); // Sound warning pulse
    } else {
      digitalWrite(STATUS_RED_LED, LOW);
      noTone(ALERT_BUZZER);
    }

    // Print CSV output format to terminal interface mapping standard logging files
    Serial.print(currentMillis);
    Serial.print(", ");
    Serial.print(rawHR);
    Serial.print(", ");
    Serial.print(smoothedHR);
    Serial.print(", ");
    Serial.print(rawSpO2);
    Serial.print(", ");
    Serial.print(smoothedSpO2);
    Serial.print(", ");
    Serial.println(conditionFlag);
  }
}
