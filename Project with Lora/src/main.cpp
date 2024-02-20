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

int id=0;
bool ho_settato_un_altro_esp=false;
int msgCount = 0;            

Preferences pref;
bool data_ready=false;

void readBLE();
void searchAncore();
void sendBLE();
void handle_queaue();
void handle_ack();

Scheduler ts;

Task ReadBLE(10000,TASK_FOREVER,&readBLE,&ts,true);
Task searchAncore_task(10000,TASK_FOREVER,&searchAncore,&ts,true);
Task handle_message_queaue(4000,TASK_FOREVER,&handle_queaue,&ts,true);
Task handle_message_ack_queaue(15000,TASK_FOREVER,&handle_ack,&ts,true);


std::list<char*> messaggi_in_arrivo;
bool data_avaible=false; 
std::list<struct_message*> messages_send;

void handle_queaue(){
  Serial.println("Handling queaue");
  if(messaggi_in_arrivo.size()==0){
    Serial.println("No message to handle");
    return;
  }
  char* incoming=messaggi_in_arrivo.front();
  messaggi_in_arrivo.pop_front();
    
  Serial.println("Message received: ");
  
  struct_message* current=new struct_message;
  memcpy(current, incoming, sizeof(struct_message));
  
  Serial.println("Message received: ");
  Serial.print("touched:   ");
  Serial.println(current->touched);

  Serial.print("messageCount:   ");
  Serial.println(current->messageCount);

  Serial.print("source:   ");
  Serial.println(current->source);

  Serial.print("text:   ");
  Serial.println(current->text);

  Serial.print("type:   ");
  Serial.println(current->type);

  Serial.print("dest:   ");
  Serial.println(current->dest);

  free(current);
  free(incoming);

  Serial.println(esp_get_free_heap_size());
}

void handle_ack(){
  Serial.println("Handling ack");
  if(messages_send.size()==0){
    Serial.println("No message to handle");
    return;
  }
  struct_message* current=messages_send.front();
  if(current->touched[0]=='1'){
    Serial.println("Message touched");
    messages_send.pop_front();
  }
  else{
    current->touched[0]='1';
    Serial.println("Message not touched");
  }

  Serial.println(esp_get_free_heap_size());
}


struct_message message;
JsonDocument MacAddress;

//Task SendBLE(100,TASK_IMMEDIATE,&sendBLE,&ts,true);

void OnReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  int i=0;                 
  char* incoming = (char*)malloc(packetSize);
  while (LoRa.available()) {            
    incoming[i] = (char)LoRa.read();
    i++;
  }
  messaggi_in_arrivo.push_back(incoming);
}



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
  if(!data_ready){
    Serial.println("No data to send");
    return;
  }
  Serial.println("Sending BLE");
  String BLEmessage;
  serializeJson(MacAddress, BLEmessage);

  struct_message *msg = new struct_message;

  memcpy(msg->text, BLEmessage.c_str(),BLEmessage.length() +1);
  memcpy(msg->type, "BraceletData\0", 13);
  memcpy(msg->source, String(id).c_str(), String(id).length() +1);
  memcpy(msg->dest, "0\0", 2);
  memcpy(msg->messageCount, String(msgCount).c_str(), String(msgCount).length() +1);
  memcpy(msg->touched, "0\0", 2);
  msgCount++;
  pref.putInt("msgCount",msgCount);

  char buffer[sizeof(struct_message)+1];
  memcpy(buffer, msg, sizeof(struct_message));
  buffer[sizeof(struct_message)]='\0';

  messages_send.push_back(msg);

  LoRa.beginPacket();
  for(int i=0;i<sizeof(struct_message)+1;i++){
    LoRa.write(buffer[i]);
  }
  LoRa.endPacket();

  Serial.println("Message sent");
  LoRa.receive(); 
  data_ready=false;
}



void readBLE(){
  Serial.println("Reading BLE");
  MacAddress.clear();
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

    if(peripheral.haveName()&&String(peripheral.getName().c_str()) =="Braccialetto"){
      pClient->connect(peripheral.getAddress());
      BLERemoteService* pRemoteService = pClient->getService(SERVICE_UUID_bracelet);
      if(pRemoteService==nullptr){
        Serial.println("service not found");
        continue;
      }

      String temperature=read_BLE_charcteristic(pRemoteService, TEMPERATURE_CHARACTERISTIC_UUID);
      String saturation=read_BLE_charcteristic(pRemoteService, SATURATION_CHARACTERISTIC_UUID);
      String heartbeat=read_BLE_charcteristic(pRemoteService, HEARTBEAT_CHARACTERISTIC_UUID);

      char buffer[100];
      snprintf(buffer, sizeof(buffer), "{%s,%s,%s}", 
        temperature.c_str(), saturation.c_str(), heartbeat.c_str());
        
      MacAddress[String(peripheral.getAddress().toString().c_str())]=buffer;
      pClient->disconnect();
      data_ready=true;
    }
    else{     
      continue;
    }
    Serial.println(peripheral.getName().c_str());
  }
  
  serializeJsonPretty(MacAddress, Serial);
  sendBLE();
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
  pref.putBool("set_esp",true);
  ho_settato_un_altro_esp=pref.getBool("set_esp");
  msgCount=pref.getInt("msgCount");
  LoRa.enableCrc();
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
      LoRa.receive();
    }else{
      Serial.println("non ho ancora settato un altro esp");
      ReadBLE.disable();
      searchAncore_task.enable();
      
    }
    
  }
  //ReadBLE.disable();
  Serial.println("ID: "+String(id));
}

void loop() {
  
  ts.execute();
}