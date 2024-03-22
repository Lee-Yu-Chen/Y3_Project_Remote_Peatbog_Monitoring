/*
 * Connect the SD card to the following pins:
 *
 * SD Card | ESP32
 *    D2       -
 *    D3       SS
 *    CMD      MOSI
 *    VSS      GND
 *    VDD      3.3V
 *    CLK      SCK
 *    VSS      GND
 *    D0       MISO
 *    D1       -
 */
#include "FS.h"
#include "SD.h"
#include "SPI.h"
#include <WiFi.h>
#include <time.h>


//SPIClass * hspi = new SPIClass(HSPI);



void listDir(fs::FS &fs, const char * dirname, uint8_t levels){
    Serial.printf("Listing directory: %s\n", dirname);

    File root = fs.open(dirname);
    if(!root){
        Serial.println("Failed to open directory");
        return;
    }
    if(!root.isDirectory()){
        Serial.println("Not a directory");
        return;
    }

    File file = root.openNextFile();
    while(file){
        if(file.isDirectory()){
            Serial.print("  DIR : ");
            Serial.println(file.name());
            if(levels){
                listDir(fs, file.name(), levels -1);
            }
        } else {
            Serial.print("  FILE: ");
            Serial.print(file.name());
            Serial.print("  SIZE: ");
            Serial.println(file.size());
        }
        file = root.openNextFile();
    }
}







void createDir(fs::FS &fs, const char * path){
    Serial.printf("Creating Dir: %s\n", path);
    if(fs.mkdir(path)){
        Serial.println("Dir created");
    } else {
        Serial.println("mkdir failed");
    }
}







void removeDir(fs::FS &fs, const char * path){
    Serial.printf("Removing Dir: %s\n", path);
    if(fs.rmdir(path)){
        Serial.println("Dir removed");
    } else {
        Serial.println("rmdir failed");
    }
}






void readFile(fs::FS &fs, const char * path){
    Serial.printf("Reading file: %s\n", path);

    File file = fs.open(path);
    if(!file){
        Serial.println("Failed to open file for reading");
        return;
    }

    Serial.print("Read from file: ");
    while(file.available()){
        Serial.write(file.read());
    }
    file.close();
}







void writeFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Writing file: %s\n", path);

    File file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }
    if(file.print(message)){
        Serial.println("File written");
    } else {
        Serial.println("Write failed");
    }
    file.close();
}






void appendFile(fs::FS &fs, const char * path, const char * message){
    Serial.printf("Appending to file: %s\n", path);

    File file = fs.open(path, FILE_APPEND);
    if(!file){
        Serial.println("Failed to open file for appending");
        return;
    }
    if(file.print(message)){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }
    file.close();
}







void renameFile(fs::FS &fs, const char * path1, const char * path2){
    Serial.printf("Renaming file %s to %s\n", path1, path2);
    if (fs.rename(path1, path2)) {
        Serial.println("File renamed");
    } else {
        Serial.println("Rename failed");
    }
}






void deleteFile(fs::FS &fs, const char * path){
    Serial.printf("Deleting file: %s\n", path);
    if(fs.remove(path)){
        Serial.println("File deleted");
    } else {
        Serial.println("Delete failed");
    }
}







void testFileIO(fs::FS &fs, const char * path){
    File file = fs.open(path);
    static uint8_t buf[512];
    size_t len = 0;
    uint32_t start = millis();
    uint32_t end = start;
    if(file){
        len = file.size();
        size_t flen = len;
        start = millis();
        while(len){
            size_t toRead = len;
            if(toRead > 512){
                toRead = 512;
            }
            file.read(buf, toRead);
            len -= toRead;
        }
        end = millis() - start;
        Serial.printf("%u bytes read for %u ms\n", flen, end);
        file.close();
    } else {
        Serial.println("Failed to open file for reading");
    }


    file = fs.open(path, FILE_WRITE);
    if(!file){
        Serial.println("Failed to open file for writing");
        return;
    }

    size_t i;
    start = millis();
    for(i=0; i<2048; i++){
        file.write(buf, 512);
    }
    end = millis() - start;
    Serial.printf("%u bytes written for %u ms\n", 2048 * 512, end);
    file.close();
}

















