#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <DHT.h>
#include "config.h" // Includes ssid, password, apiKey, and city safely

// DHT11 Configuration
#define DHTPIN 4          // GPIO pin connected to the DHT11 data pin
#define DHTTYPE DHT11     // Specifying the sensor type
DHT dht(DHTPIN, DHTTYPE);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize DHT sensor
  dht.begin();

  // Connect to Wi-Fi
  Serial.println("\n--- Connecting to Wi-Fi ---");
  Serial.print("Connecting to: ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  
  Serial.println("\nWiFi Connected Successfully!");
  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());
  Serial.println("---------------------------\n");
}

void loop() {
  if (WiFi.status() == WL_CONNECTED) {
    
    // 1. Read Local Data from DHT11
    float localTemp = dht.readTemperature();
    float localHumid = dht.readHumidity();

    // Check if DHT sensor readings failed
    if (isnan(localTemp) || isnan(localHumid)) {
      Serial.println("⚠️ Failed to read from DHT sensor! Checking API anyway...");
      localTemp = 0.0; // Placeholder if sensor fails
    }

    // 2. Fetch Remote Weather Data from OpenWeatherMap API
    HTTPClient http;
    
    // Construct the endpoint URL using variables from config.h
    // Units=metric gives Celsius temperature
    String url = "http://api.openweathermap.org/data/2.5/weather?q=" + String(city) + 
                 "&appid=" + String(apiKey) + "&units=metric";
    
    http.begin(url);
    int httpResponseCode = http.GET();
    
    if (httpResponseCode == 200) {
      String payload = http.getString();
      
      // Allocate the JSON document
      // DynamicJsonDocument is optimized for ESP32
      DynamicJsonDocument doc(1024);
      DeserializationError error = deserializeJson(doc, payload);
      
      if (!error) {
        // Parse the required data points from JSON
        const char* cityName = doc["name"];
        float apiTemp = doc["main"]["temp"];
        float apiHumid = doc["main"]["humidity"];
        const char* weatherDesc = doc["weather"][0]["description"];
        
        // Calculate differences between API and local sensor
        float tempDiff = apiTemp - localTemp;
        float humidDiff = apiHumid - localHumid;

        // 3. Display data on Serial Monitor
        Serial.println("================ WEATHER REPORT ================");
        Serial.print("City Name:           "); Serial.println(cityName);
        Serial.print("Weather Condition:   "); Serial.println(weatherDesc);
        Serial.println("------------------------------------------------");
        
        // Temperature comparison
        Serial.print("API Temperature:     "); Serial.print(apiTemp); Serial.println(" °C");
        if (localTemp != 0.0) {
          Serial.print("DHT11 Temperature:   "); Serial.print(localTemp); Serial.println(" °C");
          Serial.print("Temp Difference:     "); Serial.print(tempDiff); Serial.println(" °C");
        }
        Serial.println("------------------------------------------------");
        
        // Humidity comparison
        Serial.print("API Humidity:        "); Serial.print(apiHumid); Serial.println(" %");
        if (localHumid != 0.0) {
          Serial.print("DHT11 Humidity:      "); Serial.print(localHumid); Serial.println(" %");
          Serial.print("Humidity Difference: "); Serial.print(humidDiff); Serial.println(" %");
        }
        Serial.println("================================================\n");

      } else {
        Serial.print("❌ JSON Parsing failed: ");
        Serial.println(error.c_str());
      }
    } else {
      Serial.print("❌ Error on HTTP request. Code: ");
      Serial.println(httpResponseCode);
    }
    
    http.end(); // Free resources
  } else {
    Serial.println("❌ Wi-Fi Disconnected");
  }

  // Fetch and update data every 15 minutes (OpenWeatherMap free tier request limit safety)
  delay(900000); 
}
