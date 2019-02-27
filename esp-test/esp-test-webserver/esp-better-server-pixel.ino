/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

   Author: John Glatts

   Redistribution and use in source and binary forms, with or without modification,
   are permitted provided that the following conditions are met:

 * * Redistributions of source code must retain the above copyright notice, this
     list of conditions and the following disclaimer.

 * * Redistributions in binary form must reproduce the above copyright notice, this
     list of conditions and the following disclaimer in the documentation and/or
     other materials provided with the distribution.

 * * Neither the name of Majenko Technologies nor the names of its
     contributors may be used to endorse or promote products derived from
     this software without specific prior written permission.

   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
   ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
   WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
   DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
   ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
   (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
   LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
   ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

*/
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// WiFi defines and I/O Pins
#ifndef STASSID
#define STASSID " "
#define STAPSK  " "
#endif
// double check these pins are useable
#define PIXPIN 5
#define TRIG 4
#define ECHO 0
#define NUM_PIX 12
#define MAX_SIZE 1000


// Strings for WiFi
const char *ssid = STASSID;
const char *password = STAPSK;


// Variables for I/O, initialed to 0 to avoid garbage values
int cycles = 0;
int motor_revs = 0;
int collected_dists = 0;
int duration = 0;
int distance = 0;
int all_distances[MAX_SIZE];
String dist_pix_color = "";
String color = "";
String btn_state = "";


// Create a new Server and Neopixel instance
ESP8266WebServer server(80);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIX, PIXPIN, NEO_GRB + NEO_KHZ800);


/* Landing page will work like this, because no data is being displayed */
const char* index_html = "<!DOCTYPE html>"
                         "<html>"
                         "<head>"
                         "<meta http-equiv='refresh' content='3'/>"
                         "<style>"
                         "p {"
                         "font-size: 3em;"
                         "}"
                         "a {"
                         "font-size: 4em;"
                         "}"
                         "body {"
                         "background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;"
                         "}"
                         "</style>"
                         "</head>"
                         "<body>"
                         "<p>JDG ESP SERVER</p>"
                         "<p>Please select a button below.</p>"
                         "<a href=\"/PixOn\">Pixel On</a>"
                         "</br>"
                         "<a href=\"/PixCrawl\">Pixel Crawl</a>"
                         "</br>"
                         "<a href=\"/PixOnebyOne\">Pixel One by One</a>"
                         "</br>"
                         "<a href=\"/Distance\">Get Distance</a>"
                         "</body>"
                         "</html>";


/* This setup will also work, no data being displayed */
const char* pix_off_html = "<!DOCTYPE html>"
                           "<html>"
                           "<head>"
                           "<style>"
                           "p {"
                           "font-size: 3em;"
                           "}"
                           "a {"
                           "font-size: 4em;"
                           "}"
                           "body {"
                           "background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;"
                           "}"
                           "</style>"
                           "</head>"
                           "<body>"
                           "<p>JDG ESP SERVER</p>"
                           "<p>Please select a button below.</p>"
                           "<p>Pixel is OFF</p>"
                           "<a href=\"/PixOn\">Pixel On</a>"
                           "</br>"
                           "<a href=\"/Distance\">Get Distance</a>"
                           "</body>"
                           "</html>";


/* Landing Page */
void handleRoot() {
    server.send(200, "text/html", index_html);
}


/* URL not found */
void handleNotFound() {
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";

    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    server.send(404, "text/plain", message);
}


/* Setup Everything Dawg */
void setup(void) {
    all_distances[0] = 0;   
    pinMode(TRIG, OUTPUT);
    pinMode(ECHO, INPUT);
    pixels.begin();
    pixels.show();
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        Serial.println("Not Connected");
    }

    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp8266")) {
        Serial.println("MDNS responder started");
    }
    server.on("/", handleRoot);
    server.on("/PixOn", flashLED);
    server.on("/PixOff", offLED);
    server.on("/PixCrawl", pixCrawl);
    server.on("/Distance", getDistance);
    server.on("/PixOnebyOne", pixOneByOne);
    server.on("/inline", []() {
         server.send(200, "text/plain", "this works as well");
     });
    server.on("/about", []() {
         server.send(200, "text/plain", "JDG ESP-8266 WEBSERVER\n2019");
     });
     server.onNotFound(handleNotFound);
    server.begin();
    Serial.println("HTTP server started");
}


/* Keep the server up and update it */
void loop(void) {
    server.handleClient();
    MDNS.update();
}


