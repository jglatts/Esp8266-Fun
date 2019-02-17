/*
   Copyright (c) 2015, Majenko Technologies
   All rights reserved.

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
#define STASSID " "
#define STAPSK  " "
#endif
#define PIXPIN 5
#define NUM_PIX 12

const char *ssid = STASSID;
const char *password = STAPSK;

int cycles = 0;

ESP8266WebServer server(80);
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIX, PIXPIN, NEO_GRB + NEO_KHZ800);


void handleRoot() {
  char temp[400];
  int sec = millis() / 1000;
  int min = sec / 60;
  int hr = min / 60;

  snprintf(temp, 400,

           "<html>\
  <head>\
    <meta http-equiv='refresh' content='5'/>\
    <title>ESP8266 Demo</title>\
    <style>\
      body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088; }\
      p {font-size:2em;}\
    </style>\
  </head>\
  <body>\
    <h1>Hello from ESP8266!</h1>\
    <p>Uptime: %02d:%02d:%02d</p>\
    <img src=\"/test.svg\" />\
    <p><a href=\"/PixOn\">Pixel On</a></p>\
  </body>\
</html>",

           hr, min % 60, sec % 60
          );
  server.send(200, "text/html", temp);
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

void setup(void) {
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
  server.on("/test.svg", drawGraph);
  server.on("/inline", []() {
    server.send(200, "text/plain", "this works as well");
  });
  server.onNotFound(handleNotFound);
  server.begin();
  Serial.println("HTTP server started");
}


void loop(void) {
  server.handleClient();
  MDNS.update();
}


void flashLED() {
      char temp_pix[500];
      String color= "";
      
      // setup html first
      snprintf(temp_pix, 500,
             "<html>\
             <head>\
                <meta http-equiv='refresh' content='2'/>\
                <title>ESP8266 Demo</title>\
                <style>\
                    body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;}\
                    p {font-size: 2em;}\
                </style>\
                </head>\
             <body>\
                <h1>Hello from ESP8266!</h1>\
                <p>PIXEL CYCLES: %d</p>\
                <p><a href=\"/PixOff\">Pixel Off</a></p>\
             </body>\
             </html>", cycles);

    server.send(200, "text/html", temp_pix);
    // 5 cycles when href is active, testing what cycle time is the best
    for (int j = 0;j < 8; ++j) {
        // PIX On
        for (int i = 0; i < NUM_PIX; ++i) {
            pixels.setPixelColor(i, pixels.Color(50,0,155));
            pixels.show();
            color = "Blue";
        }
        delay(10);
        // PIX Off
        for (int x = 0; x < NUM_PIX; ++x) {
            pixels.setPixelColor(x, pixels.Color(0,0,0));
            pixels.show();
            color = "Off";
        }
        delay(10);  
    }
     cycles++;   // increase cycle value after every off/on
}


void offLED() {
      char temp_pix[500];
      
      // setup html first
      snprintf(temp_pix, 500,
             "<html>\
             <head>\
                <meta http-equiv='refresh' content='3'/>\
                <title>ESP8266 Demo</title>\
                <style>\
                    body { background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;}\
                    p {font-size: 2em;}\
                </style>\
                </head>\
             <body>\
                <h1>Hello from ESP8266!</h1>\
                <p><a href=\"/PixOn\">Pixel On</a></p>\
             </body>\
             </html>");
             
             for (int i = 0; i < NUM_PIX; ++i) {
                pixels.setPixelColor(i, pixels.Color(0,0,0));
                pixels.show();
             }
             
        server.send(200, "text/html", temp_pix);
}


void drawGraph() {
   // Make sure pixel is off
    for (int i = 0; i < NUM_PIX; ++i) {
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
    }
  String out = "";
  char temp[100];
  out += "<svg xmlns=\"http://www.w3.org/2000/svg\" version=\"1.1\" width=\"400\" height=\"150\">\n";
  out += "<rect width=\"400\" height=\"150\" fill=\"rgb(250, 230, 210)\" stroke-width=\"1\" stroke=\"rgb(0, 0, 0)\" />\n";
  out += "<g stroke=\"black\">\n";
  int y = rand() % 130;
  for (int x = 10; x < 390; x += 10) {
    int y2 = rand() % 130;
    sprintf(temp, "<line x1=\"%d\" y1=\"%d\" x2=\"%d\" y2=\"%d\" stroke-width=\"1\" />\n", x, 140 - y, x + 10, 140 - y2);
    out += temp;
    y = y2;
  }
  out += "</g>\n</svg>\n";

  server.send(200, "image/svg+xml", out);
}
