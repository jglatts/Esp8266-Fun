#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
 
// Replace with your network credentials
const char* ssid = " ";
const char* password = " ";
 
ESP8266WebServer server(80);   //instantiate server at port 80 (http port)
 
String page = "";
String test_data = "";
int idx = 0;  //test value that will serve as the data
const int LEDPin = LED_BUILTIN;
double data; 

void setup(void){ 
  pinMode(LEDPin, OUTPUT);
  digitalWrite(LEDPin, HIGH);
  test_data = "Led Is Off";
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
    page = "<h1>Whasup Mane!!!!</h1><h2>LED Data</h2><p><a href=\"LEDOn\"><button>LED On</button></a>&nbsp;<a href=\"LEDOff\"><button>LED Off</button></a><h3>"+test_data+"</h3></p>";
    server.send(200, "text/html", page);
  });
  server.on("/LEDOn", [](){
    test_data = "Led Is On";  
    page = "<h1>Whasup Mane!!!!</h1><h2>LED Data</h2><p><a href=\"LEDOn\"><button>LED On</button></a>&nbsp;<a href=\"LEDOff\"><button>LED Off</button></a><h3>"+test_data+"</h3></p>";
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, LOW);
    delay(1000);
  });
  server.on("/LEDOff", [](){
    test_data = "Led Is Off";  
    page = "<h1>Whasup Mane!!!!</h1><h2>LED Data</h2><p><a href=\"LEDOn\"><button>LED On</button></a>&nbsp;<a href=\"LEDOff\"><button>LED Off</button></a><h3>"+test_data+"</h3></p>";
    server.send(200, "text/html", page);
    digitalWrite(LEDPin, HIGH);
    delay(1000);
  });
  
  server.begin();
  Serial.println("Web server started!");
}
 
void loop(void){
  delay(1000);
  server.handleClient();
}
