#include <BLEDevice.h>
#include <SPI.h>     
#include <Arduino.h>
#include <TaskScheduler.h>
#include <Preferences.h>

BLEClient* pClient;
BLEAdvertisedDevice myAncora;
bool AncoraFound = false;
BLEScan *pBLEScan;

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define ID_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a7"


#define SERVICE_UUID_bracelet "4fafc201-1fb5-459e-8fcc-c5c9c331914b"
#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SATURATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define HEARTBEAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a0"
#define COLESTEROL_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a3"
#define SUGAR_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a4"

#define SERVICE_UUID_FOR_SEX_AND_AGE "4fafc201-1fb5-459e-8fcc-c5c9c331914j"
#define DEAD_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a5"
#define RUN_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a6"

class callbackgenerica: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
   //printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
  }
};


class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer) {
    //BLEDevice::startAdvertising();
    Serial.println("Device disconnected");
  }
};

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
class callbackSetId : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic);
};

void StartAdvertisingToSetID(){
  
  BLECharacteristic *pAncoraCharacteristic;
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




BLERemoteCharacteristic* pRemoteCharacteristic1;
String read_BLE_charcteristic(BLERemoteService* pRemoteService, const char* charUUID){
  
  //Serial.println("Reading characteristic");
  pRemoteCharacteristic1 = pRemoteService->getCharacteristic(charUUID);
  if (pRemoteCharacteristic1 == nullptr) {
    Serial.print("Failed to find characteristic UUID: ");
    Serial.println(charUUID);
    //free(pRemoteCharacteristic);
    return "fail";
  }
  if(pRemoteCharacteristic1->canRead()){
    std::string value = pRemoteCharacteristic1->readValue();
    //free(pRemoteCharacteristic);
    return String(value.c_str());
  }
  //free(pRemoteCharacteristic);
  return "fail";

}

extern Task searchAncore_task;
extern Preferences pref;
extern Task ReadBLE;
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
      while(!pClient->isConnected()){
        delay(100);
      }
      BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
      while(pRemoteService==nullptr){
        pRemoteService = pClient->getService(SERVICE_UUID);
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
extern int id;

void callbackSetId::onWrite(BLECharacteristic *pCharacteristic) {
    String value = pCharacteristic->getValue().c_str();
    if (value.toInt() != 0) {
      Serial.println("New value: " + value);
      id=value.toInt();
      pref.putInt("id",id);
      BLEDevice::stopAdvertising();
      
      ReadBLE.disable();
      searchAncore_task.enable();
      //LoRa.receive();
  
    }
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



