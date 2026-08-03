# IoT Gateway Architecture & Protocols (Theory Write-up)

This repository covers the fundamental theory behind IoT Gateways, maps out the systemic relationships between edge sensors, gateway bridges, cloud platforms, and user applications, and details the protocols typically utilized at each layer.

---

## 1. What is an IoT Gateway?
An **IoT Gateway** acts as an intermediary bridge or proxy connecting local, low-power IoT sensor nodes to the broader internet and cloud infrastructure. 

Because many edge sensors communicate over specialized local protocols (like Bluetooth or Zigbee) to save battery, they cannot talk directly to standard internet servers. The gateway handles **protocol translation** (e.g., converting Zigbee data packets into MQTT over TCP/IP), data aggregation, filtering, and local preprocessing before passing relevant data up to the cloud.

---

## 2. Architecture Diagram

Below is the conceptual flow showing how data travels from the physical environment to the end user:

```text
+------------------+         +---------------+         +---------------+         +------------------+
|   IoT Sensor     |  Local  |  IoT Gateway  | Internet|  Cloud Platform| Internet| User Application |
|     Nodes        |-------->|    (Bridge)   |-------->|   (Broker/DB) |-------->|  (Web/Mobile App)|
| (DHT11, LDR, etc)| Protocol| (ESP32/Pi/etc)| Protocol| (AWS, Firebase)| Protocol| (Dashboard/GUI)  |
+------------------+         +---------------+         +---------------+         +------------------+
