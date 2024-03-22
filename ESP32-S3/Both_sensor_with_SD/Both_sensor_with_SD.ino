
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <Wire.h>

#include <Adafruit_AHTX0.h>


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






int logData(sensors_event_t temperature, float height, float pressure, sensors_event_t humidity){

    
    File f = SD.open("/Data.txt", FILE_APPEND);
    //File f = SD.open("/Data.txt", FILE_WRITE);
    int succ;


    f.print("T = ");
    f.print(temperature.temperature);
    f.print("C, ");

    f.print("H = ");
    f.print(height);
    f.print("m, ");

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
  pinMode(10, OUTPUT);


  
  digitalWrite(37, HIGH);   // turn on sensors
  Serial.println("pin 37 set to high");
  digitalWrite(14, HIGH);   // turn on SD card
  Serial.println("pin 14 set to high");

  
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







    Serial.println(logData(temp, 10.38, pressure, humidity));  // logData(temperature, height, pressure, humidity)  all float

    rreadFile("/Data.txt");


    

    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    SD.end();
    Serial.println("End of operation, SD card detached");


  
}

void loop() {
  
}
