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
String msg_to_bracelet = "0";      

Preferences pref;
bool data_ready=false;

void readBLE();
void searchAncore();
void sendBLE();
void handle_queaue();
void handle_ack();

Scheduler ts;

Task ReadBLE(15000,TASK_FOREVER,&readBLE,&ts,true);
Task searchAncore_task(10000,TASK_FOREVER,&searchAncore,&ts,true);
Task handle_message_queaue(100,TASK_FOREVER,&handle_queaue,&ts,true);
Task handle_message_ack_queaue(20000,TASK_FOREVER,&handle_ack,&ts,true);


std::list<char*> messaggi_in_arrivo;
bool data_avaible=false; 
std::list<struct_message*> messages_send;

void handle_queaue(){

  if(messaggi_in_arrivo.size()==0){
    return;
  }
  Serial.println("Handling queaue");
  char* incoming=messaggi_in_arrivo.front();
  messaggi_in_arrivo.pop_front();
    
  Serial.println("Message received");
  struct_message* current=new struct_message;
  memcpy(current, incoming, sizeof(struct_message));

  bool ho_inviato_un_message=false;

  if (String(current->dest).toInt() == pref.getInt("id")&&
      (String(current->source).toInt() == id+1||
      String(current->source).toInt() == id-1)){
    if(String(current->type)=="ACK"){
      for (std::list<struct_message*>::iterator it = messages_send.begin(); it != messages_send.end(); ++it){
        if(String((*it)->messageCount)==String(current->text)){
          Serial.println("ACK eliminato");
          //elimina it da messages_send
          Serial.println((*it)->messageCount);
          messages_send.erase(it);
          break;
        }
      }
    }
    else if(String(current->type)=="MSG_to_bracelet"){
      //scrivere come inviare un messaggio per i braccialetti
      //e gestire l'inoltro del messaggio se non conosco il destinatario
      msg_to_bracelet=String(current->text);
      ho_inviato_un_message=true;
    }
    else if(String(current->type)=="BraceletData"){
      // io sono il gateway se ricevo dei braceletdata
      // devo inviarli al server
      //send_string_to_server(String("{\""+ String(current->original_sender) +"\":\""+current->text+"\"}"));
      ho_inviato_un_message=true;
    }
  }
  else{
    //il messaggio non è per me ma forse lo devo inoltrare  
    
    if(String(current->dest).toInt() < id &&
      String(current->source).toInt() == (pref.getInt("id")+1)){
        //il messaggio proviene da destra ed è per sinistra
        //quindi lo inoltro a sinistra
        char buffer[sizeof(struct_message)+1];

        struct_message* inoltro = new struct_message;

        memset(inoltro,0,sizeof(struct_message));

        memcpy(inoltro->type, current->type, String(current->type).length()+1);
        String temp_dest(current->dest);
        memcpy(inoltro->dest, current->dest, String(current->dest).length()+1);
        memcpy(inoltro->original_sender, current->original_sender, String(current->original_sender).length()+1);
        memcpy(inoltro->source, String(id).c_str(), String(id).length()+1);
        memcpy(inoltro->messageCount, String(msgCount).c_str(), String(msgCount).length()+1);  
        msgCount++;
        pref.putInt("msgCount",msgCount);
        memcpy(inoltro->touched, "0\0", 2);
        memcpy(inoltro->text, current->text, String(current->text).length()+1);
        
        messages_send.push_back(inoltro);

        memcpy(buffer, inoltro, sizeof(struct_message));
        buffer[sizeof(struct_message)]='\0';

        LoRa.beginPacket();
        for(int i=0;i<sizeof(struct_message)+1;i++){
          LoRa.write(buffer[i]);
        }
        LoRa.endPacket();
        delay(500);
        ho_inviato_un_message=true;
      }

    if(String(current->dest).toInt() > id &&
      String(current->source).toInt() == (pref.getInt("id")-1)){
      //se il messaggio proviene da sinistra ed è per un nodo alla destra
      //lo inoltro a destra
      char buffer[sizeof(struct_message)+1];

      struct_message* inoltro = new struct_message;

      memset(inoltro,0,sizeof(struct_message));

      memcpy(inoltro->type, current->type, String(current->type).length()+1);
      String temp_dest(current->dest);
      memcpy(inoltro->dest, current->dest, String(current->dest).length()+1);
      memcpy(inoltro->original_sender, current->original_sender, String(current->original_sender).length()+1);
      memcpy(inoltro->source, String(id).c_str(), String(id).length()+1);
      memcpy(inoltro->messageCount, String(msgCount).c_str(), String(msgCount).length()+1);  
      msgCount++;
      pref.putInt("msgCount",msgCount);
      memcpy(inoltro->touched, "0\0", 2);
      memcpy(inoltro->text, current->text, String(current->text).length()+1);
      
      messages_send.push_back(inoltro);

      memcpy(buffer, inoltro, sizeof(struct_message));
      buffer[sizeof(struct_message)]='\0';

      LoRa.beginPacket();
      for(int i=0;i<sizeof(struct_message)+1;i++){
        LoRa.write(buffer[i]);
      }
      LoRa.endPacket();
      delay(500);
      ho_inviato_un_message=true;
    }
      
    else{
      Serial.println("Message not for me");
    }
  }

  if(ho_inviato_un_message){
      //se ho inoltrato un messaggio devo anche inviare l'ack per
      //dire che ho ricevuto il messaggio

      Serial.println("Inoltrato messaggio da: "+String(current->source)+" a: "+String(current->dest));
      struct_message ack;

      memset(&ack,0,sizeof(struct_message));

      memcpy(ack.type, "ACK\0", 4);
      memcpy(ack.dest, current->source, String(current->source).length()+1);
      memcpy(ack.original_sender, String(id).c_str(), String(id).length() +1);
      memcpy(ack.source, String(id).c_str(), String(id).length() +1);
      String temp_msgCount = current->messageCount;
      memcpy(ack.messageCount, String(msgCount).c_str(), String(msgCount).length()+1);  
      memcpy(ack.touched, "0\0", 2);
      memcpy(ack.text, temp_msgCount.c_str(),temp_msgCount.length()+1);
      
      Serial.println("ack source:"+String(ack.source));
      Serial.println("current dest"+String(current->dest));
      msgCount++;
      pref.putInt("msgCount",msgCount);

      char buffer[sizeof(struct_message)+1];
      memcpy(buffer, &ack, sizeof(struct_message));
      buffer[sizeof(struct_message)]='\0';  
      LoRa.beginPacket();
      for(int i=0;i<sizeof(struct_message)+1;i++){
        LoRa.write(buffer[i]);
      }
      LoRa.endPacket();
      delay(500);
      Serial.println("ACK sent");
      
    }
    
  free(current);
  free(incoming);
  LoRa.receive(); 
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
    free(current);
  }
  else{
    current->touched[0]='1';
    Serial.println(current->messageCount);
    Serial.println("Message not touched");
  }

  Serial.println(esp_get_free_heap_size());
}


