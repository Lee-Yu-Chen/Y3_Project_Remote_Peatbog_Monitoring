#include <Wire.h>

#define I2C_SDA 20
#define I2C_SCL 21

uint8_t id = 0x28;                   // i2c address
uint8_t data[7];                     // holds output data
uint8_t cmd[3] = {0xAA, 0x00, 0x00}; // command to be sent
double press_counts = 0;             // digital pressure reading [counts]
double temp_counts = 0;              // digital temperature reading [counts]
double pressure = 0;                 // pressure reading [bar, psi, kPa, etc.]
double temperature = 0;              // temperature reading in deg C
double outputmax = 15099494;         // output at maximum pressure [counts]
double outputmin = 1677722;          // output at minimum pressure [counts]
double pmax = 1;                     // maximum value of pressure range [bar, psi, kPa, etc.]
double pmin = 0;                     // minimum value of pressure range [bar, psi, kPa, etc.]
double percentage = 0;               // holds percentage of full scale data
char printBuffer[200], cBuff[20], percBuff[20], pBuff[20], tBuff[20];

void setup()
{
    Serial.begin(9600);
    while (!Serial)
    {
        delay(10);
    }
    Wire.begin(I2C_SDA, I2C_SCL);
    sprintf(printBuffer, "\nStatus Register, 24 - bit Sensor data, Digital Pressure Counts,\
    Percentage of full scale pressure, Pressure Output, Temperature\n");
    Serial.println(printBuffer);
}

void loop()
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


    sprintf(printBuffer, "%s \t %s \t %s \n", percBuff, pBuff, tBuff);
    for(int i=0;i>=7; i++){
      Serial.print(data[i]);
      Serial.print('\t');
      }
    Serial.print('\n');
    Serial.print(printBuffer);
    //Serial.println(pressure);

    delay(200);
}
