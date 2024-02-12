#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <esp_now.h>
#include <ArduinoBLE.h>
#include <HTTPClient.h>
#include <LoRa.h>



//SSID of your network
char ssid[] = "Readmi Note 12 5G";
//password of your WPA Network
char pass[] = "ciccio2305";

#define ss 18
#define rst 23
#define dio0 26


void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return

  String incoming = "";                 // payload of packet

  while (LoRa.available()) {            // can't use readString() in callback, so
    incoming += (char)LoRa.read();      // add bytes one by one
  }

  Serial.println("Message: " + incoming);
  HTTPClient http;
    String url="http://192.168.60.129:5000/get_data";
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
  
  Serial.println("Messaggio ricevuto");
  
  
}


void setup() {
  // Init Serial Monitor
  Serial.begin(115200);

  // Set device as a Wi-Fi Station
  WiFi.mode(WIFI_STA);
  Serial.println();
  WiFi.begin(ssid, pass);


  LoRa.setPins(ss, rst, dio0);
  while (!LoRa.begin(866E6)) {
    Serial.println(".");
    delay(500); 
  }

  LoRa.setSyncWord(0xffff);
  LoRa.onReceive(onReceive);
  LoRa.receive();
  Serial.println("LoRa Initializing OK!");

  
}


void loop(){
  
}
