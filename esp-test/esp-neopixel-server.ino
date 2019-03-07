/*

  Author: John Glatts

  Brief: NodeMCU program that sends html, and a bit off css, to a webserver. An Adafruit neo-pixel is hooked up with various
         LED patterns. Testing different LED patters with random()

  -ToDo
        - Add more dope patterns mayne
        - Figure out why esp is timing out at the Z
*/
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>


// Pixel defines
#define PIXPIN 5
#define NUM_PIX 12


// Strings for WiFi
const char *ssid = "";
const char *password = "";


// I/O variables
int cycles = 0;
int global_start_time = millis();
String color = "";


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
    randomSeed(analogRead(0));  // seed for random(), only has to be called once
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
    int current_time = millis();
    int total_on_time = current_time - global_start_time; // get total PIXEL off time, in Milliseconds(), may have change
    long minutes = (total_on_time / 1000)  / 60;

    // setup HTML string
    String pix_cycle_html = "<!DOCTYPE html>";
    pix_cycle_html += "<html>";
    pix_cycle_html += "<head>";
    pix_cycle_html += "<meta http-equiv='refresh' content='3'/>";
    pix_cycle_html += "<style>";
    pix_cycle_html += "p {";
    pix_cycle_html += "font-size: 3em;";
    pix_cycle_html += "}";
    pix_cycle_html += "a {";
    pix_cycle_html += "font-size: 4em;";
    pix_cycle_html += "}";
    pix_cycle_html += "body {";
    pix_cycle_html += "background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;";
    pix_cycle_html += "}";
    pix_cycle_html += "</style>";
    pix_cycle_html += "</head>";
    pix_cycle_html += "<body>";
    pix_cycle_html += "<p>JDG ESP SERVER</p>";
    pix_cycle_html += "<p>Please select a button below.</p>";
    pix_cycle_html += "<p>Current Pixel Cycle: ";
    pix_cycle_html += cycles;
    pix_cycle_html += "<p>PIXEL LED On Time: ";
    pix_cycle_html += minutes;
    pix_cycle_html += "</p>";
    pix_cycle_html += "<p>PIXEL Color: ";
    pix_cycle_html += color;
    pix_cycle_html += "</p>";
    pix_cycle_html += "<a href=\"/PixOff\">Pixel Off</a>";
    pix_cycle_html += "</br>";
    pix_cycle_html += "</body>";
    pix_cycle_html += "</html>";
    server.send(200, "text/html", pix_cycle_html);

    // 32 cycles when href is active, completes loop first then can go to other links
    for (int j = 0; j < 32; ++j) {
        // PIX On and switch Color
        if (j % 2 == 0) {
            // use a switch-case here
            for (int i = 0; i < NUM_PIX; ++i) {
                pixels.setPixelColor(i, pixels.Color(0, 0, 155)); // blue
                pixels.show();
                color = "Blue";
            }
        } else {
            for (int q = 0; q < NUM_PIX; ++q) {
                pixels.setPixelColor(q, pixels.Color(255, 0, 0)); // red
                pixels.show();
                color = "Red";
            }
        }
        delay(25);
        // PIX Off
        for (int x = 0; x < NUM_PIX; ++x) {
            pixels.setPixelColor(x, pixels.Color(0, 0, 0));
            pixels.show();
            color = "Off";
        }
        delay(25);
    }
    cycles++;
}

/* Turn the PIXEL off */
void offLED() {
    int current_time = millis();
    int total_off_time = current_time - global_start_time; // get total PIXEL off time, in Milliseconds(), may have change
    long minutes = (total_off_time / 1000)  / 60;

    // move the loop to the bottom, and sens the web-page first, may help with timing
    for (int i = 0; i < NUM_PIX; ++i) {
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
    }

    // create html page and display total OFF time
    String pix_off_html = "<!DOCTYPE html>";
    pix_off_html += "<html>";
    pix_off_html += "<head>";
    pix_off_html += "<style>";
    pix_off_html += "p {";
    pix_off_html += "font-size: 3em;";
    pix_off_html += "}";
    pix_off_html += "a {";
    pix_off_html += "font-size: 4em;";
    pix_off_html += "}";
    pix_off_html += "body {";
    pix_off_html += "background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;";
    pix_off_html += "}";
    pix_off_html += "</style>";
    pix_off_html += "</head>";
    pix_off_html += "<body>";
    pix_off_html += "<p>JDG ESP SERVER</p>";
    pix_off_html += "<p>Please select a button below.</p>";
    pix_off_html += "<p>Pixel is OFF</p>";
    pix_off_html += "<p>Total Pixel off time: ";
    pix_off_html += minutes;
    pix_off_html += " minutes </p>";
    pix_off_html += "<a href=\"/PixOn\">Pixel On</a>";
    pix_off_html += "<a href=\"/\">Home</a>";
    pix_off_html += "</br>";
    pix_off_html += "</body>";
    pix_off_html += "</html>";

    server.send(200, "text/html", pix_off_html);
}


/* Theatre-Style LED chase, from example code in pixel-library */
void pixCrawl() {
    int rand_num = random(0, 256);
    int rand_num_1 = random(0, 256);
    int rand_num_2 = random(0, 256);

    server.send(200, "text/html", index_html);
    // 10 cycles of crawling
    for (int j=0;j < 10; j++) {
        if (j%2==0) {
            // Crawl-effect, looping through every sixth pixel
            for (int k = 0; k < 6; ++k) {
                    if (a + k == 2 || a + k == 8) {
                        pixels.setPixelColor(a + k, 0, rand_num_1, rand_num_1);
                    } else {
                        pixels.setPixelColor(a + k, rand_num, rand_num_1, rand_num_2);
                    }
                }
                pixels.show();
                delay(80);
                for (int z=0; z < pixels.numPixels(); z++) {
                    pixels.setPixelColor(z, 0);
                }
            }
        } else {
            // Crawl-effect, looping through every second pixel
            for (int k = 0; k < 2; ++k) {
                for (int a=0; a < pixels.numPixels(); a=a+2) {
                    if (a + k == 2 || a + k == 6 || a + k == 10) {
                        // poss change back to rand_num_1, LED seems to 'flicker' when random() is here
                        pixels.setPixelColor(a + k, 0, rand_num_1, random(10, 190));
                    } else {
                        pixels.setPixelColor(a + k, rand_num, rand_num_1, rand_num_2);
                    }
                }
                pixels.show();
                delay(80);
                // turn every second pixel off
                for (int z=0; z < pixels.numPixels(); z++) {
                    pixels.setPixelColor(z, 0);
                }
            }
        }
    }
}


/* Light up each pixel, then turn each pixel of */
void pixOneByOne() {
    int rand_numb_0 = random(0, 256);
    int rand_numb_1 = random(0, 256);
    int rand_numb_2 = random(0, 256);

    server.send(200, "text/html", index_html);
    // test to change color on each PIXEL
    for (int i = 0; i < pixels.numPixels(); ++i) {
        pixels.setPixelColor(i, random(0,256), random(0,256), random(0,256));
        pixels.show();
        delay(40);
    }
    // turn pixel off, then start another cycle
    for (int x = 0; x < NUM_PIX; ++x) {
        pixels.setPixelColor(x, 0, 0, 0);
        pixels.show();
        delay(60);
    }
}
