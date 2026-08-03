# Wi-Fi Security for IoT (Research Write-up)

I put together this research document to analyze the primary Wi-Fi attack vectors targeting IoT devices, outline best practices for hardening ESP32 wireless connections, and highlight the security flaws inherent in hardcoding sensitive credentials into firmware.

---

## 1. Common Wi-Fi Attack Vectors on IoT Devices
Legacy protocol limitations leave many IoT devices open to serious network exploits:
* **WPA2 Vulnerabilities:** While WPA2 is widely used, it is inherently vulnerable to offline dictionary attacks if an attacker intercepts the initial 4-way handshake. Furthermore, flaws like the KRACK (Key Reinstallation Attacks) exploit allow attackers to decrypt network traffic by manipulating cryptographic keys during the handshake process.
* **Evil Twin Attacks:** Attackers can deploy a fraudulent access point broadcasting the exact same SSID as the legitimate network. Because many microcontrollers lack rigorous network authentication logic, they blindly connect to the strongest available signal, allowing malicious actors to intercept data via man-in-the-middle (MitM) positioning.

## 2. Best Practices for Securing ESP32 Connections
To mitigate these risks, modern firmware implementations should enforce robust encryption and validation standards:
* **Transition to WPA3:** Upgrading to WPA3 leverages Simultaneous Authentication of Equals (SAE), which effectively neutralizes traditional dictionary and handshake interception attacks.
* **Certificate Pinning:** When establishing HTTPS connections, the ESP32 should always be configured to validate the target server's SSL/TLS root certificate. Pinning specific server certificates guarantees that the device rejects connections to spoofed networks, effectively neutralizing MitM or Evil Twin threats.

## 3. The Danger of Hardcoding Passwords
Hardcoding network credentials directly into IoT source code introduces massive security vulnerabilities. Microcontroller firmware is frequently stored in plaintext SPI flash memory, making it trivial for unauthorized actors with physical access to dump the binary data and extract raw Wi-Fi passwords. Additionally, pushing hardcoded secrets to version control systems like GitHub opens the door to widespread public credential leaks.

---

## References
1. **Wi-Fi Alliance.** *WPA3 Security Whitepaper: Advanced Protections for Wi-Fi Networks.*
2. **Espressif Systems.** *ESP-IDF Security Guide: Transport Layer Security (TLS) and Flash Encryption.*
