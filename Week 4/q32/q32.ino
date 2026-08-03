/*
 * Project: Bluetooth Sensor Dashboard (ESP32 BLE)
 * Question: Q32
 * Author: [Shashwat Singh]
 */

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <DHT.h>

#define DHTPIN 4
#define DHTTYPE DHT11

DHT dht(DHTPIN, DHTTYPE);

// UUIDs
#define SERVICE_UUID      "12345678-1234-1234-1234-123456789000"
#define TEMP_UUID         "12345678-1234-1234-1234-123456789001"
#define HUM_UUID          "12345678-1234-1234-1234-123456789002"
#define TIME_UUID         "12345678-1234-1234-1234-123456789003"

BLECharacteristic *tempCharacteristic;
BLECharacteristic *humCharacteristic;
BLECharacteristic *timeCharacteristic;

void setup() {

  Serial.begin(115200);

  dht.begin();

  BLEDevice::init("ESP32_BLE_DHT11");

  BLEServer *server = BLEDevice::createServer();

  BLEService *service =
      server->createService(SERVICE_UUID);

  tempCharacteristic =
      service->createCharacteristic(
          TEMP_UUID,
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);

  humCharacteristic =
      service->createCharacteristic(
          HUM_UUID,
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);

  timeCharacteristic =
      service->createCharacteristic(
          TIME_UUID,
          BLECharacteristic::PROPERTY_READ |
          BLECharacteristic::PROPERTY_NOTIFY);

  // Add descriptors
  tempCharacteristic->addDescriptor(new BLE2902());
  humCharacteristic->addDescriptor(new BLE2902());
  timeCharacteristic->addDescriptor(new BLE2902());

  service->start();

  BLEAdvertising *advertising =
      BLEDevice::getAdvertising();

  advertising->addServiceUUID(SERVICE_UUID);
  advertising->start();

  Serial.println("BLE Sensor Started...");
}

void loop() {

  float temperature = dht.readTemperature();
  float humidity = dht.readHumidity();

  unsigned long timestamp = millis() / 1000;

  if (!isnan(temperature) && !isnan(humidity)) {

    tempCharacteristic->setValue(String(temperature).c_str());
    tempCharacteristic->notify();

    humCharacteristic->setValue(String(humidity).c_str());
    humCharacteristic->notify();

    timeCharacteristic->setValue(String(timestamp).c_str());
    timeCharacteristic->notify();

    Serial.print("Temp: ");
    Serial.print(temperature);
    Serial.print(" °C   ");

    Serial.print("Humidity: ");
    Serial.print(humidity);
    Serial.print("%   ");

    Serial.print("Time: ");
    Serial.println(timestamp);
  }

  delay(3000);
}
