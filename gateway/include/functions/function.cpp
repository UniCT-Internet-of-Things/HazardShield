
#include <HTTPClient.h>
#include <Arduino.h>
#include <WiFi.h>

extern String base_url;


#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define ID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a7"


#define SERVICE_UUID_bracelet "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SATURATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define HEARTBEAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a0"


BLEClient* pClient;
BLEAdvertisedDevice myAncora;
bool AncoraFound = false;
BLEScan *pBLEScan;


bool send_ip(){
HTTPClient http;
  String url=base_url+"register_ip";
  http.begin(url);
  http.addHeader("Content-Type", "Content-Type: application/json"); 

  int httpResponseCode = http.POST(String("{\"my_ip\":\"")+String(WiFi.localIP())+String("\"}"));

  if(httpResponseCode>0){
    String response = http.getString();  //Get the response to the request
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);        //Print request answer
    http.end();     
    return true;     
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);
    
    http.end();
    return false;
  }
}

class callbackgenerica: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
   //printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
  }
};

extern Task searchAncore_task;
extern Preferences pref;
extern int id;

void searchAncore(){
  Serial.println("Reading BLEfor ancore");
  
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setAdvertisedDeviceCallbacks(new callbackgenerica());
  pBLEScan->start(5);
  BLEScanResults foundDevices = pBLEScan->getResults();

  for(int i = 0; i < foundDevices.getCount(); i++){
    BLEAdvertisedDevice peripheral=foundDevices.getDevice(i);
    
    //Serial.println(peripheral.getAddress().toString().c_str());
    if(peripheral.haveName())
      Serial.println(String(peripheral.getName().c_str()));

    if(peripheral.haveName()&&String(peripheral.getName().c_str()) =="Ancora"){

      pClient->connect(peripheral.getAddress());
      BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
      if(pRemoteService==nullptr){
        Serial.println("service not found");
        continue;
      }

      BLERemoteCharacteristic* pRemoteCharacteristic=  pRemoteService->getCharacteristic(ID_CHARACTERISTIC_UUID);
      if(pRemoteCharacteristic==nullptr){
        Serial.println("characteristic not found");
        continue;
      }
      pRemoteCharacteristic->writeValue(String(id+1).c_str());
    
      pClient->disconnect();
      searchAncore_task.disable();
      pref.putBool("set_esp",true);
      break;

    }
  }
}


bool send_string_to_server(String toSend){
  HTTPClient http;
  String url=base_url+"get_data";
  http.begin(url); 
  http.addHeader("Content-Type", "Content-Type: application/json"); 

  int httpResponseCode = http.POST(toSend); //Send the actual POST request

  if(httpResponseCode>0){
    String response = http.getString();  //Get the response to the request
    Serial.println(httpResponseCode);   //Print return code
    Serial.println(response);        //Print request answer
    http.end();          
  } else {
    Serial.print("Error on sending POST: ");
    Serial.println(httpResponseCode);

    http.end();

  }

  Serial.println("Messaggio ricevuto");
  SendToServer = false;
}

typedef struct struct_message {
    char type[20];
    char text[100]; 
    char source[8];
    char original_sender[8];
    char dest[8]; 
    char messageCount[12];
    char touched[4];
} struct_message;
