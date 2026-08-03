#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Jammu_SmartCity_WiFi";
const char* password = "SecureSmartPassword2026";
const char* mqtt_server = "10.10.0.5";
const char* mqtt_topic = "jammu/smartcity/wastebins";

#define TRIG_PIN 12
#define ECHO_PIN 14

const float BIN_MAX_DEPTH = 100.0; // Bin depth is 100cm when empty
unsigned long lastPublish = 0;
const unsigned long publishInterval = 15000; // Post data every 15s

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  client.setServer(mqtt_server, 1883);
}

float getBinClearanceDistance() {
  digitalWrite(TRIG_PIN, LOW); delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH); delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);
  return (duration * 0.0343) / 2.0;
}

void loop() {
  if (!client.connected()) { reconnectMqtt(); }
  client.loop();

  unsigned long currentMillis = millis();

  if (currentMillis - lastPublish >= publishInterval) {
    lastPublish = currentMillis;
    
    float clearance = getBinClearanceDistance();
    if (clearance > BIN_MAX_DEPTH) clearance = BIN_MAX_DEPTH;
    
    // Calculate fill percentage: smaller clearance distance means bin is filling up
    float fillPercentage = ((BIN_MAX_DEPTH - clearance) / BIN_MAX_DEPTH) * 100.0;
    if (fillPercentage < 0) fillPercentage = 0;

    String payload = "{\"node_id\":\"BIN_REHARI_12\",\"clearance_cm\":" + String(clearance, 1) + 
                     ",\"fill_percent\":" + String(fillPercentage, 0) + "}";
    client.publish(mqtt_topic, payload.cString());
    Serial.println("[NODE]: Waste telemetry dispatched -> " + payload);
  }
}

void reconnectMqtt() {
  while (!client.connected()) {
    if (client.connect("WasteBinNode-12")) {
      Serial.println("[MQTT]: Connected.");
    } else {
      delay(3000);
    }
  }
}
