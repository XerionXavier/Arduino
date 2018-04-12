#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 6, NEO_GRB + NEO_KHZ800);

int pirPin = 2;
int ledPin = 6;
int ldrPin = 0;
int threshold = 1000;
int v = analogRead(ldrPin);
volatile unsigned long lastOnTime;
int long onDuration = 120000;
// Variables will change :
int ledState = LOW;             // ledState used to set the LED
//void colorWipe();

// Generally, you should use "unsigned long" for variables that hold time
// The value will quickly become too large for an int to store
//unsigned long previousMillis = 0;        // will store last time LED was updated

//constants won't change :
//const long interval = 100000;           // Delay keeping PIR HIGH

void setup() {
  //  Serial.begin(9600);
  pinMode(ldrPin, INPUT);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(50);                // Set LED brightness 0-255

}
void loop() {
  Serial.println(v);
  delay(1000);
  if (analogRead(ldrPin) < threshold)
    if (digitalRead(pirPin) == HIGH)
    ColorWipe(strip.Color(255, 0, 0), 50);  // Red
  //delay(500);
  //unsigned long currentMillis = millis();
  lastOnTime = 0;
  // save the last time you blinked the LED
  //previousMillis = currentMillis;
  //(millis() <= lastOnTime +  onDuration);


  if (digitalRead(pirPin) == LOW);
  lastOnTime = 0; //Reset
  ReverseColorWipe(strip.Color(0, 0, 0), 50); // Black
  //delay(1000);
}

void ColorWipe(uint32_t color, uint8_t wait) {
  for (uint16_t i = 0; i < strip.numPixels(); i++) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }
}

void ReverseColorWipe(uint32_t color, uint8_t wait) {
  for (int i = (strip.numPixels() - 1); i >= 0; i--) {
    strip.setPixelColor(i, color);
    strip.show();
    delay(wait);
  }

}
