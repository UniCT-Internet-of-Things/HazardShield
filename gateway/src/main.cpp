#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoBLE.h>
#include <HTTPClient.h>

bool is_broadcast(uint8_t* mac){
  return (mac[0]==0xff&&
          mac[1]==0xff&&
          mac[2]==0xff&&
          mac[3]==0xff&&
          mac[4]==0xff&&
          mac[5]==0xff
          );
}

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

//SSID of your network
char ssid[] = "Vodafone-66433532";
//password of your WPA Network
char pass[] = "e53mcuz2phlxzpc";


uint8_t remote_wifi_next[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

uint8_t broadcastAddress[] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};

typedef struct struct_message {
    char type[20];
    char text[100]; 
    char source[30];
    char dest[30]; 
} struct_message;

// Create a struct_message to hold incoming data
struct_message incomingReadings;


esp_now_peer_info_t peerInfo;
esp_now_peer_info_t peerInfo2;
esp_now_peer_info_t peerInfo3;

WiFiClient client;

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
    BLE.stopAdvertise();
  }

  else if(String(incomingReadings.dest).equals(WiFi.macAddress())){
    Serial.println("arrivato a destinazione");
    Serial.println(incomingReadings.text);
  }
  else {
    Serial.println("errore"); 
    Serial.println(incomingReadings.text);
  }

}


void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println();
  WiFi.begin(ssid, pass);

  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");
    while (1);
  }

  BLE.setLocalName("Ancora");

  BLE.advertise();
  

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

  
}

void loop(){
  
  if(false){
    HTTPClient http;
    String url="http://192.168.1.10:5000/get_data";
    String jsondata="{'datadatadatadatadatadatadatadatadatadatadatadatadatadatadata':'datadatadatadatadatadatadatadatadatadatadatadatadata'}";

    http.begin(url); 
    http.addHeader("Content-Type", "Content-Type: application/json"); 

    int httpResponseCode = http.POST(jsondata); //Send the actual POST request

    if(httpResponseCode>0){
      String response = http.getString();  //Get the response to the request
      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);           //Print request answer
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);

      http.end();

    }
  }
}

  