/*
 * ----------------------------------------------------------
 * Project: MQTT Sensor Publisher (ESP32 + DHT11)
 * Question: Q34
 * Author: [Shashwat Singh]
 */

#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include <DHT.h>

// =========================
// Wi-Fi Credentials
// =========================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// =========================
// MQTT Broker
// =========================
const char* mqtt_server = "broker.hivemq.com";
const int mqtt_port = 1883;

// Replace with your own name
const char* topicTemp = "iitjammu/summer26/yourname/temperature";
const char* topicHum = "iitjammu/summer26/yourname/humidity";
const char* topicLED = "iitjammu/summer26/yourname/led_control";

// =========================
// DHT11 Configuration
// =========================
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// =========================
// LED Pin
// =========================
#define LED_PIN 2

WiFiClient espClient;
PubSubClient client(espClient);

unsigned long lastPublish = 0;

// =======================================
// Connect to Wi-Fi
// =======================================
void setup_wifi() {

  Serial.println("Connecting to WiFi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi Connected");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
}

// =======================================
// MQTT Callback
// =======================================
void callback(char* topic, byte* payload, unsigned int length) {

  String message = "";

  for (int i = 0; i < length; i++)
    message += (char)payload[i];

  Serial.print("Message Received: ");
  Serial.println(message);

  if (message == "ON") {
    digitalWrite(LED_PIN, HIGH);
  }

  else if (message == "OFF") {
    digitalWrite(LED_PIN, LOW);
  }
}

// =======================================
// MQTT Reconnect
// =======================================
void reconnect() {

  while (!client.connected()) {

    Serial.print("Connecting to MQTT...");

    String clientID = "ESP32Client-";
    clientID += String(random(0xffff), HEX);

    if (client.connect(clientID.c_str())) {

      Serial.println("Connected");

      client.subscribe(topicLED);

    } else {

      Serial.print("Failed, rc=");
      Serial.println(client.state());

      delay(3000);
    }
  }
}

// =======================================
// Setup
// =======================================
void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);

  dht.begin();

  setup_wifi();

  client.setServer(mqtt_server, mqtt_port);

  client.setCallback(callback);
}

// =======================================
// Main Loop
// =======================================
void loop() {

  if (!client.connected())
    reconnect();

  client.loop();

  if (millis() - lastPublish >= 5000) {

    lastPublish = millis();

    float temp = dht.readTemperature();
    float hum = dht.readHumidity();

    if (!isnan(temp) && !isnan(hum)) {

      // ---------- Temperature JSON ----------
      StaticJsonDocument<128> tempDoc;

      tempDoc["value"] = temp;
      tempDoc["unit"] = "C";
      tempDoc["ts"] = millis();

      char tempBuffer[128];

      serializeJson(tempDoc, tempBuffer);

      client.publish(topicTemp, tempBuffer);

      // ---------- Humidity JSON ----------
      StaticJsonDocument<128> humDoc;

      humDoc["value"] = hum;
      humDoc["unit"] = "%";
      humDoc["ts"] = millis();

      char humBuffer[128];

      serializeJson(humDoc, humBuffer);

      client.publish(topicHum, humBuffer);

      Serial.println("Published Sensor Data");
    }
  }
}
