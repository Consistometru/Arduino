#include <Arduino.h>

#define PIN_IESIRE  13
#define LED_ONBOARD  2
#define TOUCH_PIN 4
uint8_t deschis = 1;
uint8_t inchis = 0;
const int threshold = 40;

int touchValue;
bool lasttouchValue = false;

RTC_DATA_ATTR int bootCount = 0;
/*#define uS_TO_S_FACTOR 1000000  /* Conversion factor for micro seconds to seconds */
/*#define TIME_TO_SLEEP  15        /* Time ESP32 will go to sleep (in seconds) */

void pornire(){
    Serial.print(touchValue);
    digitalWrite(PIN_IESIRE, deschis);
    digitalWrite(LED_ONBOARD, inchis);
    Serial.println("=semnal || LED on");
    
}

void oprire(){
    Serial.print(touchValue);
    digitalWrite(PIN_IESIRE, inchis);
    digitalWrite(LED_ONBOARD, deschis);
    Serial.println("=semnal || LED off");
}

void callback3(){
  //placeholder callback function only for touch wake
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  delay(1000);
  pinMode(PIN_IESIRE,OUTPUT);
  //pinMode(LED_ONBOARD,OUTPUT); - avoid wake-up by touchpin 2
  Serial.println("Temp ESP="+ String(temperatureRead()) +"C"); 

  ++bootCount;
  Serial.println("Boot number: " + String(bootCount));  

 /* esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
  Serial.println("Setup ESP32 to sleep for every " + String(TIME_TO_SLEEP) +  " Seconds"); */  
  
  touchAttachInterrupt(T3, callback3, threshold);  //only for touch wake
  //Configure Touchpad as wakeup source
  esp_sleep_enable_touchpad_wakeup();
  
}

void loop() {
  // put your main code here, to run repeatedly:
  touchValue = touchRead(TOUCH_PIN);  
  if (touchValue < threshold){
    lasttouchValue = !lasttouchValue;
    }
  if (lasttouchValue){
    pornire();
    //Go to sleep now
    Serial.println("Going to sleep now");
    Serial.flush();
    delay(500);
    esp_deep_sleep_start();
  } else {
    oprire();
  }
  delay(500);  
}