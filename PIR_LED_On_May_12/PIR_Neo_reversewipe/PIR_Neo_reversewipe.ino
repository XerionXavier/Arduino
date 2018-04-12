#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 6, NEO_GRB + NEO_KHZ800);

int pirPin = 2;
int ledPin = 6;

void setup() {
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(55);                // Set LED brightness 0-255
}

void loop() {
  if(digitalRead(pirPin)==HIGH)
   ColorWipe(strip.Color(255, 0, 0), 25); // Red
   
  if(digitalRead(pirPin)==LOW)
   ReverseColorWipe(strip.Color(0, 0, 0), 25); // Black
}


void ColorWipe(uint32_t color, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, color);
      strip.show();
      delay(wait);
      }

}

void ReverseColorWipe(uint32_t c, uint8_t wait) {
  for(int i=(strip.numPixels()-1); i>=0; i--) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
      }
}
