# Q44: PIR-Based Security Camera Trigger & Alert System

I built this low-cost, multi-tier security alert system designed for a small storefront commercial retail layout in Rohtak, Haryana, India. 

The system provides localized physical deterrence and over-the-air threat notification. It continuously screens for physical movement during configured closing hours, dynamically escalating acoustic and visual signaling patterns if intrusion events persist, while instant alert messages are dispatched to store managers via a automated Telegram Bot API link.

---

## 📋 Features & Technical Specifications
* **Sensitivity-Adjustable PIR Tracking:** Implements human motion tracking utilizing a **PIR sensor** cross-referenced against a **potentiometer mapping**, letting operators adjust detection sensitivity margins directly.
* **3-Stage Non-Blocking Escalation:** Protects against false positives via an explicit temporal threat escalation state-machine:
  * **Warning (Stage 1):** Intermittent slow audio pulse + single flashing LED channel.
  * **Alarm (Stage 2):** Alternating high-low audio frequencies + twin chasing LED channels.
  * **Urgent (Stage 3):** Piercing emergency tone sweeps + maximum velocity sequential strobing.
* **Dynamic Time-Window Configurations:** Features runtime configuration updates parsing live string inputs over the hardware Serial interface (e.g., typing `SET_HOURS 22 06` alters security active windows instantly without re-flashing).
* **Network Telegram Bot Integrations:** Integrates a secure **HTTPS client connection** to push real-time alerts directly into a dedicated private Telegram chat group upon intrusion triggers.
* **Audit Trail Time Logging:** Prints structured, timestamped system logs directly to the Serial port detailing state adjustments.

---

## 🛠️ Hardware Inventory
* **ESP32** NodeMCU System Development Board
* **PIR Motion Sensor** (e.g., HC-SR501)
* **Rotary Potentiometer** (10kΩ for sensitivity calibration)
* **Active Piezo Buzzer**
* 3x High-Brightness **LEDs** (Red/Yellow/Blue to simulate warning strobes)
* 3x **220Ω Current Limiting Resistors**
* Solderless breadboard and connection wire arrays

---

## ⚡ Wokwi Physical Interconnect Schematic Diagram

```text
       +-----------------------------------------------------------+
       |                         ESP32 MCU                         |
       +-----------------------------------------------------------+
         [3V3]   [5V]   [GND]   [GPIO 13] [GPIO 34] [GPIO 12] [GPIO 14] [GPIO 27] [GPIO 26]
           |       |      |         |         |         |         |         |         |
           +---|---|------|---------+         |         |         |         |         |
               +---|------|-------------------+         |         |         |         |
                   +------|-----------------------------|----+----+----+----+----+
                          |                             |    |    |    |    |
                    +-----------+                       |    |    |    |    |
                    |PIR Sensor |                       |    |    |    |    |
                    +-----------+                       |    |    |    |    |
                     [SignalOut]                        |    |    |    |    |
                                                        |    |    |    |    |
                                                  +-----------+   |    |    |    |
                                                  |Active Buzz|   |    |    |    |
                                                  +-----------+   |    |    |    |
                                                   [Signal In]    |    |    |    |
                                                                  v    v    v    v
                                                                [LED1][LED2][LED3]