struct_message message;
JsonDocument MacAddress;

void OnReceive(int packetSize) {
  if (packetSize == 0) return; 
  //Serial.println(packetSize);
  int i=0;                 
  char* incoming = (char*)malloc(packetSize);
  while (LoRa.available()) {            
    incoming[i] = (char)LoRa.read();
    i++;
  }
  messaggi_in_arrivo.push_back(incoming);
  LoRa.receive(); 
}



extern BLEClient*  pClient;
extern BLEAdvertisedDevice myAncora;
extern bool AncoraFound;
extern BLEScan *pBLEScan;


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
  memcpy(msg->original_sender, String(id).c_str(), String(id).length() +1);
  
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
  delay(500);
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
        Serial.println("service 1 not found");
        Serial.println("service 1 not found");
        continue;
      }

      String temperature=read_BLE_charcteristic(pRemoteService, TEMPERATURE_CHARACTERISTIC_UUID);
      String saturation=read_BLE_charcteristic(pRemoteService, SATURATION_CHARACTERISTIC_UUID);
      String heartbeat=read_BLE_charcteristic(pRemoteService, HEARTBEAT_CHARACTERISTIC_UUID);
      String colesterol=read_BLE_charcteristic(pRemoteService, COLESTEROL_CHARACTERISTIC_UUID);
      String sugar=read_BLE_charcteristic(pRemoteService, SUGAR_CHARACTERISTIC_UUID);

      BLERemoteService* pRemoteService1 = pClient->getService(SERVICE_UUID_FOR_SEX_AND_AGE);
      if(pRemoteService1==nullptr){
        Serial.println("service 2 not found");
        continue;
      }

      String dead = read_BLE_charcteristic(pRemoteService1, DEAD_CHARACTERISTIC_UUID);
      BLERemoteCharacteristic* pRunCharacteristic = pRemoteService1->getCharacteristic(RUN_CHARACTERISTIC_UUID);
      if (pRunCharacteristic == nullptr) {
        Serial.println("Failed to find run characteristic UUID");
      }
      else{
        if(msg_to_bracelet != "0"){
          int index1 = msg_to_bracelet.indexOf(":\"");
          int index2 = msg_to_bracelet.indexOf("\"}'");
          String temp = msg_to_bracelet.substring(index1+2,index2);
          if(msg_to_bracelet.indexOf("FF:FF:FF:FF:FF:FF")!=-1){
            //qui ci vorrebbe una riga per prendere solo il messaggio e non tutto il json
            pRunCharacteristic->writeValue(temp.c_str());
          }
          if(msg_to_bracelet.indexOf(peripheral.getAddress().toString().c_str())!=-1){
            //qui ci vorrebbe una riga per prendere solo il messaggio e non tutto il json
            pRunCharacteristic->writeValue(temp.c_str());
            msg_to_bracelet="0";
          }
        }
      }

      char buffer[100];
      snprintf(buffer, sizeof(buffer), "{%s,%s,%s,%s,%s,%s}", 
        temperature.c_str(), saturation.c_str(), heartbeat.c_str(), colesterol.c_str(), sugar.c_str(), dead.c_str());
      snprintf(buffer, sizeof(buffer), "{%s,%s,%s,%s,%s,%s}", 
        temperature.c_str(), saturation.c_str(), heartbeat.c_str(), colesterol.c_str(), sugar.c_str(), dead.c_str());
        
      MacAddress[String(peripheral.getAddress().toString().c_str())]=buffer;
      pClient->disconnect();
      data_ready=true;
    }
    else{     
      continue;
    }
    Serial.println(peripheral.getName().c_str());
    msg_to_bracelet="0";
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
  LoRa.onReceive(OnReceive);
  Serial.println("LoRa Initializing OK!");

  BLEDevice::init("Ancora");
  pClient = BLEDevice::createClient();

  //pref.putBool("set_esp",true); //ricordiamoci di metterlo a false nella versione finale
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
      LoRa.receive();
    }else{
      Serial.println("non ho ancora settato un altro esp");
      searchAncore_task.enable();
      ReadBLE.disable();
      
    }
    
  }
  //ReadBLE.disable();
  Serial.println("ID: "+String(id));
  //LoRa.disableCrc();
}

void loop() {
  
  ts.execute();
}