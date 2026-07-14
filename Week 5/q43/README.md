# Q43: Smart Door Lock System with Bluetooth OTP

I designed and developed this secure, keyless access control framework for university hostel entry gates using an ESP32 microcontroller. 

The system completely replaces mechanical hardware locks with a two-factor verification paradigm (User Identifier Code + One-Time Password). Students present their identity code via an edge keypad interface, generating a temporary 6-digit credential routed directly to their registered mobile device over a peer-to-peer Bluetooth connection.

---

## 📋 Features & Technical Specifications
* **Physical Interface Entry:** Decodes user sequences securely using a multi-line **4x4 Matrix Keypad**.
* **Masked Display Profiles:** Utilizes an **I2C 16x2 LCD Panel** displaying raw values during ID compilation, which automatically masks inputs with character placeholders (`*`) during high-security OTP fields.
* **Over-the-Air Token Delivery:** Broadcasts auto-generated tokens over **Classic Bluetooth** straight to terminal consoles on paired mobile phone devices.
* **Non-Blocking Expiry Loop:** Leverages internal `millis()` differentials to keep track of a strict **30-second token lifetime**, printing an updating countdown banner on screen (`T-30`) without stopping code execution.
* **Servo Lock Actuation:** Operates a **Servo Motor** sweeping 0° to 90° with software-managed automatic return timeouts to simulate latch mechanisms.
* **Audit Trail Security Logs:** Sends timestamped operational markers tracking access attempts (`LOG [SUCCESS]` or `LOG [FAILURE]`) via the hardware Serial line.

---

## 🛠️ Hardware Requirements
* **ESP32** Development Module
* Standard **4x4 Matrix Keypad**
* **16x2 LCD Display** (with embedded I2C interface backpack driver)
* High-Torque Micro **Servo Motor** (e.g., SG90)
* Prototyping breadboard and multi-colored jumper wires

---

## ⚡ Wokwi Physical Schematic Layout Diagram

```text
       +-----------------------------------------------------------+
       |                         ESP32 MCU                         |
       +-----------------------------------------------------------+
         [GND]   [5V]   [GND]  [GPIO 15]   [Row Pins]     [Col Pins]
           |       |      |        |       (13,12,14,27) (26,25,33,32)
           |   [I2C Power]         |             |             |
           |   (VCC)  (GND)        |             |             |
           |     |      |          |             v             v
     +-----------------------+     |       +-----------------------+
     |   16x2 LCD (I2C)      |     |       |       4x4 KEYPAD      |
     |   [SDA->G21, SCL->G22]|     |       |      MATRIX ARRAY     |
     +-----------------------+     |       +-----------------------+
                                   |
                             +-----------+
                             |SERVO MOTOR|
                             +-----------+
                              [Signal In]
