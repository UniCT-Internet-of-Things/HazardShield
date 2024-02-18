#include <Arduino.h>
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <TaskScheduler.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <function/functions.cpp>
#include <list>

#define ss 18
#define rst 23
#define dio0 26

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEclient.h>
#include <BLEScan.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define ID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a7"

#define SERVICE_UUID_bracelet "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SATURATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define HEARTBEAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a0"

int id=0;
bool ho_settato_un_altro_esp=false;

int msgCount = 0;            // count of outgoing messages

Preferences pref;

void readBLE();
void searchAncore();
void sendBLE();

Scheduler ts;

Task ReadBLE(10000,TASK_FOREVER,&readBLE,&ts,true);
Task searchAncore_task(10000,TASK_FOREVER,&searchAncore,&ts,true);

typedef struct struct_message {
    char type[20];
    char text[100]; 
    char source[30];
    char dest[30]; 
    char messageCount[30];
} struct_message;

struct_message message;
JsonDocument MacAddress;
std::list<struct_message*> messages;

//Task SendBLE(100,TASK_IMMEDIATE,&sendBLE,&ts,true);

void OnReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  char incoming[250];
  int i=0;                 

  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming[i] = (char)LoRa.read();      // add bytes one by one
  }

  memcpy(&message, incoming, sizeof(message));
  

  
  // LoRa.beginPacket();
  // String messagge = nominativo + value.c_str();
  // LoRa.print(messagge);
  // LoRa.endPacket();
  // LoRa.receive();
  
}

class callbackSetId : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.toInt() != 0) {
      Serial.println("New value: " + value);
      id=value.toInt();
      pref.putInt("id",id);
      BLEDevice::stopAdvertising();
      
      ReadBLE.disable();
      searchAncore_task.enable();
      LoRa.receive();
  
    }
  }
};

extern BLEClient*  pClient;
extern BLEAdvertisedDevice myAncora;
extern bool AncoraFound;
extern BLEScan *pBLEScan;


BLECharacteristic *pTemperatureCharacteristic;

uint8_t remote_mac_next[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t remote_mac_prec[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

String remote_mac_next_str = "FF:FF:FF:FF:FF:FF";
String remote_mac_prec_str = "FF:FF:FF:FF:FF:FF";



void sendBLE(){
      String BLEmessage;
      serializeJson(MacAddress, BLEmessage);
      messages.push_front(new struct_message);
      memcpy(messages.front()->text, BLEmessage.c_str(),  100);
      memcpy(messages.front()->type, "BraceletData", 20);
      memcpy(messages.front()->source, String(id).c_str(), 30);
      memcpy(messages.front()->dest, "0", 30);
      memcpy(messages.front()->messageCount, String(msgCount).c_str(), 30);
      msgCount++;
      pref.putInt("msgCount",msgCount);

      LoRa.beginPacket();
      LoRa.print((char*)&message);
      LoRa.endPacket();
      LoRa.receive(); 
}


void readBLE(){
  Serial.println("Reading BLE");
  
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setAdvertisedDeviceCallbacks(new callbackgenerica());
  pBLEScan->start(5);
  BLEScanResults foundDevices = pBLEScan->getResults();

  for(int i = 0; i < foundDevices.getCount(); i++){
    BLEAdvertisedDevice peripheral=foundDevices.getDevice(i);
    
    Serial.println(peripheral.getAddress().toString().c_str());
    Serial.println(String(peripheral.getName().c_str()));

    if(peripheral.haveName()&&String(peripheral.getName().c_str()) =="Braccialetto"){
      pClient->connect(peripheral.getAddress());
      BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID_bracelet);
      if(pRemoteService==nullptr){
        Serial.println("service not found");
        continue;
      }

      BLERemoteCharacteristic* pRemoteCharacteristic=  pRemoteService->getCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID);
      if(pRemoteCharacteristic==nullptr){
        Serial.println("characteristic not found");
        continue;
      }

      String temperature=pRemoteCharacteristic->readValue().c_str();

      pRemoteCharacteristic=  pRemoteService->getCharacteristic(SATURATION_CHARACTERISTIC_UUID);
      if(pRemoteCharacteristic==nullptr){
        Serial.println("characteristic not found");
        continue;
      }

      String saturation=pRemoteCharacteristic->readValue().c_str();

      pRemoteCharacteristic=  pRemoteService->getCharacteristic(HEARTBEAT_CHARACTERISTIC_UUID);
      if(pRemoteCharacteristic==nullptr){
        Serial.println("characteristic not found");
        continue;
      }

      String heartbeat=pRemoteCharacteristic->readValue().c_str();

      char buffer[100];
      snprintf(buffer, sizeof(buffer), "{%s,%s,%s}", 
        temperature.c_str(), saturation.c_str(), heartbeat.c_str());
        
      MacAddress[String(peripheral.getAddress().toString().c_str())]=buffer;
      pClient->disconnect();

    }
    else{     
      continue;
    }
    Serial.println(peripheral.getName().c_str());
  }
  
  serializeJsonPretty(MacAddress, Serial);
  sendBLE();
}

void searchAncore(){
  Serial.println("Reading BLEfor ancore");
  
  pBLEScan = BLEDevice::getScan();
  pBLEScan->setActiveScan(true);
  pBLEScan->setAdvertisedDeviceCallbacks(new callbackgenerica());
  pBLEScan->start(5);
  BLEScanResults foundDevices = pBLEScan->getResults();

  for(int i = 0; i < foundDevices.getCount(); i++){
    BLEAdvertisedDevice peripheral=foundDevices.getDevice(i);
    
    Serial.println(peripheral.getAddress().toString().c_str());
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
      ReadBLE.enable();
      break;

    }
  }
}

BLECharacteristic *pAncoraCharacteristic;
void StartAdvertisingToSetID(){
  
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pAncoraCharacteristic = pService->createCharacteristic(ID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ | BLECharacteristic::PROPERTY_WRITE);
  pAncoraCharacteristic->setCallbacks(new callbackSetId());
  pAncoraCharacteristic->setValue("");

  BLEDescriptor *pDescriptor = new BLEDescriptor((uint16_t)0x2901);
  pDescriptor->setValue("ID");
  pAncoraCharacteristic->addDescriptor(pDescriptor);

  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();

}

void setup(){

  pref.begin("my_id", false); 
  Serial.begin(115200);
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500); 
  }
  LoRa.setSyncWord(0xffff);
  Serial.println("LoRa Initializing OK!");

  BLEDevice::init("Ancora");
  pClient = BLEDevice::createClient();
  LoRa.onReceive(OnReceive);

  ho_settato_un_altro_esp=pref.getBool("set_esp");
  msgCount=pref.getInt("msgCount");

  id=pref.getInt("id");
  Serial.println("ID: "+String(id));

  if(id==0){
    Serial.println("First time");
    
    ts.disableAll();
    StartAdvertisingToSetID();
    //String ID=scan_ancore_blocked();
    //pref.putInt("id",ID.toInt()+1);
  }
  else{
    Serial.println("Second time");
    if(ho_settato_un_altro_esp){
      Serial.println("ho gia settato un altro esp");
      ReadBLE.enable();
      searchAncore_task.disable();
    }else{
      Serial.println("non ho ancora settato un altro esp");
      ReadBLE.disable();
      searchAncore_task.enable();
      
    }
    
  }

  Serial.println("ID: "+String(id));
}

void loop() {
  
  ts.execute();
}