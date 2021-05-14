#include <Arduino.h>


uint32_t passkey = 123456 ; // PASS
#define NUME "Interfon – test" //NUME 

int timp_deschis_b = 5000 ; //milisecunde
int tic = 200 ; //milisecunde

uint8_t deschis = 1;
uint8_t inchis = 0;
#define PIN_IESIRE  13
#define LED_ONBOARD  2

#define my_log 0
#define time_out 1

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEHIDDevice.h>


BLEServer *pServer ;
BLEService *pService ;
bool deviceConnected = false;
bool conf = false;
int device_buff = 0 ; 
int timp_deschis = timp_deschis_b;

BLEHIDDevice* hid;

void setupCharacteristics() {

  BLECharacteristic* input = hid->inputReport(1); // <-- input REPORTID from report map
  input = pService->createCharacteristic(
            (uint16_t) 0x2a4d,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY);
  std::string name = "Cosmin";   // <-- OPTIONAL
  hid->manufacturer()->setValue(name);  // <-- OPTIONAL

  hid->pnp(0x01, 0xe502, 0xa111, 0x0210);  // <-- example pnp values
  hid->hidInfo(0x00, 0x01);
  hid->setBatteryLevel(90);


  const uint8_t report[] = {

    0x05, 0x01,        // Usage Page (Generic Desktop Ctrls)
    0x09, 0x02,        // Usage (Mouse)
    0xA1, 0x01,        // Collection (Application)
    0x85, 0x01,        //   Report ID (1)
    0x09, 0x01,        //   Usage (Pointer)
    0xA1, 0x00,        //   Collection (Physical)
    0x05, 0x09,        //     Usage Page (Button)
    0x19, 0x01,        //     Usage Minimum (0x01)
    0x29, 0x03,        //     Usage Maximum (0x03)
    0x15, 0x00,        //     Logical Minimum (0)
    0x25, 0x01,        //     Logical Maximum (1)
    0x95, 0x03,        //     Report Count (3)
    0x75, 0x01,        //     Report Size (1)
    0x81, 0x02,        //     Input (Data,Var) 3bits button
    0x95, 0x01,        //     Report Count (1)
    0x75, 0x05,        //     Report Size (5)
    0x81, 0x01,        //     Input (Const,Array) 5 bits padding
    0x05, 0x01,        //     Usage Page (Generic Desktop Ctrls)
    0x09, 0x30,        //     Usage (X)
    0x09, 0x31,        //     Usage (Y)
    0x09, 0x38,        //     Usage (WHEEL)
    0x16, 0x81, 0xff,  //     Logical Minimum (-127)
    0x26, 0x7f, 0x00,  //     Logical Maximum (127)
    0x75, 0x08,        //     Report Size (8)
    0x95, 0x03,        //     Report Count (3)
    0x81, 0x06,        //     Input (Data,Var,Rel) 3 bytes (x,y,wheel)
    0xC0,              //   End Collection
    0xC0,              // End Collection
  };

  hid->reportMap((uint8_t*)report, sizeof(report));

  BLECharacteristic* output = hid->outputReport(1);
  output = pService->createCharacteristic(
             (uint16_t) 0x2a4d,
             BLECharacteristic::PROPERTY_READ |
             BLECharacteristic::PROPERTY_WRITE |
             BLECharacteristic::PROPERTY_NOTIFY);


  hid->startServices();
}


class MyServerCallbacks: public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
      deviceConnected = true;
      if (time_out) timp_deschis = timp_deschis_b;
      BLEDevice::startAdvertising();
  };

  void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
      if (time_out) device_buff -- ;
      if (pServer->getConnectedCount() == 0)      
      conf = false;

      
  }
};


class MySecurity : public BLESecurityCallbacks {

  bool onConfirmPIN(uint32_t pin) {
      conf = true;
      if (time_out) {timp_deschis = timp_deschis_b;
      device_buff -- ; }
      return false;
  }

  uint32_t onPassKeyRequest() {
      Serial.println ( "PassKeyRequest");
      return 0;
  }

