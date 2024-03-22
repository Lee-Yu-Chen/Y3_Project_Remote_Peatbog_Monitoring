#include <Wire.h>
#include <Adafruit_AHTX0.h>
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#define I2C_SDA 20
#define I2C_SCL 21
#define BUTTON_PIN 47

#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  3        /* Time ESP32 will go to sleep (in seconds) */

struct metalCore 
{
  const int PIN;
  double onTime;
  bool pressed;
};

///locations of GPIO pins 
metalCore low_Core = {4, 0, false};  //pins to be changed 
metalCore mid_Core = {5, 0, false};
metalCore top_Core = {6, 0, false}; 

RTC_DATA_ATTR int bootCount = 0;
Adafruit_AHTX0 aht;
int GREEN_LED_PIN = 18;
//int YELLOW_LED_PIN = 17;
int POWER_ON_LC = 17;
int PULSE_CORE_PIN = 15; //to be changed when we know the pin 
long low_time = 0;
long last_low_time = 0;
long mid_time = 0;
long last_mid_time = 0;
long top_time = 0;
long last_top_time = 0;
uint8_t id = 0x28;                   // i2c address
uint8_t data[7];                     // holds output data
uint8_t cmd[3] = {0xAA, 0x00, 0x00}; // command to be sent
double press_counts = 0;             // digital pressure reading [counts]
double temp_counts = 0;              // digital temperature reading [counts]
double pressure = 0;                 // pressure reading [bar, psi, kPa, etc.]
double temperature = 0;              // temperature reading in deg C
//double humidity = 0;
double outputmax = 15099494;         // output at maximum pressure [counts]
double outputmin = 1677722;          // output at minimum pressure [counts]
double pmax = 1;                     // maximum value of pressure range [bar, psi, kPa, etc.]
double pmin = 0;                     // minimum value of pressure range [bar, psi, kPa, etc.]
double percentage = 0;               // holds percentage of full scale data
char printBuffer[200], cBuff[20], percBuff[20], pBuff[20], tBuff[20];

void IRAM_ATTR lowTimer()
{
  low_time = micros();
  low_Core.onTime = (low_time - last_low_time);
  low_Core.pressed = true;
  last_low_time = low_time;
}
void IRAM_ATTR midTimer()
{
  mid_time = micros();
  mid_Core.onTime = (mid_time - last_mid_time);
  mid_Core.pressed = true;
  last_mid_time = mid_time;
}
void IRAM_ATTR topTimer()
{
  top_time = micros();
  top_Core.onTime = (top_time - last_top_time);
  top_Core.pressed = true;
  last_top_time = top_time;
}






double lowCoreTime()
{
  if (low_Core.pressed)
  {
    double returnVal = low_Core.onTime;
    Serial.printf("Low core time : %f\n", returnVal);
    low_Core.pressed = false;
    low_Core.onTime = 0;
    return(returnVal);
  }
}


double midCoreTime()
{
  if (mid_Core.pressed)
  {
    double returnVal = mid_Core.onTime;
    Serial.printf("Mid core time : %f\n", returnVal);
    mid_Core.pressed = false;
    mid_Core.onTime = 0;
    return(returnVal);
  }
}



double topCoreTime()
{
  if (top_Core.pressed)
  {
    double returnVal = top_Core.onTime;
    Serial.printf("Top core time : %f\n", returnVal);
    top_Core.pressed = false;
    top_Core.onTime = 0;
    return(returnVal);
  }
}



void temperatureSensor()
{
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");
//  return(temp);
}




