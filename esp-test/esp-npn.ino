/*
 * Nodemcu esp8266 program to control a npn transistor as a switch
 * This example uses a 9v battery and a DC motor as it's collector and emitter
 * Author: John Glatts
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
 

// Replace with your network credentials
const char* ssid = "";
const char* password = "";
 
  
ESP8266WebServer server(80);   //server at port 80 (http port)
 

String page = "";
// style not working, update html with css
String style = "";
const int pos = 0;
const int LEDPin = LED_BUILTIN;
const int switch_pin = 4;  // base pin for npn transistor, when HIGH it will close the DC motor connection

void setup(void){
  //the HTML of the web page
  style = "button{size:5em;} h1 {font-size:3em;}";
  // update with npn info
  page = "<h1>Whasup Mane!!!!</h1><h2>Servo Control!</h2><p><a href=\"LEDOn\"><button>On</button></a>&nbsp;<a href=\"LEDOff\"><button>OFF</button></a>&nbsp;<a href=\"LEDOn90\"><button>Tiny Move</button></a></p>";
  
  //setup pins as outpuuts
  pinMode(LEDPin, OUTPUT);    
  pinMode(switch_pin, OUTPUT); 
  digitalWrite(LEDPin, HIGH);
  // start with npn open
  // attach resistor from switch_pin to base of npn
  // npn seems to have trouble with the direct digitalWrite()
  digitalWrite(switch_pin, LOW);  
  delay(1000);
  
  Serial.begin(115200);
  WiFi.begin(ssid, password); //begin WiFi connection
  Serial.println("");
 
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print("Not Connected");
    Serial.print(" ");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // add working CSS at some point
  server.on("/", [](){
    server.send(200, "text/html", page);
  });
  server.on("/LEDOn", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);
    digitalWrite(switch_pin, HIGH); // close the DC motor ground wire, turning motor on
    delay(1000);
  });
  // change url to something representive
  server.on("/LEDOn90", [](){
    // move motor in small increments
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);  // keep LED on
    digitalWrite(switch_pin, HIGH); 
    delay(100);
    digitalWrite(switch_pin, LOW); 
    delay(1000);
    digitalWrite(switch_pin, HIGH); // leave motor on
    delay(1000);
  });
  server.on("/LEDOff", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, HIGH);
    digitalWrite(switch_pin, LOW);  // open the DC motor ground wire, turning motor off
    delay(1000); 
  });
  server.begin();
  Serial.println("Web server started!");
}

 
void loop(void){
  // eskedit
  server.handleClient();
}
