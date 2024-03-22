
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>

#include <Adafruit_AHTX0.h>




#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
#define TIME_TO_SLEEP  1        /* Time ESP32 will go to sleep (in seconds) */


#define I2C_SDA 20   // GREEN : SDA
#define I2C_SCL 21   // BLUE : SCL

Adafruit_AHTX0 aht;




uint8_t id = 0x28;                   // i2c address
uint8_t data[7];                     // holds output data
uint8_t cmd[3] = {0xAA, 0x00, 0x00}; // command to be sent
double press_counts = 0;             // digital pressure reading [counts]
double pressure = 0;                 // pressure reading [bar, psi, kPa, etc.]
double outputmax = 15099494;         // output at maximum pressure [counts]
double outputmin = 1677722;          // output at minimum pressure [counts]
double pmax = 1;                     // maximum value of pressure range [bar, psi, kPa, etc.]
double pmin = 0;                     // minimum value of pressure range [bar, psi, kPa, etc.]





struct metalCore 
{
  const int PIN;
  double onTime;
  bool pressed;
};

///locations of GPIO pins 
metalCore low_Core = {5, 0, false};  //pins to be changed 
metalCore mid_Core = {6, 0, false};
metalCore top_Core = {4, 0, false}; 
metalCore dummy_Core = {8, 0, false};

RTC_DATA_ATTR int bootCount = 0;

int POWER_ON_LC = 17;
int PULSE_CORE_PIN = 15; //to be changed when we know the pin 


long low_time = 0;
long last_low_time = 0;
long mid_time = 0;
long last_mid_time = 0;
long top_time = 0;
long last_top_time = 0;
long dummy_time = 0;
long last_dummy_time = 0;







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
void IRAM_ATTR dummyTimer()
{
  dummy_time = micros();
  dummy_Core.onTime = (dummy_time - last_dummy_time);
  dummy_Core.pressed = true;
  last_dummy_time = dummy_time;
}









double lowCoreTime()
{
  if (low_Core.pressed)
  {
    double returnValLow = low_Core.onTime;
    if (returnValLow > 100 && returnValLow < 500)
    {
    Serial.printf("Low core time : %f\n", returnValLow);
    }
    else{
      Serial.printf("Low Core failed : %f\n", returnValLow);
      returnValLow = -1;
    }
    low_Core.pressed = false;
    low_Core.onTime = 0;
    return(returnValLow);
  }
}




double midCoreTime()
{
  if (mid_Core.pressed)
  {
    double returnValMid = mid_Core.onTime;
    if (returnValMid > 100 && returnValMid < 500)
    {
    Serial.printf("Mid core time : %f\n", returnValMid);
    }
    else{
      Serial.printf("Mid Core failed : %f\n", returnValMid);
      returnValMid = -1;
    }
    mid_Core.pressed = false;
    mid_Core.onTime = 0;
    return(returnValMid);
  }
}




double topCoreTime()
{
  if (top_Core.pressed)
  {
    double returnValTop = top_Core.onTime;
    if ( returnValTop > 100 && returnValTop < 500)
    {
    Serial.printf("Top core time : %f\n", returnValTop);
    }
    else{
      Serial.printf("Top Core failed : %f\n", returnValTop);
      returnValTop = -1;
    }
    top_Core.pressed = false;
    top_Core.onTime = 0;
    return(returnValTop);
  }
}



double dummyCoreTime()
{
  if (dummy_Core.pressed)
  {
    double returnValdummy = dummy_Core.onTime;
    Serial.printf("Dummy core time : %f\n", returnValdummy);
    dummy_Core.pressed = false;
    dummy_Core.onTime = 0;
    return(returnValdummy);
  }
}










int logData(sensors_event_t temperature, int LowCore, int MidCore, int TopCore, float pressure, sensors_event_t humidity){

    
    File f = SD.open("/Data.txt", FILE_APPEND);
    //File f = SD.open("/Data.txt", FILE_WRITE);
    int succ;


    f.print("T = ");
    f.print(temperature.temperature);
    f.print("C, ");

    f.print("LowCore = ");
    f.print(LowCore);
    f.print(", ");

    f.print("MidCore = ");
    f.print(MidCore);
    f.print(", ");

    f.print("TopCore = ");
    f.print(TopCore);
    f.print(", ");

    f.print("P = ");
    f.print(pressure);
    f.print("bar, ");

    f.print("Hu = ");
    f.print(humidity.relative_humidity);
    f.print("% rH\n");


    f.close();

    return succ;
  }




