/*

  Author: John Glatts
  
  Brief: NodeMCU program that sends html, and a bit off css, to a webserver. An Adafruit neo-pixel is hooked up with various 
         LED patterns. The server also displays data from a HC-SR04 distance sensor.  
  
  -ToDo
        - 3/1/19 No Compile, yet....
        - Use random() to generate color codes
        - Check if randomseed() of a[0] is needed
*/
#include <Adafruit_NeoPixel.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>

// WiFi defines and I/O Pins
#ifndef STASSID
#define STASSID ""
#define STAPSK  ""
#endif
// double check these pins are useable
#define PIXPIN 5
#define TRIG 4
#define ECHO 0
#define NUM_PIX 12
#define MAX_SIZE 1000
#define COLOR_SIZE 3    // 3 for now, may change if randomint() works


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
int dist_increase = 0;  // variable to iterate over all_distances[]
int global_color_change[COLOR_SIZE] = {255, 155, 55};
int global_start_time = millis();
// global strings for html
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
                         "</br>"
                         "<a href=\"/AllDistances\">All Distances</a>"
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
    message += "Click a Link to Change";
    message += "<a href=\"/PixOn\">Pixel On</a>";
    message += "<br>";
    message += "<a href=\"/Distance\">Get Distance</a>";
    message += "<br>";
    message += "<a href=\"/PixCrawl\">Pixel Crawl</a>";
    message += "<br>";
    message += "<a href=\"/PixOnebyOne\">Pixel One By One</a>";

    for (uint8_t i = 0; i < server.args(); i++) {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }

    server.send(404, "text/plain", message);
}


