/* 
 *  For more information see http://42bots.com
 *  Configure the ESP8266 unit as a Wi-Fi access point
 */

#include <ESP8266WiFi.h>

/* Your WiFi Soft Access Point settings */
const char* ssid = "";          //this will be the network name
const char* password = "";  //this will be the network password

void setup() {
  delay(1000);
  Serial.begin(9600);
  Serial.println();
  Serial.println();
  Serial.print("Configuring WiFi access point...");
  
  /* You can remove the password parameter if you want the AP to be open. */
  boolean result = WiFi.softAP(ssid, password);
  if(result==true) {
    IPAddress myIP = WiFi.softAPIP();
  
    Serial.println("done!");
    Serial.println("");
    Serial.print("WiFi network name: ");
    Serial.println(ssid);
    Serial.print("WiFi network password: ");
    Serial.println(password);
    Serial.print("Host IP Address: ");
    Serial.println(myIP);
    Serial.println("");
  }

  else {
    Serial.println("error! Something went wrong...");
  }
}

void loop() {
  Serial.printf("Number of connected devices (stations) = %d\n", WiFi.softAPgetStationNum());
  delay(3000);
}
