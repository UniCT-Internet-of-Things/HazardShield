#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoBLE.h>
#include <HTTPClient.h>
#include <LoRa.h>



//SSID of your network
char ssid[] = "Redmi Note 12 5G";
//password of your WPA Network
char pass[] = "ciccio2305";

#define ss 18
#define rst 23
#define dio0 26


bool SendToServer = false;
String toSend = "";

void onReceive(int packetSize) {
  Serial.println("Received a packet");
  if (packetSize == 0) return;          // if there's no packet, return
  SendToServer = true;   
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
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa Initializing OK!");

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println();
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  
}


void loop(){
  if(SendToServer){
    String incoming = "";                 // payload of packet

    if (LoRa.available()) {            // can't use readString() in callback, so
      incoming += LoRa.readString();
      Serial.println("Message: " + incoming);
      toSend = incoming;     
    } 
    
    HTTPClient http;
    String url="http://192.168.231.130:5000/get_data";
    http.begin(url); 
    http.addHeader("Content-Type", "Content-Type: application/json"); 

    int httpResponseCode = http.POST(toSend); //Send the actual POST request

    if(httpResponseCode>0){
      String response = http.getString();  //Get the response to the request
      Serial.println(httpResponseCode);   //Print return code
      Serial.println(response);        //Print request answer
      http.end();          
    } else {
      Serial.print("Error on sending POST: ");
      Serial.println(httpResponseCode);

      http.end();

    }
  
    Serial.println("Messaggio ricevuto");
    SendToServer = false;
  }
  delay(100);
}
