#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 6, NEO_GRB + NEO_KHZ800);

#define LDR 0
#define PIR 2
#define LED 6

int pirState;
int ldrValue;

void setup() {
  //Serial.begin(9600);
  pinMode(LED, OUTPUT);
  pinMode(PIR, INPUT);
  digitalWrite(LED, LOW);
  strip.begin();
  strip.show();                   // Initialize all pixels to 'off'
  strip.setBrightness(55);        // Set LED brightness 0-255
  delay(30 * 1000);               // 30 seconds warm up time for PIR
}

void loop() {
  //ldrValue = analogRead(LDR);
  //Serial.print("Analog reading = ");
  //Serial.println(ldrValue);

  //if (ldrValue <= 200) { // dark
    digitalWrite(LED, HIGH);
    ColorWipe(strip.Color(255, 0, 0), 25); // Red
    delay(300);
    delay (5000);
    
    //if (digitalWrite(pirState) == LOW)
   // ReverseColorWipe(strip.Color(0, 0, 0), 25); // Black
  }
  
  else { // ldrValue > 200
    pirState = digitalRe//ad(PIR);
    if (pirState == LOW) {
      digitalWrite(LED, LOW);
      delay(5000);
      digitalWrite(LED, LOW);
      delay(1000);
    }
    else { // pirState == LOW
      digitalWrite(LED, LOW);
    }
  }

void ColorWipe(uint32_t color, uint8_t wait) {
    for (uint16_t i = 0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, color);
      strip.show();
      delay(wait);
  }

void ReverseColorWipe(uint32_t c, uint8_t wait) {
    for (int i = (strip.numPixels() - 1); i >= 0; i--) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
  // The processing in the Arduino occurs faster
  // than the response from the PIR, and adding this delay
  // eliminated a flickering on the LED
  delay(1000);
}