int logTemperature(int temperature){

    time_t t;
    time(&t);
    File f = SD.open("/Temperature.txt", FILE_APPEND);
    int succ;
    
    f.print(ctime(&t));
    f.print(" : T = ");
    f.print(temperature);
    succ = f.print("\n");

    f.close();

    return succ;
  }



int readTemp(){
  int temp;

  File r = SD.open("/Temperature.txt", FILE_READ);

  temp = r.read();

  r.close();
  
  return temp;
  }







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

  










void setup(){
    Serial.begin(115200);

    initWiFi();
    configTime(0, 0, ntpServer); //?

    //hspi = new SPIClass(HSPI);
    SPIClass hspi(HSPI);

    hspi.begin(14, 12, 13, 15);  // sck, miso, mosi, ss

    //SPIClass &ref = hspi;

    pinMode(15, OUTPUT);


    
    if(!SD.begin(15, hspi)){
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



    /*
    int hasMkdir = SD.mkdir("/Data");

    Serial.println(hasMkdir);

    listDir(SD, "/", 0);


    delay(3000);
    

    hasMkdir = SD.mkdir("/Data/Pressure");

    Serial.println(hasMkdir);

    listDir(SD, "/", 1);

    delay(3000);

    Serial.println(SD.mkdir("/Data/Temperature"));
    Serial.println(SD.mkdir("/Data/Height"));
    Serial.println(SD.mkdir("/Data/Humidity"));
    Serial.println(SD.mkdir("/Data/Temperature/today"));
    Serial.println(SD.mkdir("/Data/Temperature/tmr"));
    Serial.println(SD.mkdir("/Data/Temperature/today/1200"));


    listDir(SD, "/", 0);
    listDir(SD, "/", 1);
    listDir(SD, "/", 2);
    listDir(SD, "/", 3);
    */

    //time_t t;
    //time(&t);

    File w = SD.open("/Temperature.txt", FILE_WRITE);
    w.close();
    

    

    

    //Serial.println(logTemperature(13));

    //Serial.write(readTemp());

    //Serial.println(readTemp());

    
    




    
    
    /*
    listDir(SD, "/", 0);
    
    createDir(SD, "/mydir");
    listDir(SD, "/", 0);
    removeDir(SD, "/mydir");
    listDir(SD, "/", 2);
    writeFile(SD, "/hello.txt", "Hello ");
    appendFile(SD, "/hello.txt", "World!\n");
    

    
    readFile(SD, "/hello.txt");
    // deleteFile(SD, "/foo.txt");
    // renameFile(SD, "/hello.txt", "/foo.txt");
    readFile(SD, "/foo.txt");
    testFileIO(SD, "/test.txt");
    */
    Serial.printf("Total space: %lluMB\n", SD.totalBytes() / (1024 * 1024));
    Serial.printf("Used space: %lluMB\n", SD.usedBytes() / (1024 * 1024));
    //SD.end();
    //Serial.println("End of operation, SD card detached");
}





void loop(){
  epochTime = getTime();
  File w = SD.open("/Temperature.txt", FILE_APPEND);

  if(!w){
    Serial.println("Failed to open file for appending");
    return;
    }
  else{
    Serial.println("File opened successful");
    }
    if(w.print("Temperature = ")){
        Serial.println("Message appended");
    } else {
        Serial.println("Append failed");
    }

  Serial.println("1");

    
  w.print("13, ");

  Serial.println("2");
  
  w.print(ctime(&epochTime));

  Serial.println("3");
  
  w.print("\n");

  Serial.println("4");
  
  //w.close();

  //Serial.println("Data added, file closed");

  delay(1000);

    

}
