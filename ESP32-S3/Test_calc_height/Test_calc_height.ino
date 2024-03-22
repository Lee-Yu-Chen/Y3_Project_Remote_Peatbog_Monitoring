
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










int logData(sensors_event_t temperature, double LowCore, double MidCore, double TopCore, float pressure, sensors_event_t humidity){

    
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
  


  
  digitalWrite(37, HIGH);   
  Serial.println("pin 37 set to high");
  
  digitalWrite(14, HIGH);   
  Serial.println("pin 14 set to high");
  
  digitalWrite(48, HIGH);   
  Serial.println("pin 48 set to high");

  

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






    File r = SD.open("/Data.txt", FILE_READ);
  
  Serial.print("Read from file: ");


  int low, mid, top;
  char s[15];

  
  while(r.available()){
    
    s[14] = s[13];
    s[13] = s[12];
    s[12] = s[11];
    s[11] = s[10];
    s[10] = s[9];
    s[9] = s[8];
    s[8] = s[7];
    s[7] = s[6];
    s[6] = s[5];
    s[5] = s[4];
    s[4] = s[3];
    s[3] = s[2];
    s[2] = s[1];
    s[1] = s[0];
    s[0] = r.read();

    if (s[0] == 'w' && s[1] == 'o' && s[2] == 'L'){
      if(s[10] == '-'){
        low = -1;
        }
      else{
        low = (int)s[10]*100 + (int)s[11]*10 + (int)s[12];
        }
    }
    else if (s[0] == 'd' && s[1] == 'i' && s[2] == 'M'){
      if(s[10] == '-'){
        mid = -1;
        }
      else{
        mid = (int)s[10]*100 + (int)s[11]*10 + (int)s[12];
        }
      
      }
    else if (s[0] == 'p' && s[1] == 'o' && s[2] == 'T'){
      if(s[10] == '-'){
        top = -1;
        }
      else{
        top = (int)s[10]*100 + (int)s[11]*10 + (int)s[12];
        }
      }
      
      


    
      //Serial.print((char)r.read());
  }
  
  Serial.print("\n");
  Serial.println("Done reading file");
  Serial.print(low);
  Serial.print(mid);
  Serial.println(top);

  r.close();



    

    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    SD.end();
    Serial.println("End of operation, SD card detached");
  


}


void loop() {
  
}
