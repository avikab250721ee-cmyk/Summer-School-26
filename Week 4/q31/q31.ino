/*
 * Project: Bluetooth LED Controller (HC-05 + Arduino)
 * Question: Q31
 * Author: [Shashwat Singh]
 */

#include <SoftwareSerial.h>

SoftwareSerial BT(10, 11);   // RX, TX

const int RED_LED = 3;
const int GREEN_LED = 4;
const int BLUE_LED = 5;

void setup() {
  pinMode(RED_LED, OUTPUT);
  pinMode(GREEN_LED, OUTPUT);
  pinMode(BLUE_LED, OUTPUT);

  Serial.begin(9600);
  BT.begin(9600);

  Serial.println("Bluetooth LED Controller Ready");
}

void loop() {

  if (BT.available()) {

    char command = BT.read();

    switch (command) {

      case 'R':
        digitalWrite(RED_LED, HIGH);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, LOW);
        BT.println("Red LED ON");
        break;

      case 'G':
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, HIGH);
        digitalWrite(BLUE_LED, LOW);
        BT.println("Green LED ON");
        break;

      case 'B':
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, HIGH);
        BT.println("Blue LED ON");
        break;

      case 'X':
        digitalWrite(RED_LED, LOW);
        digitalWrite(GREEN_LED, LOW);
        digitalWrite(BLUE_LED, LOW);
        BT.println("All LEDs OFF");
        break;

      case 'F':
        for (int i = 0; i < 5; i++) {
          digitalWrite(RED_LED, HIGH);
          digitalWrite(GREEN_LED, HIGH);
          digitalWrite(BLUE_LED, HIGH);
          delay(300);

          digitalWrite(RED_LED, LOW);
          digitalWrite(GREEN_LED, LOW);
          digitalWrite(BLUE_LED, LOW);
          delay(300);
        }
        BT.println("Flash Complete");
        break;

      default:
        BT.println("Invalid Command");
        break;
    }
  }
}
