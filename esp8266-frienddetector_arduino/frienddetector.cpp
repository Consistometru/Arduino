#include <Arduino.h>

#include <EasyButton.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "./esppl_functions.h"
#define MY_LOG 1

/*<===========================modifica mai jos=============================>*/

                    const char *ssid = "test";
                    const char *password = "11122233";

/*<===========================modifica mai sus============================>*/

/*<==========================modificare pin, putere, timp semnal pentru actiune =======================>*/
                      uint8_t pin = { D0 };
                      int putere = -60;  // putere deschidere
                      int timp = 8000;  //timp autentificare
                      int t = 3;  //secunde deschidere releu
                      uint8_t deschis = 0;
                      uint8_t inchis = 1;
/*<==========================mmodificare pin, putere, timp semnal pentru actiune =======================>*/

String content;
File logs;
int dimensiune;
uint8_t friendmac[1800][6];  // limita 'soft' adrese - neoptimizata
ESP8266WebServer server(80);
EasyButton button(0);



void actiune() {  //----------------------------------------->actiune
digitalWrite(pin, deschis); 
Serial.println("led on ##########"); 
button.read();
}  


void adresa(){  //------------------------------------------->adrese 
     logs = SPIFFS.open("/logs.txt", "r");
     if (!logs) Serial.println("file open failed");           
     Serial.print ("logs size: "); Serial.println(logs.size());  

     int j=0;
     while (logs.available()) {
        int addr_buf0=0,addr_buf1=0,addr_buf2=0,addr_buf3=0,addr_buf4=0,addr_buf5=0 ;
        String line; // - optimizare citire adrese
        line = logs.readStringUntil('\n');
        sscanf(line.c_str(),"%d %d %d %d %d %d", &addr_buf0,&addr_buf1,&addr_buf2,&addr_buf3,&addr_buf4,&addr_buf5) ;  
        friendmac[j][0] = addr_buf0;  friendmac[j][1] = addr_buf1;  friendmac[j][2] = addr_buf2;
         friendmac[j][3] = addr_buf3;  friendmac[j][4] = addr_buf4;  friendmac[j][5] = addr_buf5;
        j++; dimensiune++;
        }  
     Serial.print("nr adrese= ");Serial.println(dimensiune); 
     logs.close();        
}  



bool maccmp(uint8_t *mac1, uint8_t *mac2) { //------------------------------>comparare
  return !memcmp(mac1,mac2,ESPPL_MAC_LEN);
}

unsigned long previousMillis = 0;        // will store last time LED was updated
const long interval = t*1000;           // interval at which to blink (milliseconds)
unsigned long currentMillis = 0;



void cb(esppl_frame_info *info) {  // ----------------------------------------------------------------->magie
             
    for (int i=0; i<dimensiune ; i++) {
      if (maccmp(info->sourceaddr, friendmac[i]) || maccmp(info->receiveraddr, friendmac[i])) {  
      if (info->rssi > putere) {    //<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<ce putere sa fie depistata <<<<<<<<<<
      Serial.print("open the goddamn door! ");      
      Serial.print(" RSSI: ");
      Serial.println(info->rssi); 
      String s;
      for (int j=0;j<6;j++){
        s+=friendmac[i][j];
      }
      Serial.print(s); Serial.print(" de pe pozitia: "); Serial.println(i+1);
     
      actiune();
      // save the last time you blinked the LED
      previousMillis = currentMillis;
      
      } } else {
      currentMillis = millis();
      if (currentMillis - previousMillis >= interval) {      
      digitalWrite(pin, inchis); 
         }                                 
      } 
   } 
}

void handleRoot()   //--------------------------------->pagina web

   { server.on("/", []() {
      content = "<!DOCTYPE HTML>\r\n<html><h1 style=text-align:center>Apasa pe buton pentru autentificare </h1>";  
      content += "<span style=text-align:center><form method='get' action='setting'><input type='submit'></form>";
      content += "</html>";   
  server.send(200, "text/html", content);
   
  }); 
   server.on("/setting", []() {
   logs = SPIFFS.open("/logs.txt", "a+"); //------deschide fisierul pentru scriere
   if (!logs) Serial.println("file open failed");
    
     struct station_info *stat_info;
     stat_info = wifi_softap_get_station_info(); 
    
     if (wifi_softap_get_station_num() == 1 && wifi_softap_get_station_num()>0){
       
     logs.print(stat_info->bssid[0]); logs.print(" ");
     logs.print(stat_info->bssid[1]); logs.print(" ");
     logs.print(stat_info->bssid[2]); logs.print(" ");
     logs.print(stat_info->bssid[3]); logs.print(" ");
     logs.print(stat_info->bssid[4]); logs.print(" ");
     logs.println(stat_info->bssid[5]); 
     logs.close(); Serial.println("succes")  ;

         content = "<!DOCTYPE HTML>\r\n<html><h1 style=text-align:center>Autentificare reusita</h1> ";
         server.send(200, "text/html", content);    
                                     
         } else{
      delay(100);
     WiFi.softAPdisconnect(true);
     Serial.println("HTTP server stopped");
     logs.close(); 
     ESP.restart();
     } 
      delay(100);
     WiFi.softAPdisconnect(true);
     Serial.println("HTTP server stopped");
     logs.close(); 
     ESP.restart(); 
     });
   }   



void onPress()        //-------------------------------->apasare buton
{
   
    Serial.println("Button has been pressed!");
    esppl_sniffing_stop();
    wifi_station_disconnect();
    wifi_promiscuous_enable(false);
    wifi_set_opmode(SOFTAP_MODE);    
    delay(100);
    WiFi.softAP(ssid, password); 
    delay(100);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("HTTP server started");  
    server.begin();   
    handleRoot();    
    for (int i= 0;  i<timp; i++) {   
      server.handleClient();                      
      delay(10);
      }
    WiFi.softAPdisconnect(true);
    Serial.println("HTTP server stopped"); 
    logs.close(); 
    ESP.restart();
        
}



void setup() { //----------------------------------->setup
  delay(500);
  pinMode(pin, OUTPUT); // ---------sets the pin to output mode
  if (MY_LOG) Serial.begin(115200);
  button.read();
  esppl_init(cb);
  Serial.println("so it begins");
  esppl_sniffing_start();
  digitalWrite(pin, inchis);  // ------------pin off
  button.begin();
  button.onPressed(onPress);
  SPIFFS.begin();   
  adresa();                    
  
  while ( esppl_sniffing_enabled  ) {
    button.read();
    for (int i = ESPPL_CHANNEL_MIN; i <= ESPPL_CHANNEL_MAX; i++ ) {
      esppl_set_channel(i);
      button.read();
      while (esppl_process_frames()) {
      button.read();
      }
    }
  }
  
}


void loop() {   //-------------------------------->loop 
  button.read();      
}