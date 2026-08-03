/*
 * Project: Wi-Fi Temperature Monitor (ESP32/ESP8266)
 * Question: Q33
 * Author: [Shashwat Singh]
 */

#ifdef ESP32
#include <WiFi.h>
#include <WebServer.h>
WebServer server(80);
#else
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);
#endif

#include <DHT.h>

// ================= Wi-Fi Credentials =================
const char* ssid = "YOUR_WIFI_NAME";
const char* password = "YOUR_WIFI_PASSWORD";

// ================= DHT Configuration =================
#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// ================= LED Pin =================
#ifdef ESP32
const int LED_PIN = 2;     // Built-in LED
#else
const int LED_PIN = LED_BUILTIN;
#endif

bool ledState = false;

// =====================================================
// Function to generate HTML webpage
// =====================================================
void handleRoot() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  unsigned long seconds = millis() / 1000;
  unsigned long minutes = seconds / 60;
  unsigned long hours = minutes / 60;

  String html;

  html += "<!DOCTYPE html><html>";
  html += "<head>";

  // Refresh page every 10 seconds
  html += "<meta http-equiv='refresh' content='10'>";

  html += "<title>ESP Temperature Monitor</title>";

  html += "<style>";
  html += "body{font-family:Arial;text-align:center;background:#f5f5f5;}";
  html += "h1{color:#1565C0;}";
  html += ".card{background:white;padding:20px;margin:auto;width:350px;border-radius:10px;box-shadow:0 0 10px gray;}";
  html += "button{padding:12px 25px;font-size:18px;}";
  html += "</style>";

  html += "</head><body>";

  html += "<div class='card'>";

  html += "<h1>Wi-Fi Temperature Monitor</h1>";

  html += "<h2>Temperature: ";
  html += String(temperature);
  html += " &deg;C</h2>";

  html += "<h2>Humidity: ";
  html += String(humidity);
  html += " %</h2>";

  html += "<h3>Uptime: ";
  html += String(hours);
  html += "h ";
  html += String(minutes % 60);
  html += "m ";
  html += String(seconds % 60);
  html += "s</h3>";

  html += "<p>LED State: ";
  html += (ledState ? "ON" : "OFF");
  html += "</p>";

  html += "<a href='/toggle'><button>Toggle LED</button></a>";

  html += "</div>";

  html += "</body></html>";

  server.send(200, "text/html", html);
}

// =====================================================
// Toggle LED
// =====================================================
void handleToggle() {

  ledState = !ledState;

  digitalWrite(LED_PIN, ledState);

  server.sendHeader("Location", "/");
  server.send(303);
}

// =====================================================
// Setup
// =====================================================
void setup() {

  Serial.begin(115200);

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);

  dht.begin();

  Serial.println();
  Serial.println("Connecting to Wi-Fi...");

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("Wi-Fi Connected!");

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/toggle", handleToggle);

  server.begin();

  Serial.println("Web Server Started");
}

// =====================================================
// Main Loop
// =====================================================
void loop() {

  server.handleClient();

}
