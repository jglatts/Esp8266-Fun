#include <ESP8266WiFi.h>
#include <Adafruit_NeoPixel.h>
#include "Gsender.h"

#define pix_pin 0
#define NUM_PIX 12

#pragma region Globals
const char* ssid = "yourWifi";
const char* password = "yourPW";
String subject = "AYY - Sent From ESP";
uint8_t connection_state = 0;
uint16_t reconnect_interval = 10000;
#pragma endregion Globals

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_PIX, pix_pin, NEO_GRB + NEO_KHZ800);


uint8_t WiFiConnect(const char* nSSID = nullptr, const char* nPassword = nullptr)
{
  static uint16_t attempt = 0;
  Serial.print("Connecting to ");
  if (nSSID) {
    WiFi.begin(nSSID, nPassword);
    Serial.println(nSSID);
  }
  else {
    WiFi.begin(ssid, password);
    Serial.println(ssid);
  }

  uint8_t i = 0;
  while (WiFi.status() != WL_CONNECTED && i++ < 50)
  {
    delay(200);
    Serial.print(".");
  }
  ++attempt;
  Serial.println("");
  if (i == 51) {
    Serial.print("Connection: TIMEOUT on attempt: ");
    Serial.println(attempt);
    if (attempt % 2 == 0)
      Serial.println("Check if access point available or SSID and Password\r\n");
    return false;
  }
  Serial.println("Connection: ESTABLISHED");
  Serial.print("Got IP address: ");
  Serial.println(WiFi.localIP());
  return true;
}

void Awaits()
{
  uint32_t ts = millis();
  while (!connection_state)
  {
    delay(50);
    if (millis() > (ts + reconnect_interval) && !connection_state) {
      connection_state = WiFiConnect();
      ts = millis();
    }
  }
}

String msg() {
  // gotta be a cleaner way to do this
  // but this is able to print new lines
  // uses the <br> tag
  String message = "We've ticked: ";
  message += millis();
  message += " times!!";
  message += "<br><br> Now it's ";
  message += millis();
  message += "<br>ESP ChipID: ";
  message += String(ESP.getChipId());
  message += "<br>ESP Skecth: ";
  message += ESP.getSketchMD5();
  message += "<br>ESP MAC Address: ";
  message += String(WiFi.macAddress());
  return message;

}

void sendEmail() {
  Gsender* gsender = Gsender::Instance();    // Getting pointer to class instance
  if (gsender->Subject(subject)->Send("johnglatts1@hotmail.com", msg())) {
    Serial.println("Message send.");
    lightUpPixel();
  }
  else {
    Serial.print("Error sending message: ");
    Serial.println(gsender->getError());
  }

}

void lightUpPixel() {
  for (int i = 0; i < NUM_PIX; ++i) {
    pixels.setPixelColor(i, pixels.Color(155, 0, 155));
    pixels.show();
    delay(500);
  }
}

void turnOffPixel() {
  for (int i = 0; i < NUM_PIX; ++i) {
    pixels.setPixelColor(i, 0);
    pixels.show();
  }
}

void setup()
{
  Serial.begin(115200);
  pixels.begin();
  pixels.show();
  if (!WiFiConnect()) Awaits();
  turnOffPixel();
  delay(2500);
  sendEmail();
}

void loop() {
  lightUpPixel();
  turnOffPixel();
  delay(200);
}