void pressureSensor()
{
  Wire.beginTransmission(id);
    int stat = Wire.write(cmd, 3); // write command to the sensor
    stat |= Wire.endTransmission();
    delay(10);
    Wire.requestFrom(id, 7); // read back Sensor data 7 bytes
    for (int i = 0; i < 7; i++)
    {
        data[i] = Wire.read();
    }
    press_counts = data[3] + data[2] * 256 + data[1] * 65536; // calculate digital pressure counts
    temp_counts = data[6] + data[5] * 256 + data[4] * 65536;  // calculate digital temperature counts
    temperature = (temp_counts * 200 / 16777215) - 50;        // calculate temperature in deg c
    percentage = (press_counts / 16777215) * 100;             // calculate pressure as percentage of full scale
    // calculation of pressure value according to equation 2 of datasheet
    pressure = ((press_counts - outputmin) * (pmax - pmin)) / (outputmax - outputmin) + pmin;
    dtostrf(press_counts, 4, 1, cBuff);
    dtostrf(percentage, 4, 3, percBuff);
    dtostrf(pressure, 4, 4, pBuff);
    dtostrf(temperature, 4, 2, tBuff);

    Serial.print("Pressure percentage: "); Serial.println(percentage);
    Serial.print("Pressure: "); Serial.print(pressure); Serial.println(" bar");
    //sprintf(printBuffer, "Pressure percentage: %s \n Pressure output: %s \n Temperature from pressure: %s \n", percBuff, pBuff, tBuff);
    //Serial.print(printBuffer);
}



int logData(float temperature, float height, int pressure, float humidity){

    File f = SD.open("/Data.txt", FILE_APPEND);
    //File f = SD.open("/Data.txt", FILE_WRITE);
    int succ;


    f.print("T = ");
    f.print(temperature);
    f.print("C, ");

    f.print("H = ");
    f.print(height);
    f.print("m, ");

    f.print("P = ");
    f.print(pressure);
    f.print("Pa, ");

    f.print("Hu = ");
    f.print(humidity);
    succ=f.print("%,     \n");
    
    /*
    epochTime = getTime();
    Serial.print("EpochTime : ");
    Serial.print(epochTime);
    Serial.print("\t");
    Serial.println(ctime(&epochTime));
    succ = f.print(ctime(&epochTime));
    */
    
    f.close();

    return succ;
  }


  
void setup() {

  Serial.begin(115200);
  SPIClass fspi(FSPI);
  fspi.begin(12, 13, 11, 10);  // sck, miso, mosi, ss
  pinMode(10, OUTPUT); // sspin
  pinMode(GREEN_LED_PIN, OUTPUT);
  //pinMode(YELLOW_LED_PIN, OUTPUT);
  pinMode(PULSE_CORE_PIN, OUTPUT);
  pinMode(POWER_ON_LC, OUTPUT);
  pinMode(low_Core.PIN, INPUT);
  pinMode(mid_Core.PIN, INPUT);
  pinMode(top_Core.PIN, INPUT);
  attachInterrupt(low_Core.PIN, lowTimer, CHANGE);
  attachInterrupt(mid_Core.PIN, midTimer, CHANGE);
  attachInterrupt(top_Core.PIN, topTimer, CHANGE);
  Wire.begin(I2C_SDA, I2C_SCL);
  //sprintf(printBuffer, "\nStatus Register, 24 - bit Sensor data, Digital Pressure Counts,\
    Percentage of full scale pressure, Pressure Output, Temperature\n");
  //Serial.println(printBuffer);
  delay(500);

  if(!SD.begin(10, fspi))
  {
        Serial.println("Card Mount Failed");   
  }
  if (bootCount == 0) //Run this only the first time
  {
    //digitalWrite(YELLOW_LED_PIN, HIGH);
    bootCount = bootCount + 1;
  }
  if (! aht.begin())
  {
    Serial.println("Could not find AHT? Check wiring");
  }
  else  //loop for all code to go into
  {
    digitalWrite(POWER_ON_LC,HIGH);
    digitalWrite(PULSE_CORE_PIN,HIGH);
    delay(3);
    lowCoreTime();
    midCoreTime();
    topCoreTime();
    digitalWrite(PULSE_CORE_PIN,LOW);
    //digitalWrite(GREEN_LED_PIN, HIGH);
    temperatureSensor();
    pressureSensor();
    //logdata
  }


  //delay amount for deepsleep
  delay(3000);

  digitalWrite(GREEN_LED_PIN, LOW);
  //digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(PULSE_CORE_PIN, LOW);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();
}

void loop() {
}
