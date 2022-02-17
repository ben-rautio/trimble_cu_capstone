#include <AsyncTCP.h>
#include <SerialTransfer.h>
#include "WiFi.h"
#include "HTTPClient.h"
#include "ESPAsyncWebServer.h"
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-client-server-wi-fi/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// Set your access point network credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Different servers
//const char* serverStatus = "http://192.168.4.1/status";
const char* serverChange0 = "http://192.168.4.1/change?dc=0";
const char* serverChange10 = "http://192.168.4.1/change?dc=10";
const char* serverChange20 = "http://192.168.4.1/change?dc=20";
const char* serverChange30 = "http://192.168.4.1/change?dc=30";
const char* serverChange40 = "http://192.168.4.1/change?dc=40";
const char* serverChange50 = "http://192.168.4.1/change?dc=50";
const char* serverChange60 = "http://192.168.4.1/change?dc=60";
const char* serverChange70 = "http://192.168.4.1/change?dc=70";
const char* serverChange80 = "http://192.168.4.1/change?dc=80";
const char* serverChange90 = "http://192.168.4.1/change?dc=90";
const char* serverChange100 = "http://192.168.4.1/change?dc=100";
//const char* serverChange110 = "http://192.168.4.1/change?dc=110";
const char* serverBork = "http://192.168.4.1/bork";
const char* serverBen = "http://192.169.4.1/";
// Create AsyncWebServer object on port 80
AsyncWebServer server(80);
SerialTransfer myTransfer;
bool updated;
struct STRUCT {
  uint32_t z;
} typedef testStruct;

testStruct myTestStruct;
void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial1.begin(115200);
  Serial.println();
  Serial.println("Connecting...");
  WiFi.begin(ssid);
  updated = true;
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
    Serial.print("REEEE");
  }
  Serial.print("Connected to WiFi network w/ IP: ");
  Serial.println("localIP access");
  Serial.println(WiFi.localIP());
  myTransfer.begin(Serial1);
  myTestStruct.z = 0;
}

void loop(){
    if(myTransfer.available())
    {
      Serial.println("Available");
      // use this variable to keep track of how many
      // bytes we've processed from the receive buffer
      uint16_t recSize = 0;
      recSize = myTransfer.rxObj(myTestStruct.z, recSize);
      Serial.print(myTestStruct.z);
      Serial.print(" | ");
      updated = true;
    }
    //Serial.println("switch statements");
    if(updated){
      switch(myTestStruct.z){
        case 0:
          //Serial.print("Case 0");
          getDuty(serverChange0);
          updated = false;
          break;
        case 10:
          getDuty(serverChange10);
          updated = false;
          break;
        case 20:
          getDuty(serverChange20);
          updated = false;
          break;
        case 30:
          getDuty(serverChange30);
          updated = false;
          break;
        case 40:
          getDuty(serverChange40);
          updated = false;
          break;
        case 50:
          getDuty(serverChange50);
          updated = false;
          break;
        case 60:
          getDuty(serverChange60);
          updated = false;
          break;
        case 70:
          getDuty(serverChange70);
          updated = false;
          break;
        case 80:
          getDuty(serverChange80);
          updated = false;
          break;
        case 90:
          getDuty(serverChange90);
          updated = false;
          break;
        case 100:
          getDuty(serverChange100);
          updated = false;
          break;
        default:
          updated = false;
          break;
      }  
    }
}

String getDuty(const char* serverName){
  HTTPClient http;
  http.begin(serverName);
  int httpResp = http.GET();
  String payload = "Not right";
  if(httpResp > 0){
    //Serial.print("HTTP Response: ");
    //Serial.println(httpResp);
    payload = http.getString();
  }
  else{
    Serial.print("Error code: ");
    Serial.println(httpResp);
  }
  http.end();
  return payload;
}
