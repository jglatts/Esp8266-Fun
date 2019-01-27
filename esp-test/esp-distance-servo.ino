#include <Servo.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
 

// Replace with your network credentials
const char* ssid = "";
const char* password = "";
 

ESP8266WebServer server(80);   //server at port 80 (http port)
Servo espservo;
 

// string for html/css  
String page = "";


// declare vairables
int duration = 0;
int distance = 0;
double data = 0; 
int pos = 0;


// I/O pin define
const int LEDPin = LED_BUILTIN;
const int TrigPin = 5;
const int EchoPin = 0;
const int PWMpin = 4;


void servo_move(void) {
  for (pos = 0; pos <= 180; pos += 1) {
    // in steps of 1 degree
    espservo.write(pos);              
    delay(5);                        
  }
}


void servo_home(void) {
  for (pos = 180; pos >= 0; pos -= 1) {
    // in steps of 1 degree
    espservo.write(pos);              
    delay(5);                        
  }
}


void setup(void){
  //the HTML of the web page
  // set up pins
  pinMode(TrigPin, OUTPUT);
  pinMode(EchoPin, INPUT);
  espservo.attach(PWMpin);
  //setup LED
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, HIGH);
  delay(1000);
   
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
    page = "<h1>Whasup Mane!!!!</h1><h2>Distance Data</h2><p><a href=\"LEDOn\"><button>LED On</button></a>&nbsp;<a href=\"LEDOff\"><button>Get Distance</button></a>";
    server.send(200, "text/html", page);
  });
  server.on("/LEDOn", [](){
    page = "<h1>Whasup Mane!!!!</h1><h2>Distance Data</h2><p><a href=\"LEDOn\"><button>LED On</button></a>&nbsp;<a href=\"LEDOff\"><button>Get Distance</button></a>";
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);
    servo_move();
    delay(1000);
  });
  server.on("/LEDOff", [](){
    page = "<h1>Whasup Mane!!!!</h1><h2>Distance Data</h2><p><a href=\"LEDOn\"><button>LED On</button></a>&nbsp;<a href=\"LEDOff\"><button>Get Distance</button></a><h2><b>Distance: "+String(distance)+"</b></h2>";
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, HIGH);
    servo_home();
    delay(1000); 
  });
  server.begin();
  Serial.println("Web server started!");
}

 
void loop(void){
  // start the webserver
  server.handleClient();

  // activate hc-sr04 sensor
  digitalWrite(TrigPin, LOW); // clear the trig_pin
  delayMicroseconds(2);
  digitalWrite(TrigPin, HIGH); // Set trig_pin on HIGH for 10 micro seconds
  delayMicroseconds(10);
  digitalWrite(TrigPin, LOW);

  // update distance 
  duration = pulseIn(EchoPin, HIGH); // Reads the echoPin, returns the sound wave travel time in microseconds
  distance= duration * 0.034/2;
}
