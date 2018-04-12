#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 6, NEO_GRB + NEO_KHZ800);

int pirPin = 2;
int ledPin = 6;
int ldrPin = 0;
int threshold = 50;

void setup() {
  Serial.begin(9600);
  pinMode(pirPin, INPUT);         //Motion detector digital input.
  pinMode(ldrPin, INPUT);         //Light sensor analog input
  pinMode(ledPin, OUTPUT);        //NeoPixel light strip
  strip.begin();
  strip.show();                   // Initialize all pixels to 'off'
  strip.setBrightness(55);        // Set LED brightness 0-255
  delay(30 * 1000);               // 30 seconds warm up time for PIR
}

void loop() {
  if (digitalRead(pirPin) ==HIGH && (analogRead(ldrPin)) > threshold) {
      ColorWipe(strip.Color(255, 0, 0), 25); // Red
      delay(300);

  if (digitalRead(pirPin) == LOW)
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
