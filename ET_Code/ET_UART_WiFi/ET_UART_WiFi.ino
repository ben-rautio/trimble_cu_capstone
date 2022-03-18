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

#define RXD2 9
#define TXD2 10
#define TX_WIFI 34
#define REQ_UART 33
#define HEARTBEAT 32

#define WIFI_CONNECTED 25
#define HB_LED 26
#define LED4 27


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

void blink(int pin)
{
  delay(100);
  digitalWrite(pin, HIGH);
  delay(100);
  digitalWrite(pin, LOW);
}

unsigned long currentMillis = millis();
unsigned long previousMillis = 0;
int ledState = LOW;             
const long interval = 1000;          

void HB()
{
  if (currentMillis - previousMillis >= interval) {
    // save the last time you blinked the LED
    previousMillis = currentMillis;

    // if the LED is off turn it on and vice-versa:
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
  }
}
unsigned long wifiCurrent = millis();
unsigned long wifiPrevious = 0;
int wifiStateLED = LOW;             
const long wifiInterval = 5000;

void wifiStatus()
{
  if (wifiCurrent - wifiPrevious >= wifiInterval) {
    // save the last time you blinked the LED
    wifiStateLED = LOW;
  }
  else {
    wifiStateLED = HIGH;
  }
}

void setup(){
  pinMode(TX_WIFI, OUTPUT);
  pinMode(HEARTBEAT, OUTPUT);
  pinMode(REQ_UART, OUTPUT);
  pinMode(WIFI_CONNECTED, OUTPUT);
  pinMode(HB_LED, OUTPUT);
  
  digitalWrite(WIFI_CONNECTED, LOW);
  
  Serial.begin(115200);
  Serial2.begin(115200);
  Serial.println();
  Serial.println("Connecting...");
  WiFi.begin(ssid);
  updated = true;
  while(WiFi.status() != WL_CONNECTED){
    delay(1000);
     Serial.print("REEEE");
  }
  digitalWrite(WIFI_CONNECTED, HIGH);
  Serial.print("Connected to WiFi network w/ IP: ");
  Serial.println("localIP access");
  Serial.println(WiFi.localIP());
  myTransfer.begin(Serial2);
  myTestStruct.z = 0;
}

void loop(){

    //Heartbeat LED
    currentMillis = millis();
    wifiCurrent = millis();
    
    HB();
    digitalWrite(HB_LED, ledState);

    wifiStatus();

    //check uart
    if(myTransfer.available())
    {
      //blink(REQ_UART);
      digitalWrite(REQ_UART, HIGH);
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
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 10:
          getDuty(serverChange10);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 20:
          getDuty(serverChange20);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 30:
          getDuty(serverChange30);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 40:
          getDuty(serverChange40);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 50:
          getDuty(serverChange50);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 60:
          getDuty(serverChange60);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 70:
          getDuty(serverChange70);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 80:
          getDuty(serverChange80);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 90:
          getDuty(serverChange90);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        case 100:
          getDuty(serverChange100);
          updated = false;
          digitalWrite(REQ_UART, LOW);
          wifiPrevious = wifiCurrent;
          break;
        default:
          updated = false;
          break;
      }  
    }
    blink(HEARTBEAT);
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