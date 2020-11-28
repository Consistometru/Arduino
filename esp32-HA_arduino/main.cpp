#include <Arduino.h>

#include "EasyButtonBase.h"
#include <SPIFFS.h> //spiff file system   
#include <FS.h>
#include <WiFi.h> 
#include <WebServer.h>      
#include <WiFiClient.h>             

const char* ssid     = "test";         // wifi credentials
const char* password = "123adr45";

const int    buttonPin = 23;
const int    ledPin  =   2;

#include <BlynkApiArduino.h>
#include <Blynk/BlynkProtocol.h>
#include <Adapters/BlynkArduinoClient.h>

#define EASYBUTTON_READ_TYPE_INTERRUPT 0
#define EASYBUTTON_READ_TYPE_POLL 1
static WiFiClient _blynkWifiClient;
static BlynkArduinoClient _blynkTransport(_blynkWifiClient);
BlynkWifi Blynk(_blynkTransport);

File logs;
EasyButton button(buttonPin);  
//Establishing Local server at port 80 whenever required
WebServer server(80);

String      htmlPage;       // webpage text to be sent out by server when main page is accessed
String      st;

int ic, ic1;


//=======================================//


// create the html page for the root path of the web server
void buildHtmlPage() {
  htmlPage = "<!DOCTYPE html>";
  htmlPage += "<html>";
  htmlPage += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";                                         // header section
  htmlPage += "<title style=text-align:center >ESP32 Web Server</title>\n";             // title for browser window
  htmlPage += "</head>";
  
  htmlPage += "<BODY bgcolor='#E0E0D0'>";                      // body section, set background color

  IPAddress ip = WiFi.softAPIP();
      String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);
      htmlPage += "<form action=\"/scan\" method=\"POST\"><input type=\"submit\" value=\"scan\"></form>";
      htmlPage += ipStr;
      htmlPage += "<p>";
      htmlPage += st;
      htmlPage += "</p><form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input name='auth' length=64><input type='submit'></form>";


  // show led status and action buttons
  String ledState = ((digitalRead(ledPin)) ? "off" : "on");
  htmlPage += "<h2 style=text-align:center>LED: </h2>" ;  
  htmlPage += "<span style=text-align:center><form action=\"/LEDTOGGLE\" method=\"POST\"><input type=\"submit\" value=\"Toggle LED\"></form>";

  htmlPage += "</body>";
  htmlPage += "</html>";
}
// send main web page when ip+"/" is accessed
void handleRoot() {
  buildHtmlPage();
  server.send(200, "text/html", htmlPage); 
}
// toggle led and redirect to main page
void handleLEDToggle() {                          
  digitalWrite(ledPin, !digitalRead(ledPin));     
  server.sendHeader("Location", "/");       
  server.send(303);  
  Serial.println("Webpage button pressed");                       
}
// scan networks and redirect to main page
void handleScan() {                              
  server.sendHeader("Location", "/"); 
  IPAddress ip = WiFi.softAPIP();
  String ipStr = String(ip[0]) + '.' + String(ip[1]) + '.' + String(ip[2]) + '.' + String(ip[3]);    
  server.send(303);  
  Serial.println("scan pressed");                       
}
// save credentials and restart or just restart
void handleSetting() {                              
   String qssid = server.arg("ssid");
   String qpass = server.arg("pass");
   String qauth = server.arg("auth");
      if (qssid.length() > 0 && qpass.length() > 0 && qauth.length() > 0) {
        if (SPIFFS.exists("/logs.txt")){ SPIFFS.remove("/logs.txt");
         Serial.println("SPIFFS removed");} else{      
          Serial.println("n-ai niciun SPIFF");
        }        
        logs = SPIFFS.open("/logs.txt", "a+"); //------deschide fisierul pentru scriere
         if (!logs) Serial.println("file open failed");
        
        Serial.println(qssid);
        Serial.println("");
        Serial.println(qpass);
        Serial.println("");
        Serial.println(qauth);
        Serial.println("");

        Serial.println("writing ssid");
        logs.println(qssid);
         delay(10);
        Serial.println("writing pass");
        logs.println(qpass);
         delay(10);
        Serial.println("writing auth");
        logs.println(qauth);
        delay(10);   
        logs.close(); Serial.println("succes")  ;       
        delay(10);
        ESP.restart(); 
        } else {
    server.send(404, "text/plain", "404: Reset"); 
    delay(10);
    ESP.restart();
   }
  Serial.println("submit pressed");                       
}
// send HTTP status 404: Not Found when there's no handler for the client request
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found"); 
}

