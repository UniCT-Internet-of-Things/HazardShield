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


#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SATURATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define HEARTBEAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a0"


#define number_of_retry 1

bool search_nodo=false;
bool advertising=true;


bool is_broadcast(uint8_t* mac){
  return (mac[0]==0xff&&
          mac[1]==0xff&&
          mac[2]==0xff&&
          mac[3]==0xff&&
          mac[4]==0xff&&
          mac[5]==0xff
          );
}

BLEAdvertisedDevice myAncora;
bool AncoraFound = false;
BLEScan *pBLEScan;
class callbackAncore: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    if(advertisedDevice.haveName()){
      if (String(advertisedDevice.getName().c_str()) == "Ancora") {
        Serial.println("Found our Ancora! " + String(advertisedDevice.getAddress().toString().c_str()));
        myAncora = advertisedDevice;
        AncoraFound = true;
        pBLEScan->stop();
      }
    }
    
  } 
};

class callbackgenerica: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
   //printf("Advertised Device: %s \n", advertisedDevice.toString().c_str());
  }
};

callbackgenerica* pCallbackgenerica=new callbackgenerica();

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer *pServer) {
    //Serial.println("Device connected");
  }

  void onDisconnect(BLEServer *pServer) {
    //BLEDevice::startAdvertising();
    //Serial.println("Device disconnected");
  }
};

class MyCharacteristicCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pCharacteristic) {

  }

};

BLECharacteristic *caratteristica_mac;

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
String remote_wifi_next_str;
uint8_t remote_wifi_prec[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
String remote_wifi_prec_str;

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
String broadcastAddress_str;

void Search_ancora_blocked(){
  while(true){
    Serial.println("Scanning for Ancora");
    //scan ble for name Ancora
    AncoraFound = false;
    pBLEScan = BLEDevice::getScan();
    pBLEScan->setActiveScan(true);
    pBLEScan->setAdvertisedDeviceCallbacks(new callbackAncore());
    pBLEScan->start(5);
    Serial.println("Scan done");
    
    if(!AncoraFound){
      Serial.println("Ancora not found");
      continue;
    }

    Serial.print("Address: "); 
    Serial.println(myAncora.getAddress().toString().c_str());
    
    BLEClient*  pClient  = BLEDevice::createClient();
    pClient->connect(myAncora.getAddress());
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

    macStrToByteArray(String(myAncora.getAddress().toString().c_str()),remote_ble);

    macStrToByteArray(remoteaddress,remote_wifi_prec);
    remote_wifi_prec_str=remoteaddress;
    
    Serial.println("remote_ble:  ");
    for(int i = 0; i < 6; i++){
      Serial.print(remote_ble[i],HEX);
      Serial.print(":");
    }

    pClient->disconnect();
    break;
  }
  pBLEScan->setActiveScan(false);
  pBLEScan->setAdvertisedDeviceCallbacks(pCallbackgenerica);
}

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

Task ReadBLE(50000,TASK_FOREVER,&readBLE,&ts,true);
//Task SendBLE(100,TASK_IMMEDIATE,&sendBLE,&ts,true);

bool data_ready=false;
JsonDocument MacAddress;

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
    //Serial.println("sarebbe Broadcast beddu");
    return false;
  }
}

void sendBLE(){
  if(MacAddress.size()==0&&!data_ready){
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
        if(retry==number_of_retry)   
          break;
  }

  delay(1000);
  
  retry=0;
  while(!send_data_to(remote_wifi_next,(uint8_t *) &message)){
    Serial.println("try invio a next");
    retry++;
        if(retry>=number_of_retry) {
          Serial.println("errore invio a next");
          break;
        }  
  }

  data_ready=false;
  MacAddress.clear();
}

