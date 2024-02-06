#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoBLE.h>
#include <Wire.h>
#include <TaskScheduler.h>

bool is_broadcast(uint8_t* mac){
  return (mac[0]==0xff&&
          mac[1]==0xff&&
          mac[2]==0xff&&
          mac[3]==0xff&&
          mac[4]==0xff&&
          mac[5]==0xff
          );
}

bool i_m_gateway=false;

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
    String type;
    String text; 
    String dest; 
} struct_message;

String temp;

// Create a struct_message to hold incoming data
struct_message incomingReadings;

// Create a struct_message to send data
struct_message message;

esp_now_peer_info_t peerInfo;
esp_now_peer_info_t peerInfo2;
esp_now_peer_info_t peerInfo3;

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
  Serial.print("dest:  ");
  Serial.println(incomingReadings.dest);
  Serial.print("text:  ");
  Serial.println(incomingReadings.text);

  Serial.println("");

  if(incomingReadings.type.equals("new node")){
    macStrToByteArray(incomingReadings.text,remote_wifi_next);
    Serial.println("nuovo nodo aggiunto");

    memcpy(peerInfo3.peer_addr, remote_wifi_next, 6);
    peerInfo3.channel = 1;  
    peerInfo3.encrypt = false;

    if(esp_now_add_peer(&peerInfo3)!= ESP_OK){
      Serial.println("Failed to add discovered peer");
      return;
    }
    BLE.stopAdvertise();
  }else if(incomingReadings.dest.equals(WiFi.macAddress())){
    Serial.println("arrivato a destinazione");
    Serial.println(incomingReadings.text);
  }
  else{
    Serial.println("inoltro al prossimo");

    char remote_wifi_prec_str[18];
    snprintf(remote_wifi_prec_str, sizeof(remote_wifi_prec_str), "%02x:%02x:%02x:%02x:%02x:%02x",
         remote_wifi_prec[0], remote_wifi_prec[1], remote_wifi_prec[2],
          remote_wifi_prec[3], remote_wifi_prec[4], remote_wifi_prec[5]);
    String remote_wifi_prec_String(remote_wifi_prec_str);

    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
         mac[0], mac[1], mac[2],
          mac[3], mac[4], mac[5]);
    String mac_String(mac_str);

    if(mac_String.equals(remote_wifi_prec_str)){
      Serial.println("inoltro a next");
      if(!is_broadcast(remote_wifi_next)){
        esp_err_t result = esp_now_send(remote_wifi_next, (uint8_t *) &incomingReadings, sizeof(incomingReadings));
      }
      else{
        Serial.println("sarebbe broadcast beddu");
      }
    }else{
      Serial.println("inoltro a prec");
      if(!is_broadcast(remote_wifi_prec)){
        esp_err_t result = esp_now_send(remote_wifi_prec, (uint8_t *) &incomingReadings, sizeof(incomingReadings));
      }else{
        Serial.println("sarebbe broadcast beddu");
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

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }
  BLE.setLocalName("Ancora");

  BLE.advertise();
  if(!i_m_gateway)
    BLE.scanForName("Ancora");
  
  while(!i_m_gateway){
    BLEDevice peripheral = BLE.available();
    if(peripheral){
      Serial.println("Address: " + peripheral.address());
      BLE.stopScan();

      macStrToByteArray(peripheral.address(),remote_ble);
      macStrToByteArray(peripheral.address(),remote_wifi_prec);
      remote_wifi_prec[5]=remote_wifi_prec[5]-2;
      for(int i = 0; i < 6; i++){
        Serial.print(remote_ble[i],HEX);
        Serial.print(":");
      }
      Serial.println();
      
      peripheral.disconnect();
      
      BLE.stopScan();
      break;
    }

    Serial.println("searching");
    delay(500);
  }


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

  if(!i_m_gateway){

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
    
    message.type=String("new node");
    message.text=WiFi.macAddress();
    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
          remote_wifi_prec[0], remote_wifi_prec[1], remote_wifi_prec[2],
          remote_wifi_prec[3], remote_wifi_prec[4], remote_wifi_prec[5]);
    message.dest=String(mac_str);


    if(!is_broadcast(remote_wifi_prec)){
      esp_err_t result = esp_now_send(remote_wifi_prec, (uint8_t *) &message, sizeof(message));
      if (result == ESP_OK) {
      Serial.println("Sent with success");
      Serial.println();
      }
      else {
        Serial.println("Error sending the data");
        Serial.println();
      }
    }
    else{
      Serial.println("sarebbe Broadcast beddu");
    }

    
  }
}

void loop() {

  if(false){
    delay(5000);
    message.type=String("mess");
    message.text=String("ciao");

    char mac_str[18];
    snprintf(mac_str, sizeof(mac_str), "%02x:%02x:%02x:%02x:%02x:%02x",
          broadcastAddress[0], broadcastAddress[1], broadcastAddress[2],
          broadcastAddress[3], broadcastAddress[4], broadcastAddress[5]);

    message.dest=String(mac_str);
    Serial.print("destinatario nuovo messagio:   ");
    Serial.println(message.dest);

    if(!is_broadcast(remote_wifi_prec)){
      esp_err_t result = esp_now_send(remote_wifi_prec, (uint8_t *) &message, sizeof(message));
      if (result == ESP_OK) {
      Serial.println("Sent with success");
      Serial.println();
      }
      else {
        Serial.println("Error sending the data");
        Serial.println();
      }
    }
    else{
      Serial.println("sarebbe Broadcast beddu");
    }
    
  }
}
