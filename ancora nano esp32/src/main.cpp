#include <esp_now.h>
#include <WiFi.h>
#include <TaskScheduler.h>
#include <ArduinoJson.h>

#include <Arduino.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEclient.h>
#include <BLEScan.h>

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define SERVICE_UUID_bracelet "4fafc201-1fb5-459e-8fcc-c5c9c331914b"

bool is_broadcast(uint8_t* mac){
  return (mac[0]==0xff&&
          mac[1]==0xff&&
          mac[2]==0xff&&
          mac[3]==0xff&&
          mac[4]==0xff&&
          mac[5]==0xff
          );
}


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

// REPLACE WITH THE MAC Address of your receiver 
uint8_t remote_ble[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t remote_wifi_next[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
uint8_t remote_wifi_prec[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

// Variable to store if sending data was successful
String success;

typedef struct struct_message {
    char type[20];
    char text[100]; 
    char source[30];
    char dest[30]; 
} struct_message;

String temp;

// Create a struct_message to hold incoming data
struct_message incomingReadings;

// Create a struct_message to send data
struct_message message;

esp_now_peer_info_t peerInfo;
esp_now_peer_info_t peerInfo2;
esp_now_peer_info_t peerInfo3;

Scheduler ts;

void readBLE();
void sendBLE();

Task ReadBLE(30000,TASK_FOREVER,&readBLE,&ts,true);
//Task SendBLE(100,TASK_IMMEDIATE,&sendBLE,&ts,true);

bool data_ready=false;
JsonDocument MacAddress;

#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SATURATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define HEARTBEAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a0"

bool send_data_to(uint8_t* dest,uint8_t * da_spedire){

  if(!is_broadcast(dest)){
    esp_err_t result = esp_now_send(dest, da_spedire, sizeof(message));
    if (result == ESP_OK) {
    Serial.println("Sent with success");
    Serial.println();
    return true;
    }
    else {
      Serial.println("Error sending the data");
      Serial.println();
      return false;
    }
  }
  else{
    Serial.println("sarebbe Broadcast beddu");
    return false;
  }
}

void sendBLE(){
  if(MacAddress.size()==0&&data_ready){
    return;
  }
  String BLEmessage;
  serializeJson(MacAddress, BLEmessage);

  strcpy(message.type,String("BraceletData").c_str());
  message.type[String("BraceletData").length()]='\0';

  strcpy(message.source,WiFi.macAddress().c_str());
  message.source[WiFi.macAddress().length()]='\0';

  strcpy(message.text,BLEmessage.c_str());
  message.text[BLEmessage.length()]='\0';

  strcpy(message.dest,String("1:1:1:1:1:1").c_str());
  message.dest[String("1:1:1:1:1:1").length()]='\0';
  
  int retry=0;
  while(!send_data_to(remote_wifi_prec,(uint8_t *) &message)){
    retry++;
        if(retry==10)   
          break;
  }

  retry=0;
  while(!send_data_to(remote_wifi_next,(uint8_t *) &message)){
    retry++;
        if(retry==10)   
          break;
  }

  data_ready=false;
  MacAddress.clear();
}

void readBLE(){
  
  BLEScan* pBLEScan = BLEDevice::getScan();
  BLEScanResults foundDevices = pBLEScan->start(5);

  for(int i = 0; i < foundDevices.getCount(); i++){
    BLEAdvertisedDevice peripheral=foundDevices.getDevice(i);

    if(peripheral.getName()=="Braccialetto"){

      BLEClient*  pClient  = BLEDevice::createClient();
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


// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nLast Packet Send Status:\t");
  //Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("type:  ");
  Serial.println(incomingReadings.type);
  Serial.print("source:  ");
  Serial.println(incomingReadings.source);
  Serial.print("dest:  ");
  Serial.println(incomingReadings.dest);
  Serial.print("text:  ");
  Serial.println(incomingReadings.text);


  Serial.println("");

  if(String(incomingReadings.type).equals("new node")){
    macStrToByteArray(incomingReadings.source,remote_wifi_next);
    Serial.println("nuovo nodo aggiunto");

    memcpy(peerInfo3.peer_addr, remote_wifi_next, 6);
    peerInfo3.channel = 1;  
    peerInfo3.encrypt = false;

    if(esp_now_add_peer(&peerInfo3)!= ESP_OK){
      Serial.println("Failed to add discovered peer");
      return;
    }
    BLEDevice::deinit(true);
  }else if(String(incomingReadings.dest).equals(WiFi.macAddress())){
    Serial.println("arrivato a destinazione");
    Serial.println(incomingReadings.text);
  }
  else{
    Serial.println("inoltro al prossimo");

    char remote_wifi_prec_str[18];
    snprintf(remote_wifi_prec_str, sizeof(remote_wifi_prec_str), "%02x:%02x:%02x:%02x:%02x:%02x",
         remote_wifi_prec[0], remote_wifi_prec[1], remote_wifi_prec[2],
          remote_wifi_prec[3], remote_wifi_prec[4], remote_wifi_prec[5]);


    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
         mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);

    String mac_String(mac_str);
    
    if(mac_String.equals(remote_wifi_prec_str)){
      Serial.println("inoltro a next");
      int retry=0;
      while(!send_data_to(remote_wifi_next,(uint8_t *) &incomingReadings)){
        retry++;
        if(retry==10)   
          break;
      }

    }
    else{
      Serial.println("inoltro a prec");
      int retry=0;
      while(!send_data_to(remote_wifi_prec,(uint8_t *) &incomingReadings)){
        retry++;
        if(retry==10)   
          break;
      }
    }

  }

}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println();

  
  delay(3000);
  while(true){
    Serial.println("Scanning for Ancora");
    //scan ble for name Ancora
    BLEScan* pBLEScan = BLEDevice::getScan();
    BLEScanResults foundDevices = pBLEScan->start(5);
    
    int i=0;
    BLEAdvertisedDevice peripheral=foundDevices.getDevice(i);
    
    while(peripheral.getName()!="Ancora"){
      i++;
      peripheral=foundDevices.getDevice(i);
    }

    Serial.print("Address: "); 
    Serial.println(peripheral.getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->connect(peripheral.getAddress());
    BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID);
    if(pRemoteService==nullptr){
      Serial.println("service not found");
      continue;
    }

    BLERemoteCharacteristic* pRemoteCharacteristic=  pRemoteService->getCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID);
    if(pRemoteCharacteristic==nullptr){
      Serial.println("characteristic not found");
      continue;
    }
    String remoteaddress=pRemoteCharacteristic->readValue().c_str();

    macStrToByteArray(String(peripheral.getAddress().toString().c_str()),remote_ble);

    macStrToByteArray(remoteaddress,remote_wifi_prec);
    
    for(int i = 0; i < 6; i++){
      Serial.print(remote_ble[i],HEX);
      Serial.print(":");
    }

    Serial.println();
    Serial.println("Wifi Address: ");
    Serial.println(WiFi.macAddress());

    pClient->disconnect();
    
    break;
    
  }

  BLEDevice::init("Ancora");
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  pTemperatureCharacteristic = pService->createCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ );
  pTemperatureCharacteristic->setCallbacks(new MyCharacteristicCallbacks());
  String mac = WiFi.macAddress();
  pTemperatureCharacteristic->setValue(mac.c_str());
  pService->start();
  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);
  BLEDevice::startAdvertising();


  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer broadcast
  for(int i = 0; i < 6; i++){
    Serial.print(broadcastAddress[i],HEX);
    Serial.print(":");
  }
  Serial.println();

  memcpy(peerInfo2.peer_addr, broadcastAddress, 6);
  peerInfo2.channel = 1;  
  peerInfo2.encrypt = false;
  if(esp_now_add_peer(&peerInfo2)!= ESP_OK){
    Serial.println("Failed to add broadcastAddress peer");
    return;
  }


  for(int i = 0; i < 6; i++){
    Serial.print(remote_wifi_prec[i],HEX);
    Serial.print(":");
  }
  Serial.println();

  // Register peer wifi
  memcpy(peerInfo.peer_addr, remote_wifi_prec, 6);
  peerInfo.channel = 1;  
  peerInfo.encrypt = false;
  // Add peer
  if(esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer");
    return;
  }
    
  strcpy(message.type,String("new node").c_str());
  message.dest[String("new node").length()]='\0';

  strcpy(message.source,WiFi.macAddress().c_str());
  message.dest[WiFi.macAddress().length()]='\0';

  strcpy(message.text,String("").c_str());
  message.dest[String("").length()]='\0';


  char mac_str[18];
  snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
        remote_wifi_prec[0], remote_wifi_prec[1], remote_wifi_prec[2],
        remote_wifi_prec[3], remote_wifi_prec[4], remote_wifi_prec[5]);

  strcpy(message.dest,String(mac_str).c_str());
  message.dest[String(mac_str).length()]='\0';

  int retry=0;
  while(!send_data_to(remote_wifi_prec,(uint8_t *) &message)){ 
    retry++;
      if(retry==10)   
        break;
  }
  ReadBLE.disable();
  //SendBLE.disable();
  ts.addTask(ReadBLE);
  //ts.addTask(SendBLE);
  ReadBLE.enable();
  //SendBLE.enable();

  ts.startNow();
}

void loop() {
  
  ts.execute();
}