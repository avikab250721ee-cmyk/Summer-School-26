# Q42: Automated Saffron Irrigation System (Smart Horticulture)

I designed and implemented this automated smart-irrigation control system for an agricultural development plot cultivating saffron in Jammu & Kashmir, India. 

Because saffron crocus bulbs are highly sensitive to overwatering and susceptible to rot, the firmware relies on a dual-protection safety mechanism. It monitors structural soil metrics along with a combination of cloud weather APIs and local light-intensity thresholds (serving as weather proxies) to prevent active irrigation loops if local rainfall has occurred or is imminent.

---

## 📋 Technical Deliverables & Architecture
* **Saffron Moisture Zoning:** Programmed an analog evaluation matrix that categorizes real-time metrics into distinct operational brackets: **Dry**, **Optimal**, and **Excessively Wet**.
* **Dual-Layer Rain Prevention Proxy:** Bypasses irrigation sequences if either the local digital **LDR module** flags dense overcast skies, or the cloud-based **OpenWeatherMap API** forecasts rainfall in Pampore, J&K.
* **Non-Blocking Runtime Core:** Developed completely using `millis()` timing structures. This enables independent execution loops for the 30-minute sensor checks, 15-second pump runs, and manual overrides without stalling operations.
* **Farmer Override & Safety Lockout:** Integrated a hardware push button that allows instant system activation. It features an automated **5-minute hard lockout** to prevent inadvertent continuous overwatering.
* **16x2 LCD Dashboard:** Provides an active status screen tracking real-time moisture percentage, system state, and runtime metrics (*Last Watered Time* relative to system uptime).
* **Cloud Telemetry Pipeline:** Automatically packages system metrics into structured JSON payloads and relays them over a secure Wi-Fi connection to **Google Sheets via IFTTT Webhooks**.

---

## 🛠️ Bill of Materials (BOM)
* **ESP8266** NodeMCU Microcontroller Board
* **Capacitive/Resistive Soil Moisture Sensor**
* **LDR (Light Dependent Resistor)** Module with Digital Comparator Out
* **5V Relay Module** paired with an active DC Water Pump (Simulated with an LED)
* **16x2 LCD Character Display** (equipped with an I2C piggyback driver module)
* Tactile Push-Button Switch (Manual Override)
* Jumper wires, breadboard, and external 5V power supply source

---

## ⚡ Hardware Pin Interconnect Framework

```text
       +-------------------------------------------------------------+
       |                         ESP8266 MCU                         |
       +-------------------------------------------------------------+
         [A0]      [D0]      [D3]      [D5]      [D6]    [3V3]  [GND]
          |         |         |         |         |        |      |
          |         |         |         |         |   [I2C Power Bus]
          |         |         |         |         |     (VCC)  (GND)
    +-----------+   |         |         |         |       |      |
    |Soil Sensor|   |         |         |         |       |      |
    +-----------+   |         |         |         |       |      |
     [Signal Out]---+         |         |         |       |      |
                              |         |         |       |      |
                        +-----------+   |         |       |      |
                        |LDR Module |   |         |       |      |
                        +-----------+   |         |       |      |
                         [DigitalDO]----+         |       |      |
                                                  |       |      |
                                            +-----------+ |      |
                                            |Relay/Pump | |      |
                                            +-----------+ |      |
                                             [Signal In]--+      |
                                                                 |
                                                           +-----------+
                                                           |Push Button|
                                                           +-----------+
                                                            [Signal Out]