void setupAP(){
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  int n = WiFi.scanNetworks();
  Serial.println("scan done");
  if (n == 0)
    Serial.println("no networks found");
  st = "<ol>";
  for (int i = 0; i < n; i++)
  {
    // Store SSID and RSSI for each network found
    st += "<li>";
    st += WiFi.SSID(i);
    st += " (";
    st += WiFi.RSSI(i);
    st += ")";
    //st += (WiFi.encryptionType(i) == ENC_TYPE_NONE) ? " " : "*";
    st += "</li>";
  }
  st += "</ol>";
  delay(100);
  WiFi.mode(WIFI_AP);   
    delay(100);
    WiFi.softAP(ssid, password); 
    delay(100);
    Serial.print("AP IP address: ");
    Serial.println(WiFi.softAPIP());
    Serial.println("HTTP softAP started");   
    button.read();  

  // functions to call when client requests are received
  server.on("/", handleRoot);     
  server.on("/LEDTOGGLE", handleLEDToggle);
  server.on("/scan", handleScan); 
  server.on("/setting", handleSetting);    
  server.onNotFound(handleNotFound);   
  server.begin();                           // start web server
  Serial.println("HTTP server started");
}

//======================================//

void connecting(){
  Serial.println("1 conectare"); 
  Serial.println( ic);
  button.read();
  ic++;
  if (ic>250 ) {      
    Serial.println("Turning the HotSpot On");
    setupAP();
    for (int i=0; i<15000; i++){
    button.read();
    server.handleClient();    // listen for HTTP requests from clients
    button.read();
    delay(100);}
    delay(10);
    ESP.restart();
  }
  
}

class BlynkWifi
    : public BlynkProtocol<BlynkArduinoClient>
{
    typedef BlynkProtocol<BlynkArduinoClient> Base;
public:
    BlynkWifi(BlynkArduinoClient& transp)
        : Base(transp)
    {}

    
    void connectWiFi(const char* ssid, const char* pass)
    {
        WiFi.mode(WIFI_STA);
        if (WiFi.status() != WL_CONNECTED) {
            if (pass && strlen(pass)) {
                WiFi.begin(ssid, pass);
            } else {
                WiFi.begin(ssid);
            }
        }
        while (WiFi.status() != WL_CONNECTED) {
            connecting();
            button.read();
            delay(100);
        }
        Serial.println("Connected to WiFi blynk");

        IPAddress myip = WiFi.localIP();      
    }

    void config(const char* auth,
                const char* domain = BLYNK_DEFAULT_DOMAIN,
                uint16_t    port   = BLYNK_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(domain, port);
    }

    void config(const char* auth,
                IPAddress   ip,
                uint16_t    port = BLYNK_DEFAULT_PORT)
    {
        Base::begin(auth);
        this->conn.begin(ip, port);
    }

    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               const char* domain = BLYNK_DEFAULT_DOMAIN,
               uint16_t    port   = BLYNK_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass);
        config(auth, domain, port);
        while(this->connect() != true) {}
    }

    void begin(const char* auth,
               const char* ssid,
               const char* pass,
               IPAddress   ip,
               uint16_t    port   = BLYNK_DEFAULT_PORT)
    {
        connectWiFi(ssid, pass);
        config(auth, ip, port);
        while(this->connect() != true) {}
    }

};

