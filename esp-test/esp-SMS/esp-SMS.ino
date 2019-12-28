/*
  ESP8266 program that moniters distance.
  If an object gets too close to the device it well send a text
  and email message.

  @author: johng

*/
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>

#define WIFI     "yourwifi"
#define PASSWORD "yourPW"
#define TRIG     4
#define ECHO     0
#define LIMIT    11	

const char* host      = "maker.ifttt.com";
String url            = "your-key";
const int httpsPort   = 443;
const int API_TIMEOUT = 10000;
int count, distance, duration;

BearSSL::WiFiClientSecure client;

void setup() {
	pinMode(TRIG, OUTPUT);
	pinMode(ECHO, INPUT);
	connectWIFI();
}

void loop() {
	if (getDistance() < LIMIT) sendMessage();
	delay(200);
}

void connectWIFI() {
	WiFi.begin(WIFI, PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {
		count++;
		delay(200);
	}
}

int getDistance() {
	digitalWrite(TRIG, LOW);
	delayMicroseconds(2);
	digitalWrite(TRIG, HIGH);
	delayMicroseconds(10);
	digitalWrite(TRIG, LOW);
	duration = pulseIn(ECHO, HIGH);
	distance = duration * 0.034 / 2;
	return distance;
}

void sendMessage() {
	connectClient();
	sendRequest();
	clientRead();
	delay(300);
}

void connectClient() {
	client.setInsecure();
	client.setTimeout(API_TIMEOUT);
	client.connect(host, httpsPort);
}

String getJSON() {
	// JSON data
	url += (String)distance;
	url += "&value2=";
	return url += (String)millis();
}

void sendRequest() {
	client.print(String("GET ") + getJSON() + " HTTP/1.1\r\n" +
		"Host: " + host + "\r\n" +
		"User-Agent: BuildFailureDetectorESP8266\r\n" +
		"Connection: close\r\n\r\n");
}

void clientRead() {
	while (client.connected()) {
		String line = client.readStringUntil('\n');
		if (line == "\r") break;
	}
}
