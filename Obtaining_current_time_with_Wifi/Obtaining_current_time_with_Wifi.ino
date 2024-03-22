/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/epoch-unix-time-esp32-arduino/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// Same code works for ESP32-S3-DevkitC-1


// need to turn off phone Wifi and only connected to data, otherwise the hotspot sharing won't work


#include <WiFi.h>
#include "time.h"

// Replace with your network credentials
const char* ssid = "Chen Huawei";
const char* password = "00000000";


// NTP server to request epoch time
const char* ntpServer = "pool.ntp.org";

// Variable to save current epoch time
time_t epochTime; 

// Function that gets current epoch time
time_t getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

// Initialize WiFi
void initWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi ..");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(WiFi.localIP());
}

void setup() {
  Serial.begin(115200);
  initWiFi();
  configTime(0, 0, ntpServer); //?
}

void loop() {
  epochTime = getTime();
  Serial.print("Epoch Time: ");
  Serial.print(epochTime);
  Serial.print("\t");
  Serial.println(ctime(&epochTime));
  delay(1000);
}
