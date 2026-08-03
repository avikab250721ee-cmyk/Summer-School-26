# Strategic IoT Infrastructure Proposal for Jammu Smart City

* **Document Path:** `/final_project/smart_city_proposal.md`
* **Target Layout Location:** Municipal Development Corporation, Jammu
* **Project Scope:** High-efficiency architecture for urban optimization across three target vectors.

---

## 1. Executive Summary & Architectural Overview
As Jammu transitions into a sustainable urban center, managing its public utilities requires transition from manual scheduling to real-time, data-driven edge processing. This proposal highlights an architectural framework to monitor and manage **Street Lighting**, **Waste Bin Fill Levels**, and **Parking Space Availability** across major city sectors (e.g., Gandhinagar, Rehari, and old Jammu city).

By deploying low-power, edge-resilient microcontrollers communicating over lightweight transport layers to an organized MQTT data broker core, the Jammu Municipal Administration can cut energy overheads, streamline refuse collection, and reduce traffic congestion.

---

## 2. Deep-Dive Vertical Engineering Layouts

### 💡 Sub-System A: Adaptive Smart Street Lighting
Manual timer switchboards often waste power during overcast weather or seasonal day-length shifts. This module introduces standalone illumination monitoring, allowing lamps to adjust their output based on ambient light levels.

* **Sensor Layer:** **LDR (Light Dependent Resistor)** photoresistor module paired inside a moisture-sealed voltage divider configuration.
* **Microcontroller & Connectivity Module:** **ESP32 NodeMCU Module** utilizing integrated **Wi-Fi / LoRaWAN** connectivity.
* **End-to-End Data Flow Pipeline:**
  $$\text{LDR Analog Read} \longrightarrow \text{ESP32 Threshold Analysis} \longrightarrow \text{MQTT Protocol (QoS 1)} \longrightarrow \text{Mosquitto Local Server Broker} \longrightarrow \text{Municipal Node-RED Dashboard}$$
* **Estimated Cost Per Node Deployment (INR):**
  * ESP32 Controller Module: ₹450
  * Industrial LDR + Acrylic Enclosure Housing: ₹150
  * Solid State Relay (SSR 25A): ₹250
  * **Total Estimated Node Investment:** **₹850**

---

### 🗑️ Sub-System B: Automated Waste Bin Fill Monitor
Traditional waste collection involves static, pre-scheduled pickup routes, which often leads to collection trucks servicing empty dumpsters while overfilled trash bins sit unaddressed. This tracking module turns urban dumpsters into smart endpoints that report their fill level on a regular schedule.

* **Sensor Layer:** **HC-SR04 / AJ-SR4E Water-Resistant Ultrasonic Range Transceiver Module** mounted safely at the inside ceiling point of the waste bin container.
* **Microcontroller & Connectivity Module:** **ESP32 DevKitC** running light deep-sleep power scaling to minimize battery drain.
* **End-to-End Data Flow Pipeline:**
  $$\text{Ultrasonic Timing Trigger} \longrightarrow \text{Clearance Calculus via MCU} \longrightarrow \text{CoAP/MQTT Data Burst} \longrightarrow \text{Central Cloud DB} \longrightarrow \text{Sanitation Fleet Route Optimization Panel}$$
* **Estimated Cost Per Node Deployment (INR):**
  * ESP32 Controller Board: ₹450
  * AJ-SR4E Rugged Waterproof Ultrasonic Sensor: ₹350
  * 3.7V Li-ion Battery Storage Module + Miniature Solar Recharger: ₹300
  * Heavy-Duty IP67 Sealed Protective Enclosure: ₹150
  * **Total Estimated Node Investment:** **₹1,250**

---

### 🚗 Sub-System C: Smart Parking Availability Matrix
Drivers cruising around looking for open parking spaces account for a significant portion of traffic bottlenecks in crowded markets like Raghunath Bazaar. This localized space monitoring solution uses low-power proximity nodes to provide drivers with real-time slot vacancy updates.

* **Sensor Layer:** **Active Infrared (IR) Proximity Obstacle Array** (or an industrial **Geomagnetic Inductive Loop Vehicle Detector Module** for heavy street applications).
* **Microcontroller & Connectivity Module:** **ESP32 Module** using persistent Bluetooth Low Energy (BLE) beaconing or localized Wi-Fi mesh structures.
* **End-to-End Data Flow Pipeline:**
  $$\text{IR Proximity Detection Line Check} \longrightarrow \text{Software Debounce State Extraction} \longrightarrow \text{MQTT Retained Message Push} \longrightarrow \text{Cloud Registry} \longrightarrow \text{Jammu Smart City Public Mobile App API}$$
* **Estimated Cost Per Node Deployment (INR):**
  * ESP32 Development Module: ₹450
  * High-Accuracy IR Proximity Probing Element: ₹100
  * Power distribution bus line hookups + Mounting Brackets: ₹150
  * **Total Estimated Node Investment:** **₹700**

---

## 3. Comparative Infrastructure Metric Matrix

| Urban Operational Category | Target Sensor Model | Core Network Layer | Estimated Unit Cost (INR) | Primary Operational Efficiency Gain |
| :--- | :--- | :--- | :--- | :--- |
| **Street Lighting** | LDR Photoresistor | MQTT over Wi-Fi/Mesh | ₹850 | Eliminates daytime power burn; cuts grid load by ~35%. |
| **Waste Management** | AJ-SR4E Ultrasonic | Intermittent MQTT Burst | ₹1,250 | Cuts diesel fuel waste by prioritizing full bins. |
| **Smart Parking** | IR Proximity Sensor | MQTT with Retained Flags | ₹700 | Lowers driver search times, easing traffic bottlenecks. |

---

## 4. Comprehensive Architectural Data Flow Diagram

```text
    [ EDGE NODE LAYER ]                  [ NETWORK LAYER ]               [ CENTRALIZED ENTERPRISE LAYER ]

+--------------------------+
|  Streetlight Node (LDR)  |───────┐
+--------------------------+       │
                                   │
+--------------------------+       ├──> [ MQTT Broker Server ] ───> [ Cloud Database ] ───> [ Central Control ]
|   Waste Bin Node (US)    |───────┤    (QoS 1 / Port 1883)        (Time-Series Influx)     (Node-RED Dashboard)
+--------------------------+       │
                                   │
+--------------------------+       │
|   Parking Spot Node (IR) |───────┘
+--------------------------+
