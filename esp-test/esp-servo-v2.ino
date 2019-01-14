#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
 

// Replace with your network credentials
const char* ssid = " ";
const char* password = " ";
 

ESP8266WebServer server(80);   //server at port 80 (http port)
Servo myservo;  // servo object
 

String page = "";
// style not working, update html with css
String style = "";
int pos = 0;
int LEDPin = LED_BUILTIN;


void servo_cc(void) {
  for (pos = 0; pos <= 180; pos += 1) {
    // in steps of 1 degree
    myservo.write(pos);              
    delay(5);                        
  }
}


void servo_90(void) {
  for (pos = 0; pos <= 10; pos += 1) { 
    // in steps of 1 degree
    myservo.write(pos);              
    delay(10);                       
  }
}


void servo_ccw(void) {
  for (pos = 180; pos >= 0; pos -= 1) { 
    myservo.write(pos);              
    delay(5);  
  }
}


void setup(void){
  //the HTML of the web page
  style = "button{size:5em;} h1 {font-size:3em;}";
  page = "<h1>Whasup Mane!!!!</h1><h2>Servo Control!</h2><p><a href=\"LEDOn\"><button>On</button></a>&nbsp;<a href=\"LEDOff\"><button>OFF</button></a>&nbsp;<a href=\"LEDOn90\"><button>Tiny Move</button></a></p>";
  //make the LED pin output and initially turned off
  pinMode(LEDPin, OUTPUT);
  myservo.attach(4);  // pin 4 on nodemcu esp8266
  digitalWrite(LEDPin, LOW);
   
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

  // add working CSS at some point
  server.on("/", [](){
    server.send(200, "text/html", page);
  });
  server.on("/LEDOn", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);
    servo_cc();
    delay(1000);
  });
  server.on("/LEDOn90", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);
    servo_90();
    delay(1000);
  });
  server.on("/LEDOff", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, HIGH);
    servo_ccw();
    delay(1000); 
  });
  server.begin();
  Serial.println("Web server started!");
}

 
void loop(void){
  // eskedit
  server.handleClient();
}
