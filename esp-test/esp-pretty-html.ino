/*
 * Author: John Glatts
 * Base code from: www.circuits4you.com
 */
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

//SSID and Password of your WiFi router
const char* ssid = "";
const char* password = "";

ESP8266WebServer server(80); //Server on port 80

String LEDstate = "";
const int LED = LED_BUILTIN;

const char MAIN_page[] PROGMEM = R"=====(
<HTML>
  <HEAD>
      <TITLE>AYYYY</TITLE>
  </HEAD>
  <style>
    #test:hover {
      color: brown;
    }
  </style>
<BODY>
  <CENTER>
      <B>Hello World.... </B>
      <br>
      <p style="font-size:3em" id="test">Tits!</p>
      <br>
      <!-- Fix link and add to a seperate .h file -->
      <button><a href="">LED On</a></button>
      <p style="font-size:3em" id="test"> String(LEDstate) </p>
  </CENTER> 
</BODY>
</HTML>
)=====";


//===============================================================
// This routine is executed when you open its IP in browser
//===============================================================
void handleRoot() {
 String s = MAIN_page; //Read HTML contents
 server.send(200, "text/html", s); //Send web page
}


//==============================================================
//                  SETUP
//==============================================================
void setup(void){
  Serial.begin(115200);
  pinMode(LED, OUTPUT);
  digitalWrite(LED, HIGH);
  
  WiFi.begin(ssid, password);     //Connect to your WiFi router
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  //If connection successful show IP address in serial monitor
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());  //IP address assigned to your ESP
 
  server.on("/", handleRoot);      //Which routine to handle at root location

  // this link is not working, the fix is on it's way
  // going to try and replicate handleRoot, i.e just call the function inside server.on()
  server.on("/LEDOn", [](){
    LEDstate = "Led On";
    String s = MAIN_page; //Read HTML contents
    server.send(200, "text/html", s);
    digitalWrite(LED, LOW);
    delay(1000);
  });

  server.begin();                  //Start server
  Serial.println("HTTP server started");
}


//==============================================================
//                     LOOP
//==============================================================
void loop(void){
  server.handleClient();          //Handle client requests
}
