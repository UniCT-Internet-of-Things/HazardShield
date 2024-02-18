#include <Arduino.h>
#include <SPI.h>              // include libraries
#include <LoRa.h>
#include <TaskScheduler.h>
#include <ArduinoJson.h>
#include <Preferences.h>
#include <function/functions.cpp>

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


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer) {
    //BLEDevice::startAdvertising();
    Serial.println("Device disconnected");
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {
  }
};


BLECharacteristic *pTemperatureCharacteristic;

// Funzione per convertire una stringa MAC in un array di byte
void macStrToByteArray(const String &macStr, uint8_t *macArray) {
    // Controllo se la lunghezza della stringa MAC è corretta
    if (macStr.length() != 17) {
        Serial.println("Formato MAC non valido");
        return;
    }

    char hexNum[3]; // Buffer temporaneo per memorizzare coppie di cifre esadecimali
    for (int i = 0; i < 6; ++i) {
        hexNum[0] = macStr.charAt(i * 3);
        hexNum[1] = macStr.charAt(i * 3 + 1);
        hexNum[2] = '\0'; // Terminatore di stringa per assicurare una corretta conversione

        // Converti la coppia di cifre esadecimali in un byte e memorizzalo nell'array
        macArray[i] = strtoul(hexNum, NULL, 16);
    }
}

uint8_t remote_mac_next[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t remote_mac_prec[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

String remote_mac_next_str = "FF:FF:FF:FF:FF:FF";
String remote_mac_prec_str = "FF:FF:FF:FF:FF:FF";

typedef struct struct_message {
    char type[20];
    char text[100]; 
    char source[30];
    char dest[30]; 
} struct_message;

Scheduler ts;

void readBLE();
void sendBLE();

Task ReadBLE(10000,TASK_FOREVER,&readBLE,&ts,true);
//Task SendBLE(100,TASK_IMMEDIATE,&sendBLE,&ts,true);

JsonDocument MacAddress;
bool data_ready=false;

void sendBLE(){

}

void readBLE(){
  Serial.println("Reading BLE");
  BLEScan* pBLEScanBraccialetto = BLEDevice::getScan();
  pBLEScanBraccialetto->setAdvertisedDeviceCallbacks(new callbackgenerica());
  BLEScanResults foundDevices = pBLEScanBraccialetto->start(5);

  for(int i = 0; i < foundDevices.getCount(); i++){
    BLEAdvertisedDevice peripheral=foundDevices.getDevice(i);

    if(peripheral.haveName()&&peripheral.getName()=="Braccialetto"){
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
      snprintf(buffer, sizeof(buffer), "{Temperature: %s, Saturation: %s, Heartbeat: %s}", 
        temperature.c_str(), saturation.c_str(), heartbeat.c_str());
        
      MacAddress[String(peripheral.getAddress().toString().c_str())]=buffer;
      pClient->disconnect();

    }
    else{     
      continue;
    }
    Serial.println(peripheral.getName().c_str());
  }
  
  data_ready=true;
  serializeJsonPretty(MacAddress, Serial);
  sendBLE();
}


void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  String incoming = "";                 // payload of packet

  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  Serial.println("Message: " + incoming);
  // LoRa.beginPacket();
  // String messagge = nominativo + value.c_str();
  // LoRa.print(messagge);
  // LoRa.endPacket();
  // LoRa.receive();
  
}

BLECharacteristic *pAncoraCharacteristic;


void SetIDAncora(){
  
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pAncoraCharacteristic = pService->createCharacteristic(ID_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ );
  pAncoraCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  pAncoraCharacteristic->setValue("");
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();
}
Preferences pref;

void setup(){

  pref.begin("my_id", false); 
  Serial.begin(115200);
  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500); 
  }
  LoRa.setSyncWord(0xffff);
  LoRa.onReceive(onReceive);
  BLEDevice::init("Ancora");
  pClient = BLEDevice::createClient();


  int id=pref.getInt("id");

  if(id==0){
    String ID=scan_ancore_blocked();
    pref.putInt("id",ID.toInt()+1);
  }
  else{
    //non è la prima volta che si accende
  }

  Serial.println("ID: "+String(id));
  
  
  LoRa.receive();
  Serial.println("LoRa Initializing OK!");
}

void loop() {
  
  delay(1000);
}