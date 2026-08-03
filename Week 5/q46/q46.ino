#include <WiFi.h>
#include <WebSocketsServer.h>

// Network Configuration parameters
const char* ssid     = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";

// Hardware Pin Layout Configurations
#define TRIG_PIN 12
#define ECHO_PIN 14
#define BUZZER_PIN 26
#define STATUS_LED 27

// System Brackets Configuration (Distance in cm)
const float ZONE_NORMAL   = 30.0; // Distance > 30cm
const float ZONE_WARNING  = 15.0; // Distance between 15cm and 30cm

// Rolling Filter Smoothing Configuration
const int FILTER_SAMPLES = 10;
float runningSamples[FILTER_SAMPLES];
int sampleIndex = 0;

// System Level States
enum FloodState { NORMAL, WARNING, CRITICAL };
FloodState currentSystemState = NORMAL;

// Timing Registers (Non-blocking tracking variables)
unsigned long lastSensorPoll = 0;
unsigned long lastAlertToggle = 0;
const unsigned long POLL_INTERVAL = 200; // Poll ultrasonic sensor every 200ms

float smoothedDistance = 0.0;
bool indicatorToggleState = false;

// Instantiate localized WebSocket server listening on port 81
WebSocketsServer webSocket = WebSocketsServer(81);

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(STATUS_LED, OUTPUT);

  // Initialize rolling window filter buffer array with an initial telemetry probe
  float initialRead = getUltrasonicRawDistance();
  for (int i = 0; i < FILTER_SAMPLES; i++) {
    runningSamples[i] = initialRead;
  }
  smoothedDistance = initialRead;

  // Spin up Access Link Network stack
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi Network");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWiFi Infrastructure Established!");
  Serial.print("Node Local IP Server Endpoint Address: ");
  Serial.println(WiFi.localIP());

  // Startup WebSocket listener protocol
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop();
  unsigned long currentMillis = millis();

  // 1. Data Collection Pipeline (Executed every 200ms)
  if (currentMillis - lastSensorPoll >= POLL_INTERVAL) {
    lastSensorPoll = currentMillis;

    float rawRead = getUltrasonicRawDistance();
    // Validate bounds to prevent rogue echo calculations from muddying the filter array
    if (rawRead > 2.0 && rawRead < 400.0) {
      runningSamples[sampleIndex] = rawRead;
      sampleIndex = (sampleIndex + 1) % FILTER_SAMPLES;
    }

    // Compute rolling 10-sample array average
    float totalSum = 0;
    for (int i = 0; i < FILTER_SAMPLES; i++) {
      totalSum += runningSamples[i];
    }
    smoothedDistance = totalSum / FILTER_SAMPLES;

    // Evaluate Risk State Parameters
    FloodState originalState = currentSystemState;
    if (smoothedDistance < ZONE_WARNING) {
      currentSystemState = CRITICAL;
    } else if (smoothedDistance <= ZONE_NORMAL) {
      currentSystemState = WARNING;
    } else {
      currentSystemState = NORMAL;
    }

    // Broadcast live telemetry string packet instantly down the socket to active dashboards
    broadcastTelemetryData();
  }

  // 2. Local Peripheral Alert Feedback Engine (Non-blocking patterns)
  executePeripheralAlertSignaling(currentSystemState, currentMillis);
}

// Low-level hardware pulse trigger to isolate echoing acoustic delays
float getUltrasonicRawDistance() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long soundTravelDuration = pulseIn(ECHO_PIN, HIGH, 30000); // 30ms timeout barrier
  // Distance = (Speed of Sound * Time Elapsed) / 2 (accounting for structural bounce)
  float mappedDistance = (soundTravelDuration * 0.0343) / 2.0;
  return mappedDistance;
}

// Manages signaling intervals dynamically without stopping code execution
void executePeripheralAlertSignaling(FloodState state, unsigned long currentMillis) {
  switch (state) {
    case NORMAL:
      digitalWrite(STATUS_LED, HIGH); // Solid indication of normal operations
      noTone(BUZZER_PIN);
      break;

    case WARNING:
      // Modulated caution pattern: flip indicators every 400ms
      if (currentMillis - lastAlertToggle >= 400) {
        lastAlertToggle = currentMillis;
        indicatorToggleState = !indicatorToggleState;
        digitalWrite(STATUS_LED, indicatorToggleState ? HIGH : LOW);
        if (indicatorToggleState) {
          tone(BUZZER_PIN, 1000, 80); // Quick warning chirp
        } else {
          noTone(BUZZER_PIN);
        }
      }
      break;

    case CRITICAL:
      // High-frequency evacuation pattern: toggle indicators every 100ms
      if (currentMillis - lastAlertToggle >= 100) {
        lastAlertToggle = currentMillis;
        indicatorToggleState = !indicatorToggleState;
        digitalWrite(STATUS_LED, indicatorToggleState ? HIGH : LOW);
        if (indicatorToggleState) {
          tone(BUZZER_PIN, 2200, 50); // Sharp emergency pulse
        } else {
          noTone(BUZZER_PIN);
        }
      }
      break;
  }
}

// Packages metrics and alerts into a compact JSON string transmitted across sockets
void broadcastTelemetryData() {
  String statusString = "NORMAL";
  if (currentSystemState == WARNING) statusString = "WARNING";
  else if (currentSystemState == CRITICAL) statusString = "CRITICAL";

  String jsonPayload = "{";
  jsonPayload += "\"distance\":" + String(smoothedDistance, 1) + ",";
  jsonPayload += "\"status\":\"" + statusString + "\"";
  jsonPayload += "}";

  webSocket.broadcastTXT(jsonPayload);
}

void webSocketEvent(uint8_t num, WsMoveType type, uint8_t * payload, size_t length) {
  if (type == WsMoveType_CONNECTED) {
    Serial.printf("LOG [NETWORK]: Client interface node #%d established socket connection.\n", num);
  }
}
