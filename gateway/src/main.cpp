#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <LoRa.h>
#include "esp_http_server.h"
#include <ArduinoJson.h>
#include <list>

#include <Preferences.h>
#include <TaskScheduler.h>

#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>
#include <BLEclient.h>
#include <BLEScan.h>

#include <functions/function.cpp>

//SSID of your network
char ssid[] = "UniCT-Team";
//password of your WPA Network
char pass[] = "LabROBO15/16";

#define ss 18
#define rst 23
#define dio0 26

bool SendToServer = false;
String toSend = "";

Scheduler ts;
int id=0;
bool ho_settato_un_altro_esp=false;
int msgCount = 0; 
Preferences pref;


void handle_queaue();
void handle_ack();
void searchAncore();

Task searchAncore_task(10000,TASK_FOREVER,&searchAncore,&ts,true);
Task handle_message_queaue(500,TASK_FOREVER,&handle_queaue,&ts,true);
Task handle_message_ack_queaue(15000,TASK_FOREVER,&handle_ack,&ts,true);

extern BLEClient*  pClient;
extern BLEAdvertisedDevice myAncora;
extern bool AncoraFound;
extern BLEScan *pBLEScan;

BLECharacteristic *pTemperatureCharacteristic;

std::list<char*> messaggi_in_arrivo;
String base_url="http://192.168.70.18:5000/";
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
      String(current->source).toInt() == id+1){

    Serial.println("Message for me");
    if(String(current->type)=="ACK"){
      for (std::list<struct_message*>::iterator it = messages_send.begin(); it != messages_send.end(); ++it){
        if(String((*it)->messageCount)==String(current->text)){
          Serial.println("ACK eliminato");
          //elimina it da messages_send
          messages_send.erase(it);
          break;
        }
      }
    }
    else if(String(current->type)=="MSG_to_bracelet"){
      //scrivere come inviare un messaggio per i braccialetti
      //e gestire l'inoltro del messaggio se non conosco il destinatario
    }
    else if(String(current->type)=="BraceletData"){
      // io sono il gateway se ricevo dei braceletdata
      // devo inviarli al server
      Serial.println("invio al server");
      send_string_to_server(String("{\""+ String(current->original_sender) +"\":"+current->text+"}"));
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

        struct_message* inoltro;

        memset(inoltro,0,sizeof(struct_message));

        memcpy(inoltro->type, current->type, String(current->type).length());
        String temp_dest(current->dest);
        memcpy(inoltro->dest, current->dest, String(current->dest).length());
        memcpy(inoltro->original_sender, current->original_sender, String(current->original_sender).length());
        memcpy(inoltro->source, String(id).c_str(), String(id).length());
        memcpy(inoltro->messageCount, String(msgCount).c_str(), String(msgCount).length());  
        msgCount++;
        pref.putInt("msgCount",msgCount);
        memcpy(inoltro->touched, "0\0", 2);
        memcpy(inoltro->text, current->text, String(current->text).length());
        
        messages_send.push_back(inoltro);

        memcpy(buffer, inoltro, sizeof(struct_message));
        buffer[sizeof(struct_message)]='\0';

        LoRa.beginPacket();
        for(int i=0;i<sizeof(struct_message)+1;i++){
          LoRa.write(buffer[i]);
        }
        LoRa.endPacket();
        ho_inviato_un_message=true;
      }

    if(String(current->dest).toInt() > id &&
      String(current->source).toInt() == (pref.getInt("id")-1)){
      //se il messaggio proviene da sinistra ed è per un nodo alla destra
      //lo inoltro a destra
      char buffer[sizeof(struct_message)+1];

      struct_message* inoltro;

      memset(inoltro,0,sizeof(struct_message));

      memcpy(inoltro->type, current->type, String(current->type).length());
      String temp_dest(current->dest);
      memcpy(inoltro->dest, current->dest, String(current->dest).length());
      memcpy(inoltro->original_sender, current->original_sender, String(current->original_sender).length());
      memcpy(inoltro->source, String(id).c_str(), String(id).length());
      memcpy(inoltro->messageCount, String(msgCount).c_str(), String(msgCount).length());  
      msgCount++;
      pref.putInt("msgCount",msgCount);
      memcpy(inoltro->touched, "0\0", 2);
      memcpy(inoltro->text, current->text, String(current->text).length());
      
      messages_send.push_back(inoltro);

      memcpy(buffer, inoltro, sizeof(struct_message));
      buffer[sizeof(struct_message)]='\0';

      LoRa.beginPacket();
      for(int i=0;i<sizeof(struct_message)+1;i++){
        LoRa.write(buffer[i]);
      }
      LoRa.endPacket();
      ho_inviato_un_message=true;
    }
      
    else{
      Serial.println("Message not for me");
    }
  }

  if(ho_inviato_un_message){
      //se ho inoltrato un messaggio devo anche inviare l'ack per
      //dire che ho ricevuto il messaggio

      Serial.println("Inoltrato messaggio");

      struct_message ack;

      memset(&ack,0,sizeof(struct_message));

      memcpy(ack.type, "ACK\0", 4);
      String temp_dest(current->dest);
      memcpy(ack.dest, current->source, String(current->source).length());
      memcpy(ack.original_sender, current->original_sender, String(current->original_sender).length());
      memcpy(ack.source, temp_dest.c_str(), temp_dest.length());
      String temp_msgCount = current->messageCount;
      memcpy(ack.messageCount, String(msgCount).c_str(), String(msgCount).length());  
      memcpy(ack.touched, "0\0", 2);
      memcpy(ack.text, temp_msgCount.c_str(),temp_msgCount.length());
      
      Serial.print("destinatario nuova ack:");
      Serial.println(ack.dest);
      Serial.print("msgcount nuova ack:");
      Serial.println(ack.text);
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
      Serial.println("ACK sent");
      
    }
    
  free(current);
  free(incoming);
  LoRa.receive(); 
  Serial.println(esp_get_free_heap_size());
}

