# Bluetooth LED Controller (HC-05 + Arduino)

## Overview

This project controls three LEDs using an HC-05 Bluetooth module connected to an Arduino UNO. Commands are sent from an Android phone using a Bluetooth Terminal application.

---

## Hardware Required

- Arduino UNO
- HC-05 Bluetooth Module
- 3 × LEDs (Red, Green, Blue)
- 3 × 220 Ω Resistors
- Breadboard
- Jumper Wires
- Android Phone

---

## Wiring

| HC-05 | Arduino UNO |
|--------|-------------|
| VCC | 5V |
| GND | GND |
| TXD | D10 (RX) |
| RXD | D11 (TX) |

### LED Connections

| LED | Arduino Pin |
|-----|-------------|
| Red | D3 |
| Green | D4 |
| Blue | D5 |

Each LED should be connected through a **220 Ω resistor** to GND.

---

## Bluetooth Commands

| Command | Action |
|---------|--------|
| **R** | Turn Red LED ON |
| **G** | Turn Green LED ON |
| **B** | Turn Blue LED ON |
| **X** | Turn OFF all LEDs |
| **F** | Flash all LEDs 5 times |

---

## Pairing Instructions

1. Power the Arduino.
2. Turn on Bluetooth on your Android phone.
3. Search for **HC-05**.
4. Pair using the default PIN:
   - **1234**
   - or **0000**
5. Open a Bluetooth Terminal app.
6. Connect to HC-05.
7. Send any of the commands listed above.

---

## Expected Output

- Sending **R** turns ON the red LED.
- Sending **G** turns ON the green LED.
- Sending **B** turns ON the blue LED.
- Sending **X** turns OFF all LEDs.
- Sending **F** flashes all LEDs five times.

---
