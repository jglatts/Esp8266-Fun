/*
 * Author: John Glatts
 * JQC-3FF-S-Z Relay and DC Motor Program
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include "index.h"   

// Replace with your network credentials
const char* ssid = "";
const char* password = "";
 
ESP8266WebServer server(80);   //server at port 80 (http port)

// string for html
String main_page = page;
String motor_on_page = on_page;
String motor_off_page = off_page;

// declare vairables
double data = 0; 

// I/O pin define
const int LED = LED_BUILTIN;
const int SIGpin = 5;

void motor_move(void) {
  // turn LED and relay on
  digitalWrite(LED, LOW);  
  digitalWrite(SIGpin, HIGH); 
}


void motor_stop(void) {
  // turn LED and relay off
  digitalWrite(LED, HIGH);  
  digitalWrite(SIGpin, LOW);
}


void setup(void){
  //setup pins
  pinMode(LED, OUTPUT);
  pinMode(SIGpin, OUTPUT);
  digitalWrite(LED, HIGH);
  digitalWrite(SIGpin, LOW);
  delay(1000);
  Serial.begin(115200);
  WiFi.begin(ssid, password); //begin WiFi connection
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", [](){
    server.send(200, "text/html", main_page);
  });
  server.on("/LEDOn", [](){
    server.send(200, "text/html", motor_on_page);
    motor_move();
    //delay(1000);
  });
  server.on("/LEDOff", [](){
    server.send(200, "text/html", motor_off_page);
    motor_stop();
    //delay(1000); 
  });
  server.begin();
  Serial.println("Web server started!");
}

 
void loop(void){
  // start the webserver
  server.handleClient();
}