class EasyButton : public EasyButtonBase
{
	friend class EasyButtonTouch;

public:
	EasyButton(uint8_t pin, uint32_t debounce_time = 80, bool pullup_enable = true, bool active_low = true) : EasyButtonBase(active_low), _pin(pin), _db_time(debounce_time), _pu_enabled(pullup_enable), _read_type(EASYBUTTON_READ_TYPE_POLL)
	{
	}
	~EasyButton() {}

	// PUBLIC FUNCTIONS
	virtual void begin();					   // Initialize a button object and the pin it's connected to.
	bool read();							   // Returns the current debounced button state, true for pressed, false for released.
	void update();							   // Update button pressed time, only needed when using interrupts.
	void enableInterrupt(callback_t callback); // Call a callback function when the button is pressed or released.
	void disableInterrupt();
	bool supportsInterrupt(); // Returns true if the button pin is an external interrupt pin.

private:
	// PRIVATE VARIABLES
	uint8_t _pin;		// Arduino pin number where the Button is connected to.
	uint32_t _db_time;	// Debounce time (ms).
	bool _pu_enabled;	// Internal pullup resistor enabled.
	uint8_t _read_type; // Read type. Poll or Interrupt.

	virtual bool _readPin(); // Abstracts the pin value reading.
};

void onPress() {
  Serial.println("Physical button has been pressed!");
  digitalWrite(ledPin, !digitalRead(ledPin));
  Blynk.virtualWrite(1 , !digitalRead(ledPin));
}

BLYNK_WRITE(V1) {
  digitalWrite(ledPin, !digitalRead(ledPin));
  Serial.println("Blynk button has been pressed!");
  Blynk.virtualWrite(1 , !digitalRead(ledPin));
}
 
void setup() {
  Serial.begin(115200);
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);            // led is off initially  
  pinMode(buttonPin, INPUT_PULLUP);
  Serial.println("Incepem");  
  Serial.println("Disconnecting previously connected WiFi");
  WiFi.disconnect();  
  if(!SPIFFS.begin(true)){
      Serial.println("An Error has occurred while mounting SPIFFS");
      return;
  }
  delay(10);  
  String b_essid, b_epass, b_eauth;  
  logs = SPIFFS.open("/logs.txt", "r");
     if (!logs) {
        Serial.println("file open failed");
        Serial.println("Turning the HotSpot On");
        setupAP();
        for (int i=0; i<15000; i++){
        server.handleClient();    // listen for HTTP requests from clients
        button.read();
        delay(100);}
        delay(10);
        ESP.restart();        
      }                
    Serial.print ("logs size: "); Serial.println(logs.size()); 
    while (logs.available()) {
    b_essid = logs.readStringUntil('\n');
    delay(10);
    b_epass = logs.readStringUntil('\n');
    delay(10);
    b_eauth = logs.readStringUntil('\n');
    delay(10);  
    b_eauth.trim(); b_essid.trim(); b_epass.trim();  
    Serial.println(b_eauth); Serial.println(b_essid); Serial.println(b_epass);
   } 
   /*b_eauth = "OAXKDvuXi3ZRu_I2KVR1XoHi0F0OFZLz";
   b_essid = "pendula";
   b_epass = "lavandula";*/
   delay(10);
  logs.close(); 
  //MANUAL CONFIG=============
  button.onPressed(onPress);
  delay(1000);
  // Connect 
  Blynk.begin(b_eauth.c_str(), b_essid.c_str(), b_epass.c_str());
         
}

void loop() {   

  if (!Blynk.connected()) {
    delay(100);
    button.read();
    Serial.println("Connecting to WiFi..loop");
    ic1++;
     if (ic1>250 ) {      
     Serial.println("Turning the HotSpot On");
     digitalWrite(ledPin, HIGH);  // turn off led if no internet
     setupAP();
     for (int i=0; i<15000; i++){
     server.handleClient();    // listen for HTTP requests from clients
     button.read();
     delay(100);}
     delay(10);
     ESP.restart();
    }
  }

 Blynk.run(); 
 button.read();
}
