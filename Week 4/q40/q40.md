# MQTT Quality of Service (QoS) Levels (Theory & Application)

This repository contains a theoretical breakdown of MQTT Quality of Service (QoS) levels and evaluates which guarantee layer fits best within an automated IoT water level monitoring use case.

---

## 1. Understanding MQTT QoS Levels

Quality of Service (QoS) in MQTT dictates the level of agreement between a sender (client/broker) and a receiver (broker/client) regarding the delivery guarantee of a message.

### 🔹 QoS 0: At most once (Fire and Forget)
* **Mechanism:** The message is sent once over the network with no acknowledgment required from the receiver.
* **Reliability:** Lowest. Messages can be lost if the network drops or if the receiver is offline. 
* **Handshake:**
  ```text
  Sender --------------[ PUBLISH ]--------------> Receiver
