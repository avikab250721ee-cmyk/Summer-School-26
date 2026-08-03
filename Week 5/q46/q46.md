# Q46: River Water Level Alert System (Flood Detection)

I engineered and configured this low-cost IoT flood-early-warning sensor node to respond to the frequent seasonal flash flooding vulnerabilities around the Tawi and Chenab river basin networks in Jammu & Kashmir, India.

The physical apparatus tracks rising water parameters inside a glass evaluation tank model using non-contact acoustic echo timing. The system provides real-time protective oversight through local alert escalations and a browser-based visualization dashboard that streams live telemetry over non-blocking WebSocket connections.

---

## 📋 Features & Technical Accomplishments
* **Ultrasonic Telemetry Tracking:** Maps changes in river water elevation using an **HC-SR04 ultrasonic sensor module** mounted overhead ($Distance \downarrow = Water\ Level \uparrow$).
* **10-Sample Rolling Average Filter:** Uses an array buffer to calculate a running average of incoming raw echoes. This dampens structural noise caused by surface ripples, cross-current splashing, or floating debris.
* **3-Tier Alert System State-Machine:** Implements specific, non-blocking visual/audio signaling profiles corresponding to safety parameters:
  * **Normal Status (>30cm clearance):** Solid Green indicator.
  * **Warning Status (15cm–30cm clearance):** Mid-frequency flashing yellow toggle + synchronous acoustic alert chirps.
  * **Critical Status (<15cm clearance):** Maximum velocity emergency flash strobing + rapid high-frequency siren sweep.
* **Dual Wire-Free Broadcasting Pipeline:** Drops heavy overhead connection stacks to simultaneously transmit basic string logs over the hardware link while updating an open **WebSocket channel client server (Port 81)**.
* **Live Chart.js UI Web Dashboard:** Features an external dashboard interface engine built using vanilla JavaScript, HTML5 canvas animations, and **Chart.js** to map and plot water level histories in real time.

---

## 🛠️ Bill of Materials (BOM)
* **ESP32** NodeMCU System Development Board
* **HC-SR04** Ultrasonic Distance Sensor Module
* High-Brightness Status **LED**
* **Active Piezo Buzzer**
* 2x **220Ω Current Limiting Resistors**
* Solderless breadboard and connection jumper wires

---

## ⚡ Wokwi Physical Circuit Interconnect Wire Map

```text
       +-----------------------------------------------------------+
       |                         ESP32 MCU                         |
       +-----------------------------------------------------------+
         [3V3]   [5V]   [GND]   [GPIO 12] [GPIO 14] [GPIO 26] [GPIO 27]
           |       |      |         |         |         |         |
           |       +------|---------+---------+         |         |
           +--------------|-----------------------------|----+----+
                          |                             |    |    |
                    +-----------+                       |    |    |
                    |  HC-SR04  |                       |    |    |
                    +-----------+                       |    |    |
                     [Trig][Echo]                       |    |    |
                                                        v    v    v
                                                      [BUZ][LED][RES]