/* Setup Everything Dawg */
void setup(void) {
    randomSeed(analogRead(0));  // check to see if seed is really needed
    all_distances[0] = 0;   // set to 0 to avoid garbage values
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
    server.on("/AllDistances", allDistances);
    server.on("/PixOnebyOne", pixOneByOne);
    server.on("/ShutDown", shutDown);
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

    // check Uptime -- if UT > 5 -- send a separate page and ask to turn MCU off
    if (min >= 1000) {
        String time_alert = "<!DOCTYPE html>";
        time_alert += "<html>";
        //  time_alert += "<meta http-equiv='refresh' content='3'/>"; no need for update
        time_alert += "<style>";
        time_alert += "p {";
        time_alert += "color: red;";  // change text
        time_alert += "font-size: 4em";
        time_alert += "</style>";
        time_alert += "<body>";
        time_alert += "<p>";
        time_alert += "ESP HAS BEEN ON FOR: ";
        time_alert += min % 60;
        time_alert += sec % 60;
        time_alert += "</p>";
        time_alert += "<h2>";
        time_alert += "Keep The ESP and PIXEL On?";
        time_alert += "</h2>";
        // test to send it back to landing page, may have to be another link
        time_alert += "<a href=\"/\">Yes</a>";
        time_alert += "<a href=\"ShutDown\">No</a>";
        time_alert += "<style>";
        time_alert += "</body>";
        server.send(200, "text/html", time_alert);
    } else {
        // Test where the best spot for this will be
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
        pix_cycle_html += "</p>";
        pix_cycle_html += "<p>PIXEL Color: ";
        pix_cycle_html += color;
        pix_cycle_html += "</p>";
        pix_cycle_html += "<p>Uptime: ";
        pix_cycle_html += min % 60;
        pix_cycle_html += " minutes ";
        pix_cycle_html += sec % 60;
        pix_cycle_html += " seconds";
        pix_cycle_html += "</p>";
        pix_cycle_html += "<a href=\"/PixOff\">Pixel Off</a>";
        pix_cycle_html += "</br>";
        pix_cycle_html += "<a href=\"/Distance\">Get Distance</a>";
        pix_cycle_html += "</body>";
        pix_cycle_html += "</html>";
        server.send(200, "text/html", pix_cycle_html);

        // 24 cycles when href is active, completes loop first then can go to other links
        for (int j = 0; j < 24; ++j) {
            // PIX On and switch Color
            if (j % 2 == 0) {
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
}

/* Turn the PIXEL off */
void offLED() {
    int current_time = millis();
    int total_off_time = current_time - global_start_time; // get total PIXEL off time, in Milliseconds(), may have change
    long minutes = (total_off_time / 1000)  / 60;

    for (int i = 0; i < NUM_PIX; ++i) {
        pixels.setPixelColor(i, pixels.Color(0,0,0));
        pixels.show();
    }

    // create html page and display total OFF time
    String pix_off_html = "<!DOCTYPE html>";
    pix_off_html += "<html>";
    pix_off_html += "<head>";
    pix_off_html += "<meta http-equiv='refresh' content='3'/>";
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
    pix_off_html += "<a href=\"/Distance\">Get Distance</a>";
    pix_off_html += "</body>";
    pix_off_html += "</html>";

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
    // check if we can display distance
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

/* Theatre-Style LED chase, from example code in pixel-library */
void pixCrawl() {
    int rand = random(0, 256);
    server.send(200, "text/html", index_html);
    for (int j=0;j < 15; j++) {  // 15 cycles of chasing
        for (int q=0; q < 3; q++) {
            for (int i=0; i < pixels.numPixels(); i=i+3) {
                pixels.setPixelColor(i+q, 0, rand, 0);    // turn every third pixel on
            }
            pixels.show();
            delay(80);
            for (int i=0; i < pixels.numPixels(); i=i+3) {
                pixels.setPixelColor(i+q, 0);        // turn every third pixel off
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
    for (int i = 0; i < pixels.numPixels(); ++i) {
        if (i % 2 == 0) {
            pixels.setPixelColor(i, rand_numb_0, rand_numb_1, rand_numb_2);
            pixels.show();
            delay(40);
        }
        else {
            pixels.setPixelColor(i, 0, 255, 255);
            pixels.show();
            delay(40);
        }
    }
    // turn pixel off, then start another cycle
    for (int x = 0; x < NUM_PIX; ++x) {
        pixels.setPixelColor(x, 0, 0, 0);
        pixels.show();
        delay(60);
    }
}


/* Turn off pixel and add a sleep mode for the ESP */
void shutDown() {
    // first make sure PIXEL is off
    for (int i = 0; i < NUM_PIX; ++i) {
        pixels.setPixelColor(i, 0, 0, 0);
        pixels.show();
    }
}


/* Display all distances through the PIXEL, if applicable */
void allDistances() {
    // make sure PIXEL is off at start, may have to go inside html for-loop
    for (int z = 0; z < NUM_PIX; ++z) {
        pixels.setPixelColor(z, 0);
        pixels.show();
    }

    // setup new html page each time
    String all_distances_html = "<!DOCTYPE html>";
    all_distances_html += "<html>";
    all_distances_html += "<head>";
    // try using AJAX and get rid of refresh
    all_distances_html += "<meta http-equiv='refresh' content='5'/>";
    all_distances_html += "<style>";
    all_distances_html += "p {";
    all_distances_html += "font-size: 3em;";
    all_distances_html += "}";
    all_distances_html += "a {";
    all_distances_html += "font-size: 4em;";
    all_distances_html += "}";
    all_distances_html += "body {";
    all_distances_html += "background-color: #cccccc; font-family: Arial, Helvetica, Sans-Serif; Color: #000088;";
    all_distances_html += "}";
    all_distances_html += "</style>";
    all_distances_html += "</head>";
    all_distances_html += "<body>";
    all_distances_html += "<p>JDG ESP SERVER</p>";
    all_distances_html += "<p>Please select a button below.</p>";
    all_distances_html += "<p>Distance: ";
    all_distances_html += dist_increase;
    all_distances_html += " = ";
    all_distances_html += all_distances[dist_increase];
    all_distances_html += "</p>";
    all_distances_html += "<a href=\"/PixOn\">Pixel On</a>";
    all_distances_html += "</br>";
    all_distances_html += "<a href=\"/Distance\">Get Distance</a>";
    all_distances_html += "</body>";
    all_distances_html += "</html>";
    // send the webpage
    server.send(200, "text/html", all_distances_html);
    // check if we should display on PIXEL
    if (all_distances[dist_increase] <= NUM_PIX) {
        // Display distance on PIXEL
        for (int j = 0; j < all_distances[dist_increase]; ++j) {
            pixels.setPixelColor(j, 155, 20, 255);
            pixels.show();
        }
        delay(4000);
    }
    else {
        // make sure PIXEL is off
        for (int k = 0; k < NUM_PIX; ++k) {
            pixels.setPixelColor(k, 0);
            pixels.show();
        }
    }
    // turn pixel off when loop is done
    for (int x = 0; x < NUM_PIX; ++x) {
        pixels.setPixelColor(x, 0);
        pixels.show();
    }
    // for now this works, but when it goes pass the actual number of values in all_distances[],
    // it will display garbage values
    dist_increase++;
}

