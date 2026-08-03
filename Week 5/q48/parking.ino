#include <WiFi.h>
#include <PubSubClient.h>

const char* ssid = "Jammu_SmartCity_WiFi";
const char* password = "SecureSmartPassword2026";
const char* mqtt_server = "10.10.0.5";
const char* mqtt_topic = "jammu/smartcity/parking";

#define PARKING_IR_PIN 25

bool lastOccupiedState = false;
unsigned long lastDebounceTime = 0;
const unsigned long debounceDelay = 2000; // 2 seconds stabilization

WiFiClient espClient;
PubSubClient client(espClient);

void setup() {
  Serial.begin(115200);
  pinMode(PARKING_IR_PIN, INPUT);

  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) { delay(500); }
  client.setServer(mqtt_server, 1883);
}

void loop() {
  if (!client.connected()) { reconnectMqtt(); }
  client.loop();

  // IR Obstacle Module pulls line LOW when an object blocks the transmitter beam
  bool currentRead = (digitalRead(PARKING_IR_PIN) == LOW);

  if (currentRead != lastOccupiedState) {
    if ((millis() - lastDebounceTime) > debounceDelay) {
      lastDebounceTime = millis();
      lastOccupiedState = currentRead;

      String payload = "{\"spot_id\":\"PK_GANDHINAGAR_40\",\"occupied\":" + String(lastOccupiedState ? 1 : 0) + "}";
      client.publish(mqtt_topic, payload.cString(), true); // Retain flag active for new client dashboard views
      Serial.println("[NODE]: Parking state changed. Payload sent: " + payload);
    }
  }
}

void reconnectMqtt() {
  while (!client.connected()) {
    if (client.connect("ParkingNode-40")) {
      Serial.println("[MQTT]: Parking client active.");
    } else {
      delay(3000);
    }
  }
}
