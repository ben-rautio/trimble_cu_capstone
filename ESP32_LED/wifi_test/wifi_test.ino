#include <AsyncTCP.h>
/*
  Rui Santos
  Complete project details at https://RandomNerdTutorials.com/esp32-client-server-wi-fi/
  
  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files.
  
  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.
*/

// Import required libraries
#include "WiFi.h"
#include "ESPAsyncWebServer.h"

// Set your access point network credentials
const char* ssid = "ESP32-Access-Point";
const char* password = "123456789";

// Create AsyncWebServer object on port 80
AsyncWebServer server(80);

void setup(){
  // Serial port for debugging purposes
  Serial.begin(115200);
  Serial.println();
  
  // Setting the ESP as an access point
  Serial.print("Setting AP (Access Point)…");
  // Remove the password parameter, if you want the AP (Access Point) to be open
  WiFi.softAP(ssid); //, password);

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);
  
  server.on("/bork", HTTP_GET, [](AsyncWebServerRequest *request) {
    Serial.println("Bork request");
    request->send_P(200, "text/plain", "Hi bork!");
  });
  
  server.on("/change", HTTP_GET, [](AsyncWebServerRequest *request) {
    String out = "Sam is a big stinky butt!\n\n";
    int params = request->params();
    for(int i=0;i<params;i++){
      AsyncWebParameter *p = request->getParam(i);
      Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
      out = out + p->name().c_str() + ": " + p->value().c_str() + "\n";
   }
   request->send_P(200, "text/plain", out.c_str());
   });

   server.on("/POST", HTTP_GET, [](AsyncWebServerRequest *request) {
      String out = "POST routine page\n\n";
      int params = request->params();
      for(int i=0; i < params; i++){
        AsyncWebParameter *p = request->getParam(i);
        Serial.printf("GET[%s]: %s\n", p->name().c_str(), p->value().c_str());
        out = out + p->name().c_str() + ": " + p->value().c_str() + "\n";
      }
      request->send_P(200, "text/plain", out.c_str());
   });

  // Start server
  server.begin();
}


 
void loop(){
  
}
