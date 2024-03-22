
#include "FS.h"
#include "SD.h"
#include "SPI.h"

#include <Adafruit_AHTX0.h>


#define I2C_SDA 20
#define I2C_SCL 21

Adafruit_AHTX0 aht;






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
    f.print("bar, ");

    f.print("Hu = ");
    f.print(humidity);
    f.print("% rh\n");


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


/*

    File f = SD.open("/Data.txt", FILE_APPEND);
    //File f = SD.open("/Data.txt", FILE_WRITE);
    int succ;


    f.print("T = ");
    //f.print(temp.temperature);
    f.print("C, ");

    f.print("H = ");
    f.print(1);
    f.print("m, ");

    f.print("P = ");
    f.print(1);
    f.print("bar, ");

    f.print("Hu = ");
    //f.print(humidity.relative_humidity);
    f.print("% rH,     ");
    succ = f.print("\n");

    f.close();

    Serial.println("File written successfully");

    */

    Serial.println(logData(16.5, 10.38, 1.3, 30.3));  // logData(temperature, height, pressure, humidity)  all float except pressure

    rreadFile("/Data.txt");


    

    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    SD.end();
    Serial.println("End of operation, SD card detached");


  
}

void loop() {
  /*
  sensors_event_t humidity, temp;
  aht.getEvent(&humidity, &temp);// populate temp and humidity objects with fresh data
  Serial.print("Temperature: "); Serial.print(temp.temperature); Serial.println(" degrees C");
  Serial.print("Humidity: "); Serial.print(humidity.relative_humidity); Serial.println("% rH");

  //delay(500);
  */
}