void rreadFile(String filename){
  
  File r = SD.open(filename, FILE_READ);
  
  Serial.print("Read from file: ");
  Serial.println(filename);
  while(r.available()){
      Serial.write(r.read());
  }
  Serial.print("\n");
  Serial.println("Done reading file");

  r.close();
  }
  


















void setup() {
  Serial.begin(115200);
  pinMode(37, OUTPUT);
  pinMode(14, OUTPUT);
  pinMode(48, OUTPUT);
  pinMode(10, OUTPUT);


  pinMode(PULSE_CORE_PIN, OUTPUT);
  pinMode(POWER_ON_LC, OUTPUT);
  
  pinMode(low_Core.PIN, INPUT);
  pinMode(mid_Core.PIN, INPUT);
  pinMode(top_Core.PIN, INPUT);
  attachInterrupt(low_Core.PIN, lowTimer, CHANGE);
  attachInterrupt(mid_Core.PIN, midTimer, CHANGE);
  attachInterrupt(top_Core.PIN, topTimer, CHANGE);



  if (bootCount == 0) //Run this only the first time
  {
    //digitalWrite(YELLOW_LED_PIN, HIGH);
    bootCount = bootCount + 1;
  }
  
  else  //loop for all code to go into
  {

  


  
  digitalWrite(37, HIGH);   
  Serial.println("pin 37 set to high");
  
  digitalWrite(14, HIGH);   
  Serial.println("pin 14 set to high");
  
  digitalWrite(48, HIGH);   
  Serial.println("pin 48 set to high");

  
  Wire.begin(I2C_SDA, I2C_SCL);

  if (! aht.begin()) {
    Serial.println("Could not find AHT? Check wiring");
    while (1) delay(5);
  }
  Serial.println("AHT10 or AHT20 found");

  SPIClass fspi(FSPI);
    fspi.begin(12, 13, 11, 10);  // sck, miso, mosi, ss
    


    
    if(!SD.begin(10, fspi)){
        Serial.println("Card Mount Failed");
        return;
    }
    uint8_t cardType = SD.cardType();

    if(cardType == CARD_NONE){
        Serial.println("No SD card attached");
        return;
    }

    Serial.print("SD Card Type: ");
    if(cardType == CARD_MMC){
        Serial.println("MMC");
    } else if(cardType == CARD_SD){
        Serial.println("SDSC");
    } else if(cardType == CARD_SDHC){
        Serial.println("SDHC");
    } else {
        Serial.println("UNKNOWN");
    }

    uint64_t cardSize = SD.cardSize() / (1024 * 1024);
    Serial.printf("SD Card Size: %lluMB\n", cardSize);



  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");





 
    Wire.begin(I2C_SDA, I2C_SCL);


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
    pressure = ((press_counts - outputmin) * (pmax - pmin) / (outputmax - outputmin)) + pmin;



    
    Serial.println(pressure);



    digitalWrite(POWER_ON_LC,HIGH);
    digitalWrite(PULSE_CORE_PIN,HIGH);
    ets_delay_us(100);

    digitalWrite(PULSE_CORE_PIN,LOW);

    
    dummyCoreTime();
    int LowCore = (int)lowCoreTime();
    int MidCore = (int)midCoreTime();
    int TopCore = (int)topCoreTime();







    Serial.println(logData(temp, LowCore, MidCore, TopCore, pressure, humidity));  // logData(temperature, height, pressure, humidity)  all float

    rreadFile("/Data.txt");


    

    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    SD.end();
    Serial.println("End of operation, SD card detached");
  }




  delay(3000);

//  digitalWrite(GREEN_LED_PIN, LOW);
  //digitalWrite(YELLOW_LED_PIN, LOW);
  digitalWrite(PULSE_CORE_PIN, LOW);

  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  esp_deep_sleep_start();

}


void loop() {
  
}
