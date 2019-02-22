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


#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif
#define PIXPIN 5
#define TRIG 4
#define ECHO 0
#define NUM_PIX 12


const char *ssid = STASSID;
const char *password = STAPSK;


int cycles = 0; // put this in ledFlash() so it restarts at 0
int duration;
int distance = 0;
String dist_pix_color = "";


ESP8266WebServer server(80);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIX, PIXPIN, NEO_GRB + NEO_KHZ800);


// setup HTML pages as strings
const char* index_html = "<!DOCTYPE html>"
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
                         "<p>Please select a button bellow.</p>"
                         "<a href=\"/PixOn\">Pixel On</a>"
                         "</br>"
                         "<a href=\"/Distance\">Get Distance</a>"
                         "</body>"
                         "</html>";


const char* pix_on_html = "<!DOCTYPE html>"
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
                          "<p>Please select a button bellow.</p>"
                          "<a href=\"/PixOff\">Pixel Off</a>"
                          "</br>"
                          "<a href=\"/Distance\">Get Distance</a>"
                          "</body>"
                          "</html>";


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
                           "<p>Please select a button bellow.</p>"
                           "<p>Pixel is OFF</p>"
                           "<a href=\"/PixOff\">Pixel On</a>"
                           "</br>"
                           "<a href=\"/Distance\">Get Distance</a>"
                           "</body>"
                           "</html>";


/* Landing Page */
void handleRoot() {
    server.send(200, "text/html", index_html);
}


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
    server.on("/Distance", getDistance);
    server.on("/inline", []() {
        server.send(200, "text/plain", "this works as well");
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
    String color= "";

    server.send(200, "text/html", pix_on_html);
    // 5 cycles when href is active, testing what cycle time is the best
    for (int j = 0;j < 24; ++j) {
        // PIX On and switch Color
        if (j%2==0) {
            for (int i = 0; i < NUM_PIX; ++i) {
                pixels.setPixelColor(i, pixels.Color(0,0,155));
                pixels.show();
                color = "Blue";
            }
        } else {
            for (int q = 0; q < NUM_PIX; ++q) {
                // testing the 'running' cycle
                pixels.setPixelColor(q, pixels.Color(255,0,0));
                pixels.show();
                color = "Blue";
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
    cycles++;   // increase cycle value after every off/on
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
    for (int i = 0; i < NUM_PIX; ++i) {
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
    }

    digitalWrite(TRIG, LOW);
    delayMicroseconds(2);
    digitalWrite(TRIG, HIGH);
    delayMicroseconds(10);
    digitalWrite(TRIG, LOW);
    // update distance
    duration = pulseIn(ECHO, HIGH);
    distance= duration * 0.034/2;
    String color = "";
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
       color += "RED, HARMFUL OBJECT";
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
             distance_html +=                     "<p>Please select a button bellow.</p>";
             distance_html +=                     "<p>Current Distance: ";
             distance_html +=                     distance;
             distance_html +=                     "</p>";
             distance_html +=                     "<p>PIXEL Color: ";
             distance_html +=                     color;
             distance_html +=                     "</p>";       
             distance_html +=                     "<a href=\"/PixOn\">Pixel On</a>";
             distance_html +=                     "</br>";
             distance_html +=                     "<a href=\"/PixOff\">Pixel Off</a>";
             distance_html +=                     "</body>";
             distance_html +=                     "</html>";
    
    server.send(200, "text/html", distance_html);

}

