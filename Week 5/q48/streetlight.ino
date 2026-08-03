#include <WiFi.h>
#include <PubSubClient.h>

// WiFi & MQTT Configurations
const char* ssid = "Jammu_SmartCity_WiFi";
const char* password = "SecureSmartPassword2026";
const char* mqtt_server = "10.10.0.5"; // Municipal Local Broker
const char* mqtt_topic = "jammu/smartcity/streetlights";

#define LDR_PIN 34
#define LAMP_RELAY_PIN 14

const int LIGHT_THRESHOLD_ON = 500;  // Turn lamp on when it gets dark
const int LIGHT_THRESHOLD_OFF = 800; // Turn lamp off when day breaks
bool lampState = false;

unsigned long lastPublish = 0;
const unsigned long publishInterval = 10000; // 10 seconds

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(LDR_PIN, INPUT);
  pinMode(LAMP_RELAY_PIN, OUTPUT);
  digitalWrite(LAMP_RELAY_PIN, LOW);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) { reconnectMqtt(); }
  client.loop();

  unsigned long currentMillis = millis();
  int ldrRaw = analogRead(LDR_PIN);

  // Hysteresis Control Logic
  if (ldrRaw < LIGHT_THRESHOLD_ON && !lampState) {
    lampState = true;
    digitalWrite(LAMP_RELAY_PIN, HIGH);
    Serial.println("[NODE]: Ambient light low. Streetlamp ON.");
  } else if (ldrRaw > LIGHT_THRESHOLD_OFF && lampState) {
    lampState = false;
    digitalWrite(LAMP_RELAY_PIN, LOW);
    Serial.println("[NODE]: Ambient light sufficient. Streetlamp OFF.");
  }

  // Publish telemetry data periodically
  if (currentMillis - lastPublish >= publishInterval) {
    lastPublish = currentMillis;
    String payload = "{\"node_id\":\"LIGHT_JMU_04\",\"ambient_lux\":" + String(ldrRaw) + 
                     ",\"lamp_status\":" + String(lampState ? 1 : 0) + "}";
    client.publish(mqtt_topic, payload.cString());
  }
  delay(100);
}

void reconnectMqtt() {
  while (!client.connected()) {
    if (client.connect("StreetlightNode-04")) {
      Serial.println("[MQTT]: Connected to Jammu Central Broker.");
    } else {
      delay(3000);
    }
  }
}
