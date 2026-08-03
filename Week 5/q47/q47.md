# Q47: Heart Rate & SpO2 Simulation Monitor Node

I developed this healthcare telemetry processing node to simulate vital sign tracking, evaluate sensor noise isolation algorithms, and run fail-safe threshold alarms.

*Disclaimer: This system is a simulation framework engineered strictly for educational sandbox prototyping and hardware learning. It is not an active medical diagnostic instrument.*

---

## 📋 Features & Technical Specifications
* **Analog Vital Proxy System:** Interfaces with an external proxy input channel mapping raw voltage changes to vital ranges: **Heart Rate (40–140 BPM)** and **Oxygen Saturation (88%–100% SpO2)**.
* **5-Sample Signal Smoothing Window:** Implements a rolling-average array filter memory block. This smooths out incoming telemetry fluctuations and prevents accidental alarm triggers caused by erratic sensor movement or physical noise.
* **Dual Parameter Edge Alert System:** Evaluates vital metrics instantly to trigger structural red LED warning lights and piezo buzzers based on safe operating limits:
  * $\text{Heart Rate} < 50 \text{ BPM}$ or $> 120 \text{ BPM}$
  * $\text{Oxygen Saturation (SpO2)} < 94\%$
* **Non-Blocking Execution Clock:** Programmed entirely using `millis()` timing loops to handle sensor pooling, filtering, logging, and alarms concurrently without using processor-stalling `delay()` states.
* **CSV Infrastructure Data Logging:** Outputs well-formed telemetry metrics matching the local project storage file schema layout parameters (`/data/health_log.csv`).

---

## 🛠️ Bill of Materials (BOM)
* **Microcontroller Unit** (ESP32 / Arduino Compatible Node)
* **LDR Sensor** or **10kΩ Potentiometer** (Vitals parameter proxy simulation engine)
* **10kΩ Fixed Resistor** (Voltage divider line if utilizing an LDR module component)
* High-Brightness Alert **LED** (Red warning element)
* Low-Current **Active Piezo Buzzer**
* 2x **220Ω Current Limiting Resistors**
* Solderless breadboard and multi-colored hookup jumper wire configurations

---

## ⚡ Wokwi Physical Circuit Interconnect Wire Map

```text
       +-----------------------------------------------------------+
       |                     Microcontroller MCU                   |
       +-----------------------------------------------------------+
         [3V3]   [5V]   [GND]     [Pin A0]   [Pin 14]   [Pin 12]
           |       |      |          |          |          |
           +---|---|------|----------+          |          |
               +---|------|---------------------+          |
                   +------|--------------------------------+
                          |                                
                    +-----------+                          
                    |LDR Module |                          
                    +-----------+                          
                     [AnalogOut]
