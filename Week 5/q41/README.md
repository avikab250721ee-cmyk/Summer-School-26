# Q41: Smart Home Air Quality Monitor (Edge-Resilient System)

I designed and developed this ESP32-based indoor air quality monitoring system to track and manage environment metrics inside a residential home in Milan, Europe. 

The primary objective is safeguarding a household during winter months when wood-burning heating increases exposure risks to elevated carbon monoxide (CO) and smoke levels. To guarantee complete environmental protection, I structured the system around a strict **edge-resilient architecture**—ensuring safety routines, multi-tone audio warnings, and status indicators continue working perfectly locally even if the home internet router completely crashes.

---

## 📋 System Requirements & Accomplishments
* **Gas Integration:** Polling and calibration logic for an **MQ-2 gas/smoke sensor** utilizing an analog three-tier threshold analysis.
* **Climate Logging:** Local microclimate tracking for ambient parameters via a **DHT11 sensor**.
* **3-Tier Visual Indication:** Full-state custom RGB LED mapping: **Green** (Safe), **Yellow** (Moderate), and **Red** (Danger).
* **Escalated Acoustic Alerts:** Dedicated piezo buzzer producing 3 unique frequency/duration tones depending on environment severity.
* **Local Logging Pipeline:** Real-time data structuring output directly to the Serial Monitor in clean **CSV format**.
* **Wireless Backup Link:** Added fallback data mirroring and text notifications pushed directly to a mobile application over **Classic Bluetooth** (+2 marks).

---

## 🛠️ Hardware Inventory
* **ESP32** NodeMCU Development Module
* **MQ-2** Gas & Smoke Detection Sensor
* **DHT11** Temperature & Humidity Sensor
* **RGB LED** (Common Cathode module)
* **Active Piezo Buzzer**
* 3x **220Ω Resistors** (Current limitation for the RGB channels)
* Breadboard and jumper connections

---

## ⚡ Wokwi Physical Circuit Schematic Diagram

This is the physical wiring mapping I followed to establish hardware-level connectivity across the ESP32 pins:

```text
   +--------------------------------------------------------+
   |                       ESP32 MCU                        |
   +--------------------------------------------------------+
       |       |       |          |          |          |          |          |
     [3.3V]   [5V]   [GND]     [GPIO 4]   [GPIO 34]  [GPIO 14]  [GPIO 27]  [GPIO 12]  [GPIO 13]
       |       |       |          |          |          |          |          |          |
       +-------|-------|-----+    |          |          |          |          |          |
       |       +-------|-----|----+          |          |          |          |          |
       |       |       +-----|----|----------|----------|----------|----------|----+     |
       |       |       |     |    |          |          |          |          |    |     |
   +-------+   |       |     |    |          |          |          |          |    |     |
   | DHT11 |   |       |     |    |          |          |          |          |    |     |
   +-------+   |       |     |    |          |          |          |          |    |     |
    [VCC]------+       |     |    |          |          |          |          |    |     |
    [DATA]-------------|-----|----+          |          |          |          |    |     |
    [GND]--------------+     |               |          |          |          |    |     |
                             |               |          |          |          |    |     |
                         +-------+           |          |          |          |    |     |
                         |  MQ-2 |           |          |          |          |    |     |
                         +-------+           |          |          |          |    |     |
                          [VCC]--------------+          |          |          |    |     |
                          [A0]--------------------------+          |          |    |     |
                          [GND]------------------------------------+          |    |     |
                                                                              |    |     |
                                                                          +-------+|     |
                                                                          |RGB LED||     |
                                                                          +-------+|     |
                                                                           [R]-----+     |
                                                                           [G]-----------+
                                                                           [B]-----------+
                                                                                         |
                                                                                     +-------+
                                                                                     |BUZZER |
                                                                                     +-------+
                                                                                      [POS]--+