  void onPassKeyNotify(uint32_t pass_key) {
      Serial.println ( "On passkey Notify number");
  }

  bool onSecurityRequest() {
      Serial.println ( "On Security Request");
      return true;
  }

 void onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl) {
   if (cmpl.success) {          
      conf = true;      
    }
  }
};


void setup() {
  if (my_log) Serial.begin(115200);
  Serial.println("Starting BLE work!");

  pinMode(PIN_IESIRE, OUTPUT);
  pinMode(LED_ONBOARD, OUTPUT);

  BLEDevice::init(NUME);
  esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N9);
  /*ESP_PWR_LVL_N12 = 0,                < Corresponding to -12dbm */
  /*ESP_PWR_LVL_N9  = 1,                < Corresponding to  -9dbm */
  /*ESP_PWR_LVL_N6  = 2,                < Corresponding to  -6dbm */
  /*ESP_PWR_LVL_N3  = 3,                < Corresponding to  -3dbm */
  /*ESP_PWR_LVL_N0  = 4,                < Corresponding to   0dbm */
  /*ESP_PWR_LVL_P3  = 5,                < Corresponding to  +3dbm */
  /*ESP_PWR_LVL_P6  = 6,                < Corresponding to  +6dbm */
  /*ESP_PWR_LVL_P9  = 7,                < Corresponding to  +9dbm */

  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT);
  pServer = BLEDevice::createServer();
  pService = pServer->createService((uint16_t)0x1815);
  pService->start();
  //
  BLESecurity *pSecurity = new BLESecurity();
  //esp_ble_auth_req_t auth_req = ESP_LE_AUTH_BOND;
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  pSecurity->setCapability(ESP_IO_CAP_OUT);
  pSecurity->setKeySize(16);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  uint8_t rsp_key = ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK;
  uint8_t auth_option = ESP_BLE_ONLY_ACCEPT_SPECIFIED_AUTH_DISABLE;
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_STATIC_PASSKEY, &passkey, sizeof(uint32_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_ONLY_ACCEPT_SPECIFIED_SEC_AUTH, &auth_option, sizeof(uint8_t));
  esp_ble_gap_set_security_param(ESP_BLE_SM_SET_RSP_KEY, &rsp_key, sizeof(uint8_t));

  pServer->setCallbacks(new MyServerCallbacks());

  BLEDevice::setSecurityCallbacks(new MySecurity());


  BLEAdvertising *pAdvertising = pServer->getAdvertising();  // this still is working for backward compatibility
  // BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID((uint16_t)0x1815);
  pAdvertising->setAppearance(963);
  pAdvertising->setScanResponse(true);
  pAdvertising->setScanFilter(false, false);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  //pAdvertising->setMinPreferred(0x12);

  hid = new BLEHIDDevice(pServer);
  setupCharacteristics();
  delay(10);
  BLEDevice::startAdvertising();  
  delay(10);  
  Serial.println("Characteristic defined! Now you can read it in your phone!");

}


void loop() {

  if (pServer->getConnectedCount() > 0 && conf && pServer->getConnectedCount()>device_buff && timp_deschis > 0) {
    Serial.print("led ON " ); Serial.println( conf);
    digitalWrite(PIN_IESIRE, inchis);
    digitalWrite(LED_ONBOARD, inchis);
    Serial.print("conn ");Serial.println(pServer->getConnectedCount());
    if (time_out) {
      timp_deschis = timp_deschis - tic ;
      if (timp_deschis < 1000) {
        device_buff = pServer->getConnectedCount();   
        conf = false;
      }     
    }    
  }
  else{
    
    Serial.print("led off "); Serial.println( conf);
    digitalWrite(PIN_IESIRE, deschis);
    digitalWrite(LED_ONBOARD, deschis);
    Serial.print("conn ");Serial.println(pServer->getConnectedCount()); 
    if (time_out) { 
      if (pServer->getConnectedCount()< device_buff) 
      device_buff = pServer->getConnectedCount();   
      timp_deschis = timp_deschis_b; 
    }
             
  }
  
  delay(tic);
}
