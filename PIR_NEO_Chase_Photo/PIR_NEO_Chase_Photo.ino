#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 6, NEO_GRB + NEO_KHZ800);

int pirPin = 2;
int ledPin = 6;
int ldrPin = 0;
int threshold = 20;

void setup() {
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(45);                // Set LED brightness 0-255
}

void loop() {
  int v = analogRead(ldrPin);
   Serial.println(v);
   delay(1000);
   
  if (digitalRead(pirPin) ==HIGH && (analogRead(ldrPin)) < threshold)
   ColorWipe(strip.Color(255, 0, 0), 80); // Red
   delay(500);
   
  if (digitalRead(pirPin)==LOW)
   ReverseColorWipe(strip.Color(0, 0, 0), 100); // Black
   delay(1000);
}


void ColorWipe(uint32_t color, uint8_t wait) {
  for(uint16_t i=0; i<strip.numPixels(); i++) {
      strip.setPixelColor(i, color);
      strip.show();
      delay(wait);
      }

}

void ReverseColorWipe(uint32_t color, uint8_t wait) {
  for(int i=(strip.numPixels()-1); i>=0; i--) {
      strip.setPixelColor(i, color);
      strip.show();
      delay(wait);
      }
}
