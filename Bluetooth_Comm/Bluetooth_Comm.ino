//This example code is in the Public Domain (or CC0 licensed, at your option.)
//By Evandro Copercini - 2018
//
//This example creates a bridge between Serial and Classical Bluetooth (SPP)
//and also demonstrate that SerialBT have the same functionalities of a normal Serial

#include "BluetoothSerial.h"

#if !defined(CONFIG_BT_ENABLED) || !defined(CONFIG_BLUEDROID_ENABLED)
#error Bluetooth is not enabled! Please run `make menuconfig` to and enable it
#endif

BluetoothSerial SerialBT;

int LED1 = 25;
int LED2 = 26;

void setup() {
  Serial.begin(2000000);
  SerialBT.begin("ESP32"); //Bluetooth device name
  Serial.println("The device started, now you can pair it with bluetooth!");

  pinMode(LED1,OUTPUT);
  pinMode(LED2,OUTPUT);
}

void loop() {
  if (Serial.available()) {
    SerialBT.write(Serial.read());
    digitalWrite(LED1,1);
  }
  if (SerialBT.available()) {
    Serial.println(SerialBT.readStringUntil('\n'));
    digitalWrite(LED2,1);
  }
  //delay(20);
  digitalWrite(LED1,0);
  digitalWrite(LED2,0);
  
}
