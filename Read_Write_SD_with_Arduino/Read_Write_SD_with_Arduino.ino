
// #include <SPI.h>
#include <SD.h>

File read1;
File write1;
File read2;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);

  SD.begin(4);
  

  if (SD.exists("TEST.TXT")) {
    Serial.println("TEST.TXT exists");
  } else {
    Serial.println("TEST.TXT doesn't exist");
  }


  read1 = SD.open("TEST.TXT", FILE_READ);

  if (read1){
    while (read1.available()) {
      Serial.write(read1.read());
    }
    read1.close();
    Serial.println("Done reading file");
  }
  else{
    Serial.println("error reading file");
  }

  write1= SD.open("asd.txt", FILE_WRITE);
  write1.write("asdasd\n");
  write1.close();
  Serial.println("Done creating file");



  read2 = SD.open("ASD.TXT", FILE_READ);

  if (read2){
    while (read2.available()) {
      Serial.write(read2.read());
    }
    read2.close();
    Serial.println("Done reading file");
  }
  else{
    Serial.println("error reading file");
  }
}

  

  

  



void loop() {
  // nothing happens after setup finishes.
}