/* Flash the PIXEL and display cycle count */
void flashLED() {
    // variables to display uptime
    int sec = millis() / 1000;
    int min = sec / 60;
    int hr = min / 60;


    // Test where the best spot for this will be
    // setup HTML string
    String pix_cycle_html = "<!DOCTYPE html>";
    pix_cycle_html +=                     "<html>";
    pix_cycle_html +=                     "<head>";
    pix_cycle_html +=                     "<meta http-equiv='refresh' content='3'/>";
    pix_cycle_html +=                     "<style>";
    pix_cycle_html +=                     "p {";
    pix_cycle_html +=                     "font-size: 3em;";
    pix_cycle_html +=                     "}";
    pix_cycle_html +=                     "a {";
    pix_cycle_html +=                     "font-size: 4em;";
    pix_cycle_html +=                     "}";
    pix_cycle_html +=                     "body {";
    pix_cycle_html +=                     "background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;";
    pix_cycle_html +=                     "}";
    pix_cycle_html +=                     "</style>";
    pix_cycle_html +=                     "</head>";
    pix_cycle_html +=                     "<body>";
    pix_cycle_html +=                     "<p>JDG ESP SERVER</p>";
    pix_cycle_html +=                     "<p>Please select a button below.</p>";
    pix_cycle_html +=                     "<p>Current Pixel Cycle: ";
    pix_cycle_html +=                     cycles;
    pix_cycle_html +=                     "</p>";
    pix_cycle_html +=                     "<p>PIXEL Color: ";
    pix_cycle_html +=                     color;
    pix_cycle_html +=                     "</p>";
    pix_cycle_html +=                     "<p>Uptime: ";
    pix_cycle_html +=                     min%60;
    pix_cycle_html +=                     " minutes ";
    pix_cycle_html +=                     sec%60;
    pix_cycle_html +=                     " seconds";
    pix_cycle_html +=                     "</p>";
    pix_cycle_html +=                     "<a href=\"/PixOff\">Pixel Off</a>";
    pix_cycle_html +=                     "</br>";
    pix_cycle_html +=                     "<a href=\"/Distance\">Get Distance</a>";
    pix_cycle_html +=                     "</body>";
    pix_cycle_html +=                     "</html>";
    server.send(200, "text/html", pix_cycle_html);

    // 24 cycles when href is active, completes loop first then can go to other links
    for (int j = 0;j < 24; ++j) {
        // PIX On and switch Color
        if (j%2==0) {
            for (int i = 0; i < NUM_PIX; ++i) {
                pixels.setPixelColor(i, pixels.Color(0,0,155)); // blue
                pixels.show();
                color = "Blue";
            }
        } else {
            for (int q = 0; q < NUM_PIX; ++q) {
                pixels.setPixelColor(q, pixels.Color(255,0,0)); // red
                pixels.show();
                color = "Red";
            }
        }
        delay(25);
        // PIX Off
        for (int x = 0; x < NUM_PIX; ++x) {
            pixels.setPixelColor(x, pixels.Color(0,0,0));
            pixels.show();
            color = "Off";
        }
        delay(25);
    }
    cycles++;
}


/* Turn the PIXEL off */
void offLED() {
    for (int i = 0; i < NUM_PIX; ++i) {
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
    }
    server.send(200, "text/html", pix_off_html);
}


/* Activate the HC-SR04 and update the distance */
void getDistance() {
    // Turn PIXEL off  
    for (int i = 0; i < NUM_PIX; ++i) {
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
    }
    // Get Distance
    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    duration = pulseIn(ECHO, HIGH);
    distance= duration * 0.034/2;
    String color = "";
    collected_dists++;
    all_distances[collected_dists] = distance;
    if (distance > NUM_PIX) {
        for (int i = 0; i < NUM_PIX; ++i) {
            pixels.setPixelColor(i, pixels.Color(0,0,0));
            pixels.show();
        }
        color += "Off, No Harmful Objects";
    }  else {
        for (int i = 0; i < distance; ++i) {
            pixels.setPixelColor(i, pixels.Color(255,0,0));  // red
            pixels.show();
        }
        color += "RED, HARMFUL OBJECT -- CHECK PIXEL TO VERIFY";
    }
    String distance_html = "<!DOCTYPE html>";
    distance_html +=                     "<html>";
    distance_html +=                     "<head>";
    distance_html +=                     "<meta http-equiv='refresh' content='3'/>";
    distance_html +=                     "<style>";
    distance_html +=                     "p {";
    distance_html +=                     "font-size: 3em;";
    distance_html +=                     "}";
    distance_html +=                     "a {";
    distance_html +=                     "font-size: 4em;";
    distance_html +=                     "}";
    distance_html +=                     "body {";
    distance_html +=                     "background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;";
    distance_html +=                     "}";
    distance_html +=                     "</style>";
    distance_html +=                     "</head>";
    distance_html +=                     "<body>";
    distance_html +=                     "<p>JDG ESP SERVER</p>";
    distance_html +=                     "<p>Please select a button below.</p>";
    distance_html +=                     "<p>Current Distance: ";
    distance_html +=                     distance;
    distance_html +=                     "</p>";
    distance_html +=                     "<p>PIXEL Color: ";
    distance_html +=                     color;
    distance_html +=                     "</p>";
    distance_html +=                     "<p>Last Distance: ";
    distance_html +=                     all_distances[collected_dists-1];
    distance_html +=                     "<p>Distances Collected: ";
    distance_html +=                     collected_dists;
    distance_html +=                     "</p>";
    distance_html +=                     "<a href=\"/PixOn\">Pixel On</a>";
    distance_html +=                     "</br>";
    distance_html +=                     "<a href=\"/PixOff\">Pixel Off</a>";
    distance_html +=                     "</body>";
    distance_html +=                     "</html>";

    server.send(200, "text/html", distance_html);

}


void pixCrawl() {
  server.send(200, "text/html", index_html);
  for (int j=0;j < 20; j++) {  //do 20 cycles of chasing
    for (int q=0; q < 3; q++) {
      for (int i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0, 250, 0);    //turn every third pixel on
      }
      pixels.show();
      delay(150);
      for (int i=0; i < pixels.numPixels(); i=i+3) {
        pixels.setPixelColor(i+q, 0);        //turn every third pixel off
      }
    }
  }
}


void pixOneByOne() {
    server.send(200, "text/html", index_html);
    for (int i = 0;i < pixels.numPixels();++i) {
        pixels.setPixelColor(i, 0, 0, 255);
        pixels.show();
        delay(60);   
    }
    for (int x = 0;x < pixels.numPixels();++x) {
        pixels.setPixelColor(x, 0, 0, 0);
        pixels.show();
        delay(60);   
    }
}
