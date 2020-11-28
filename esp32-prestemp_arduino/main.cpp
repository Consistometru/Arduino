#include <Arduino.h>

#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP280.h>

#define SEALEVELPRESSURE_HPA (1012)

Adafruit_BMP280 bmp;

float temperature, pressure, altitude;

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
unsigned long previousMillis = 0;        // will store last time LED was updated

// constants won't change:
const long interval = 5000;           // interval at which to blink (milliseconds)  

void print(){
  temperature = bmp.readTemperature();
  pressure = bmp.readPressure() / 100.0F;
  altitude = bmp.readAltitude(SEALEVELPRESSURE_HPA);
  Serial.print("Temperatura = "); Serial.println(temperature);
  Serial.print("Presiune = "); Serial.println(pressure);
  Serial.print("Altitudine ="); Serial.println(altitude);
  Serial.print("Temp ESP=");Serial.println(temperatureRead());
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  bool status;    
    // default settings
    // (you can also pass in a Wire library object like &Wire2)
  status = bmp.begin(0x76);  //I2C address can be 0x77 or 0x76 (by default 0x77 set in library)   
  if (!status) {
   Serial.println("Could not find a valid BMP280 sensor, check wiring!");
   //while (1);
  }
}

void loop() {

  unsigned long currentMillis = millis();
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;
    print();
  }

}
