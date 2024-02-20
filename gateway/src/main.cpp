#include <Arduino.h>
#include <esp_now.h>
#include <WiFi.h>
#include <ArduinoBLE.h>
#include <HTTPClient.h>
#include <LoRa.h>
#include "esp_http_server.h"


//SSID of your network
char ssid[] = "Redmi Note 12 5G";
//password of your WPA Network
char pass[] = "ciccio2305";

#define ss 18
#define rst 23
#define dio0 26

bool SendToServer = false;
String toSend = "";

String base_url="http://192.168.231.130:5000/";

void onReceive(int packetSize) {
  if (packetSize == 0) return;          // if there's no packet, return
  SendToServer = true;   
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

  Server= start_webserver();
  Serial.println("created server");

  delay(3000);

  HTTPClient http;
  String url=base_url+"register_ip";
  http.begin(url);
  http.addHeader("Content-Type", "Content-Type: application/json"); 

  int httpResponseCode = http.POST(String("{\"my_ip\":\"")+String(WiFi.localIP())+String("\"}"));

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

void loop(){
  if(SendToServer){
    String incoming = ""; 

    if (LoRa.available()) {            // can't use readString() in callback, so
      incoming += LoRa.readString();
      Serial.println("Message: " + incoming);
      toSend = incoming;     
    } 
    
    HTTPClient http;
    String url=base_url+"get_data";
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