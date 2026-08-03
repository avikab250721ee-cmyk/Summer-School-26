# Bluetooth Sensor Dashboard (ESP32 BLE)

## Overview

This project uses an **ESP32** and a **DHT11 temperature/humidity sensor** to broadcast environmental data over **Bluetooth Low Energy (BLE)** every **3 seconds**.

The sensor data can be viewed using mobile applications such as:

- nRF Connect
- LightBlue

---

# Hardware Required

- ESP32 Development Board
- DHT11 Sensor
- Breadboard
- Jumper Wires
- Android/iPhone with BLE Scanner App

---

# Wiring

| DHT11 | ESP32 |
|-------|-------|
| VCC | 3.3V |
| GND | GND |
| DATA | GPIO4 |

---

# BLE Service

**Service UUID**

```
12345678-1234-1234-1234-123456789000
```

---

# Characteristics

| Characteristic | UUID | Property |
|---------------|------|----------|
| Temperature | 12345678-1234-1234-1234-123456789001 | Read + Notify |
| Humidity | 12345678-1234-1234-1234-123456789002 | Read + Notify |
| Timestamp | 12345678-1234-1234-1234-123456789003 | Read + Notify |

All characteristics include a **BLE2902 Client Characteristic Configuration Descriptor (CCCD)** to support notifications.

---

# How to Test

1. Upload the sketch to ESP32.
2. Enable Bluetooth.
3. Open **nRF Connect** or **LightBlue**.
4. Scan for:

```
ESP32_BLE_DHT11
```

5. Connect to the device.
6. Open the BLE Service.
7. Enable Notifications.
8. Temperature, Humidity, and Timestamp update every **3 seconds**.

---

# Expected Output

Example values:

```
Temperature : 28.6 °C

Humidity : 65 %

Timestamp : 42 s
```

---

# Project Structure

```text
week4/
└── ble_sensor/
    ├── ble_sensor.ino
    └── README.md
```

---

# Libraries Required

- ESP32 BLE Arduino
- DHT sensor library by Adafruit
- Adafruit Unified Sensor

---

# Author

**Name:** Shashwat Singh

**Course:** IoT Summer School 2026

**License:** MIT
