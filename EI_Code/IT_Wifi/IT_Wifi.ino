#include <AsyncTCP.h>
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-client-server-wi-fi/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// 


// Import required libraries
#include "WiFi.h"
#include "HTTPClient.h"
#include "ESPAsyncWebServer.h"

// Set your access point network credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Different servers
const char* serverStatus = "192.168.4.1/status";
const char* serverChange = "192.168.4.1/change";
const char* serverBork = "192.168.4.1/bork";
const char* serverBen = "192.169.4.1/";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();
  Serial.println("Connecting...");
  WiFi.begin(ssid);
  while(WiFi.status() != WL_CONNECTED){
    delay(500);
    Serial.print("REEEE");
  }
  Serial.print("Connected to WiFi network w/ IP: ");
  Serial.println(WiFi.localIP());
}

String ben;
String Bork;

void loop(){
   delay(5000);
   ben = getDuty(serverBen);
   Bork = getDuty(serverBork);
   Serial.println(ben + Bork);
}

String getDuty(const char* serverName){
  WiFiClient client;
  HTTPClient http;
  http.begin(client, serverName);
  int httpResp = http.GET();
  String payload = "Not right";
  if(httpResp > 0){
    Serial.print("HTTP Response: ");
    Serial.println(httpResp);
    payload = http.getString();
  }
  else{
    Serial.print("Error code: ");
    Serial.println(httpResp);
  }
  http.end();
  return payload;
}

//void getBen(){
//  WiFiClient client;
//  HTTPClient http;
//  http.begin(client, serverBen);
//  Serial.print("Ben Request: ");
////  Serial.println(http.getString());
//  http.end();
//  return;
//}
