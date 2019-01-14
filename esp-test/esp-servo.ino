#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
 

// Replace with your network credentials
const char* ssid = "";
const char* password = "";
 

ESP8266WebServer server(80);   //instantiate server at port 80 (http port)
Servo myservo;  // servo object
 

String page = "";
int pos = 0;
int LEDPin = LED_BUILTIN;


void servo_cc(void) {
  for (pos = 0; pos <= 180; pos += 1) { // goes from 0 degrees to 180 degrees
    // in steps of 1 degree
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(5);                        // waits 15ms for the servo to reach the position
  }
}


void servo_ccw(void) {
  for (pos = 180; pos >= 0; pos -= 1) { // goes from 180 degrees to 0 degrees
    myservo.write(pos);              // tell servo to go to position in variable 'pos'
    delay(15);  
  }
}


void setup(void){
  //the HTML of the web page
  page = "<h1>Whasup Mane!!!!</h1><p><a href=\"LEDOn\"><button>ON</button></a>&nbsp;<a href=\"LEDOff\"><button>OFF</button></a></p>";
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
   
  server.on("/", [](){
    server.send(200, "text/html", page);
  });
  server.on("/LEDOn", [](){
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);
    servo_cc();
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
