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
const char* serverStatus = "http://192.168.4.1/status";
const char* serverChange10 = "http://192.168.4.1/change?dc=10";
const char* serverChange50 = "http://192.168.4.1/change?dc=50";
const char* serverChange100 = "http://192.168.4.1/change?dc=100";
const char* serverBork = "http://192.168.4.1/bork";
const char* serverBen = "http://192.169.4.1/";
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

String Bork;
String dutyCycle;

void loop(){
//   delay(5000);
//   dutyCycle = getDuty(serverChange);
//   Bork = getDuty(serverBork);
//   Serial.println(dutyCycle);
//   Serial.println(Bork);
  dutyCycle = getDuty(serverChange10).toInt();
  delay(5000);
  dutyCycle = getDuty(serverChange50).toInt();
  delay(5000);
  dutyCycle = getDuty(serverChange100).toInt();
  delay(5000);

}

String getDuty(const char* serverName){
  HTTPClient http;
  http.begin(serverName);
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

//String getStatus(
//void getBen(){
//  WiFiClient client;
//  HTTPClient http;
//  http.begin(client, serverBen);
//  Serial.print("Ben Request: ");
////  Serial.println(http.getString());
//  http.end();
//  return;
//}
