#include <esp_now.h>
#include <WiFi.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEclient.h>

#include <BLEScan.h>
#include <TaskScheduler.h>
#include <ArduinoJson.h>
#include <LoRa.h>

#define number_of_retry 10

#define SERVICE_UUID "4fafc201-1fb5-459e-8fcc-c5c9c331914c"
#define SERVICE_UUID_bracelet "4fafc201-1fb5-459e-8fcc-c5c9c331914b"


#define TEMPERATURE_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define SATURATION_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a9"
#define HEARTBEAT_CHARACTERISTIC_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a0"

#define number_of_retry 10
#define ss 18
#define rst 23
#define dio0 26

bool search_nodo=false;
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
  
  pBLEScan->setAdvertisedDeviceCallbacks(new callbackgenerica());
}


bool is_broadcast(uint8_t* mac){
  return (mac[0]==0xff&&
          mac[1]==0xff&&
          mac[2]==0xff&&
          mac[3]==0xff&&
          mac[4]==0xff&&
          mac[5]==0xff
          );
}

bool i_m_gateway=true; 



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
    //Serial.println("sarebbe Broadcast beddu");
    return false;
  }
}


void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
   if(status != ESP_NOW_SEND_SUCCESS){
    Serial.println("Error sending the data");

    //converto il mac in stringa 
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
        mac_addr[0], mac_addr[1], mac_addr[2],
        mac_addr[3], mac_addr[4], mac_addr[5]);
    String mac_String(mac_str);

    
      Serial.println("Errore invio a prec");
      esp_now_del_peer(remote_wifi_prec);
      search_nodo=true;
    
  }
}

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&incomingReadings, incomingData, sizeof(incomingReadings));
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("type:  ");
  Serial.println(incomingReadings.type);
  Serial.print("dest:  ");
  Serial.println(incomingReadings.dest);
  Serial.print("text:  ");
  Serial.println(incomingReadings.text);

  Serial.print("source:");
  Serial.println(incomingReadings.source);

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
    BLEDevice::stopAdvertising();
  }else if(String(incomingReadings.dest).equals(WiFi.macAddress())){
    Serial.println("arrivato a destinazione");
    Serial.println(incomingReadings.text);
  }
  else{
    
    Serial.println("invio lora");
    LoRa.beginPacket();
    String LoraMessage = String("{\"Sorgente\": \"") + String(incomingReadings.source) + String("\", \"messaggio\": \"") + String(incomingReadings.text) + String("\"}"); 
    LoRa.print(LoraMessage);
    LoRa.endPacket();
    

  }
}

void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(866E6)) {
      Serial.println(".");
      delay(500); 
  }

  LoRa.setSyncWord(0xffff);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println();

  BLEDevice::init("Ancora");
  delay(3000);

  Search_ancora_blocked();

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
      if(retry==number_of_retry)   
        break;
  }
  
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
}