BLEScan* pBLEScanBraccialetto;
BLEClient*  pClient;
BLERemoteService* pRemoteService;
BLERemoteCharacteristic* pRemoteCharacteristic;
void readBLE(){

  Serial.println("Reading BLE");
  pBLEScanBraccialetto = BLEDevice::getScan();
  pBLEScanBraccialetto->setActiveScan(true);
  pBLEScanBraccialetto->setAdvertisedDeviceCallbacks(pCallbackgenerica);
  pBLEScanBraccialetto->start(5);
  
  BLEScanResults foundDevices = pBLEScanBraccialetto->getResults();
  
  for(int i = 0; i < foundDevices.getCount(); i++){
    
    BLEAdvertisedDevice peripheral=foundDevices.getDevice(i);

    if(peripheral.haveName()){

      if(String(peripheral.getName().c_str()) =="Braccialetto"){
              
        pClient->connect(peripheral.getAddress());
        
        pRemoteService = pClient->getService(SERVICE_UUID_bracelet);

        if(pRemoteService==nullptr){
          Serial.println("service not found");
          continue;
        }

        pRemoteCharacteristic=  pRemoteService->getCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID);
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
        data_ready=true;
        pClient->disconnect();
      }
      
      else{     
        continue;
      }
    }
    
  }
  
  foundDevices.dump();
  pBLEScanBraccialetto->clearResults();
  
  if(advertising){
    Serial.println("Advertising Ancora");
    BLEDevice::startAdvertising();
  }


  serializeJsonPretty(MacAddress, Serial);
  sendBLE();
  return;
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("Last Packet Send Status: ");
  Serial.println("mac_addr:  ");
  for(int i = 0; i < 6; i++){
    Serial.print(mac_addr[i],HEX);
    Serial.print(":");
  }
  Serial.println();
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

  Serial.println("Dati ricevuti");
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
    remote_wifi_next_str=String(incomingReadings.source);

    Serial.println("nuovo nodo aggiunto");

    memcpy(peerInfo3.peer_addr, remote_wifi_next, 6);
    peerInfo3.channel = 1;  
    peerInfo3.encrypt = false;

    if(esp_now_add_peer(&peerInfo3)!= ESP_OK){
      Serial.println("Failed to add discovered peer");
      return;
    }
    advertising=false;
    BLEDevice::stopAdvertising();
  }
  else if(String(incomingReadings.dest).equals(WiFi.macAddress())){
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
        if(retry==number_of_retry)   
          break;
      }

    }
    else{
      Serial.println("inoltro a prec");
      int retry=0;
      while(!send_data_to(remote_wifi_prec,(uint8_t *) &incomingReadings)){
        retry++;
        if(retry==number_of_retry)   
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

  Serial.println("my Wifi Address: ");
  Serial.println(WiFi.macAddress());

  BLEDevice::init("Ancora");
  delay(3000);

  Search_ancora_blocked();

  pBLEScan->setAdvertisedDeviceCallbacks(pCallbackgenerica);
  BLEServer *pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());
  BLEService *pService = pServer->createService(SERVICE_UUID);
  caratteristica_mac = pService->createCharacteristic(TEMPERATURE_CHARACTERISTIC_UUID, BLECharacteristic::PROPERTY_READ );
  caratteristica_mac->setCallbacks(new MyCharacteristicCallbacks());
  String mac = WiFi.macAddress();
  caratteristica_mac->setValue(mac.c_str());
  //aggiungi descriptor
  BLEDescriptor *Descrittore_mac = new BLEDescriptor(BLEUUID((uint16_t)0x2901));
  Descrittore_mac->setValue("mac address");
  caratteristica_mac->addDescriptor(Descrittore_mac);
  pService->start();

  BLEAdvertising *pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->addServiceUUID(SERVICE_UUID);
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);

  Serial.println("Advertising Ancora");
  advertising=true;

  pClient  = BLEDevice::createClient();
  
  BLEDevice::startAdvertising();

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }

  esp_now_register_send_cb(OnDataSent);
  esp_now_register_recv_cb(OnDataRecv);

  // Register peer broadcast
  Serial.println("broadcastAddress:  ");
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

  Serial.println("remote_wifi_prec:  ");
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
      if(retry==number_of_retry)   
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
  
  if(search_nodo){
    Search_ancora_blocked();
    search_nodo=false;

    memcpy(peerInfo.peer_addr, remote_wifi_prec, 6);
    peerInfo.channel = 1;  
    peerInfo.encrypt = false;

    // readd peer
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
        if(retry==number_of_retry)   
          break;
    }

  }

  ts.execute();
  //Serial.println(esp_get_free_heap_size());
}