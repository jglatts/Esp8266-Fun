#include <Adafruit_NeoPixel.h>
 
#define BTN   4
#define PIN   5

int pix = 0;  // starting increment value
int turn_off = 0; 
int rotations = 0; 

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(12, PIN, NEO_GRB + NEO_KHZ800);

void setup() {
  pinMode(BTN, INPUT_PULLUP);
  pixels.begin();
  pixels.show();
}

void loop() {
  if(digitalRead(BTN) != LOW) {
    // do nothing  
  } else {
    if (pix < 12) {
        if(rotations % 2 == 0) {
              pixels.setPixelColor(pix, pixels.Color(255,0,0)); // red
              pixels.show();
              pix++;
              delay(250);  
        } else {
              pixels.setPixelColor(pix, pixels.Color(0,0,255)); // blue
              pixels.show();
              pix++;
              delay(250); 
        }

    } else {
        for (int i = 0; i < 12; ++i) {
            pixels.setPixelColor(i, pixels.Color(0,0,0));
            pixels.show();
        } 
        pix = 0;
        rotations++;
        delay(1000);  
     }
  }
}
