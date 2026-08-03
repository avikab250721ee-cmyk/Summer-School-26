# Q49: Security Vulnerability Analysis & Hardening

This repository contains a comprehensive security audit of a poorly written IoT firmware implementation running on an ESP8266 microcontroller. Following the vulnerability identification, a hardened, refactored implementation is provided to mitigate critical transport and credential exposure risks.

---

## 🛑 Part 1: Vulnerability Assessment Report

The legacy code architecture presented five high-severity flaws that left the IoT node exposed to unauthorized monitoring and tampering:

### 1. Hardcoded Credentials in Source Code
* **Risk Mapping:** Storing the Wi-Fi SSID and password directly as plain text variables means they are permanently baked into the source code. If this code is pushed to public repositories like GitHub, the credentials instantly leak. Furthermore, because firmware binaries on ESP8266 external flash memory chips are rarely encrypted by default, an attacker with physical access to the device can dump the SPI flash chip and easily extract the raw Wi-Fi password.

### 2. Unencrypted Traffic Transport (HTTP instead of HTTPS)
* **Risk Mapping:** Using standard HTTP over port 80 transmits all packet payloads—including data endpoints, device IDs, and proprietary telemetry—in clear plaintext over the air. Any actor positioned within wireless range can use a packet sniffer (like Wireshark) to execute a Man-in-the-Middle (MitM) attack, intercepting private user insights or tampering with the payload in transit.

### 3. Public MQTT Broker Use Without Authentication
* **Risk Mapping:** Publishing sensor measurements to a wide-open public MQTT broker without a custom username or an active SSL/TLS password means your data stream is visible to anyone. Anyone can connect to that same public broker, subscribe to your topic string, and continuously harvest your raw telemetry data.

### 4. Overly Permissive / Predictable MQTT Topic Formats
* **Risk Mapping:** Unauthenticated implementations typically use generic topic paths (e.g., `home/sensor/temp`). Attackers routinely run wildcard subscription listeners (`#`) against public brokers to discover active streams, find your device, and map out your backend application's data layout.

### 5. Absence of Inbound Command Validation 
* **Risk Mapping:** Lacking logic to parse, sanitize, or check incoming MQTT commands or HTTP payloads leaves the device vulnerable to command spoofing. A malicious actor can publish custom payloads to your control topics, tricking the device into executing unauthorized hardware overrides, resetting parameters, or entering a permanent crash loop.

---

## 🛠️ Part 2: Hardened Solution Architecture

To eliminate credential leakage and transport interception vulnerabilities, the code has been refactored to enforce **Secrets Isolation** and **Transport Layer Security (TLS 1.2)**.

### 🔐 1. Isolated Configuration File (`config.h`)
*Create this file locally within your project folder. **It is added to `.gitignore` so it will never be tracked or uploaded to GitHub.***

```cpp
#ifndef CONFIG_H
#define CONFIG_H

// Network SSID credentials isolated safely away from main repo source lines
const char* WIFI_SSID     = "Your_Secure_Office_SSID";
const char* WIFI_PASSWORD = "A_Strong_Complex_Password_123";

// SHA-1 Fingerprint for the target API server (Validates host identity to block MitM)
const char* SERVER_FINGERPRINT = "C6:2D:E2:E6:C3:57:9F:E5:A2:9A:89:E8:A4:7B:A2:64:1B:67:E6:E5"; 

#endif
