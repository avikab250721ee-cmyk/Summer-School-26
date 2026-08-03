# Q45: Smart Greenhouse Climate Controller (Floriculture IoT)

I built this multi-parameter microclimate controller node using an ESP32 to automate environmental conditions inside a commercial floriculture greenhouse plot cultivating tulips and roses in Kashmir, India. 

The primary target profiles involve maintaining critical agricultural windows: **Temperature (18°C–22°C)**, **Humidity (60%–70%)**, and ensuring optimal lux parameter support. To achieve stable industrial tracking, the firmware manages peripherals using a software smoothing filter algorithm and a bang-bang operational state machine equipped with custom hysteresis deadbands.

---

## 📋 Features & Technical Specifications
* **Dual-Core Climate Analysis:** Tracks real-time ambient values securely using a **DHT11 sensor**.
* **10-Sample Rolling Light Average:** Implements a rolling-window array calculation filter across the raw analog inputs of an **LDR sensor**. This prevents rapid lighting toggles caused by fleeting cloud silhouettes or passing shadows.
* **Hysteresis Control Logic:** Implements a simple bang-bang control loop featuring software guardrails ($\pm0.5^\circ\text{C}$ temperature deadband / $\pm1.0\%$ relative humidity barrier). This shields the relay array from high-frequency switching or jitter right at threshold limits.
* **3-Channel Actuator Output Simulation:** Manages high-voltage loads (simulated via status LED channels):
  * **Heater Line** Actuated if temperature flags drop below optimal limits.
  * **Ventilation Fan Line** Energized if metrics exceed upper limits (over-humidity or overheating conditions).
  * **Supplemental Grow Lights** Driven active during overcast sky drops or dusk transitions.
* **3-Second Active Dashboard Sequencer:** Manages data presentation cleanly on a physical **16x2 I2C LCD Display**, auto-cycling through distinct formatted screens every 3 seconds without loop block timers.
* **Cloud Platform Broker Node (MQTT):** Features live data parsing wrapped inside an asynchronous JSON string container, published every 5 seconds to a remote MQTT broker cloud dashboard node (+Bonus Mark).

---

## 🛠️ Hardware Requirements
* **ESP32** NodeMCU System Development Board
* **DHT11** Digital Temperature & Humidity Sensor
* **LDR (Light Dependent Resistor)** Photoresistor
* 1x **10kΩ Resistor** (Voltage divider line setup for LDR)
* 3x Low-Current Signal **LEDs** (Red, Blue, White to map Relay states)
* 3x **220Ω Current Limiting Resistors**
* **16x2 LCD Character Panel** (paired with an I2C PCF8574 adapter module board)
* Prototyping breadboard and layout interconnect jumper wires

---

## ⚡ Wokwi Physical Circuit Interconnect Wire Map

```text
       +-----------------------------------------------------------+
       |                         ESP32 MCU                         |
       +-----------------------------------------------------------+
         [3V3]   [5V]   [GND]   [GPIO 4]  [GPIO 34] [GPIO 14] [GPIO 27] [GPIO 26]
           |       |      |         |         |         |         |         |
           +---|---|------|---------+         |         |         |         |
               +---|------|-------------------+         |         |         |
                   +------|-----------------------------|----+----+----+----+
                          |                             |    |    |    |
                    +-----------+                       |    |    |    |
                    |DHT11 Data |                       |    |    |    |
                    +-----------+                       |    |    |    |
                                                        v    v    v    v
                                                     [LDR] [HTR][FAN][LGT]
                                                    (Analog)   (Relay Outputs)