void handle_ack(){
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
    Serial.println("Message not touched");
  }

  Serial.println(esp_get_free_heap_size());
}

void ricezioneLora(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  int i=0;                 
  char* incoming = (char*)malloc(packetSize);
  while (LoRa.available()) {            
    incoming[i] = (char)LoRa.read();
    i++;
  }
  messaggi_in_arrivo.push_back(incoming);
}


esp_err_t post_handler(httpd_req_t *req)
{
    /* Destination buffer for content of HTTP POST request.
     * httpd_req_recv() accepts char* only, but content could
     * as well be any binary data (needs type casting).
     * In case of string data, null termination will be absent, and
     * content length would give length of string */
    
    char content[req->content_len+1];
    content[req->content_len] = '\0';
    int ret = httpd_req_recv(req, content, req->content_len);
    if (ret <= 0) { 
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }
    Serial.println(content);
    String contenuto(content);
    contenuto.replace("\%3A", ":");
    
    JsonDocument doc;
    doc.clear();

    DeserializationError error = deserializeJson(doc, contenuto);
    
    if (error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      const char resp[] = "URI POST Response";
      httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
      return ESP_FAIL;
    }

    char dest[200]; 
    char text[200];
    
    
    serializeJson(doc["dest"], dest);
    serializeJson(doc["msg"], text);
    
    String cose(dest);
    cose.replace("\"", "");
    
    struct_message* inoltro=new struct_message();
    memset(inoltro,0,sizeof(struct_message));

    memcpy(inoltro->type, "MSG_to_bracelet\0", 16);
    memcpy(inoltro->dest, cose.c_str(),cose.length()+1);

    memcpy(inoltro->original_sender, "0\0",2);
    memcpy(inoltro->source, "0\0",2);

    memcpy(inoltro->messageCount, String(msgCount).c_str(), String(msgCount).length()+1);  
    msgCount++;
    pref.putInt("msgCount",msgCount);

    memcpy(inoltro->touched, "0\0", 2);
    memcpy(inoltro->text, text, String(text).length()+1);

    

    messages_send.push_back(inoltro);
    

    char buffer[sizeof(struct_message)+1];
    memcpy(buffer, inoltro, sizeof(struct_message));
    buffer[sizeof(struct_message)]='\0';
    
    LoRa.beginPacket();
    for(int i=0;i<sizeof(struct_message)+1;i++){
      LoRa.write(buffer[i]);
    }
    LoRa.endPacket();
    LoRa.receive();
    /* Send a simple response */
    const char resp[] = "URI POST Response";
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);
    return ESP_OK;
}

/* URI handler structure for GET /uri */
httpd_uri_t uri_post = {
    .uri      = "/receive_data",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

httpd_handle_t Server;

httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();

    /* Empty handle to esp_http_server */
    httpd_handle_t server = NULL;

    /* Start the httpd server */
    if (httpd_start(&server, &config) == ESP_OK) {
        /* Register URI handlers */
        httpd_register_uri_handler(server, &uri_post);
    }
    /* If server failed to start, handle will be NULL */
    return server;
}

void stop_webserver(httpd_handle_t server)
{
    /* Stop the httpd server */
    httpd_stop(server);
}


void setup() {
  // Init Serial Monitor
  Serial.begin(115200);
  pref.begin("my_id", false); 

  //pref.putBool("set_esp",true); //ricordiamoci di metterlo a false nella versione finale
  
  ho_settato_un_altro_esp=pref.getBool("set_esp");
  msgCount=pref.getInt("msgCount");
  id=pref.getInt("id");

  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500); 
  }
  LoRa.setSyncWord(0xffff);
  LoRa.onReceive(ricezioneLora);
  LoRa.receive();
  Serial.println("LoRa Initializing OK!");

  BLEDevice::init("Ancora");
  pClient = BLEDevice::createClient();

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }

  Server=start_webserver();
  Serial.println("created server");

  delay(3000);

  if(ho_settato_un_altro_esp){
    Serial.println("ho gia settato un altro esp");
    searchAncore_task.disable();
    LoRa.receive();
  }else{
    Serial.println("non ho ancora settato un altro esp");
    searchAncore_task.enable();
  }

  while (!send_ip())
  {
    delay(1000);
    Serial.println("retrying to send IP");
  }
  
  Serial.println("Messaggio ricevuto");
  SendToServer = false;
  LoRa.disableCrc();
}

void loop(){
  ts.execute();
}