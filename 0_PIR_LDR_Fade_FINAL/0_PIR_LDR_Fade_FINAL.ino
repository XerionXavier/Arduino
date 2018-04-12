#include <Adafruit_NeoPixel.h>
#define NUM_LEDS 60

//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 20;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 15000;

boolean lockLow = true;
boolean takeLowTime;

int pirPin = 2;
int ledPin = 6;
int lightPin = 0;
int threshold = 70; //Should be 70

Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUM_LEDS, ledPin, NEO_GRB + NEO_KHZ800);

/////////////////////////////
//SETUP
void setup() {
  Serial.begin(9600);
  pinMode(pirPin, INPUT);
  pinMode(ledPin, OUTPUT);
  digitalWrite(pirPin, LOW);
  pinMode(lightPin, INPUT);

  strip.begin();
  strip.show(); // Initialize all pixels to 'off'
  strip.setBrightness(70);                // Set LED brightness 0-255

  //give the sensor some time to calibrate
  Serial.print("20 seconds to calibrate sensor ");
  for (int i = 0; i < calibrationTime; i++) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("Waiting for motion event");
  Serial.println(analogRead(lightPin));
  delay(25);
}

void loop() {

  if (analogRead(lightPin) < threshold) {
    if (digitalRead(pirPin) == HIGH) {
      if (lockLow) {  //makes sure we wait for a transition to LOW before any further output is made:
        lockLow = false;
        Serial.println("---");
        Serial.print("motion detected at ");
        Serial.print(millis() / 1000);
        Serial.println(" sec");
        Serial.println(analogRead(lightPin));
        FadeIn(0xff, 0x00, 0x00, 10); //Wait until Serial print work is done before activating strip
        delay(25);
      }
      takeLowTime = true;
    }
  }

  if (digitalRead(pirPin) == LOW) {
    if (takeLowTime) {
      lowIn = millis();          //save the time of the transition from high to LOW make sure this is only done at the start of a LOW phase
      takeLowTime = false;
    }
    if (!lockLow && millis() - lowIn > pause) { //if the sensor is low for more than the given pause,we assume that no more motion is going to happen
      //makes sure this block of code is only executed again after a new motion sequence has been detected
      lockLow = true;
      Serial.println("motion ended at ");      //output
      Serial.print((millis() - pause) / 1000);
      Serial.println(" sec");
      Serial.println(analogRead(lightPin));
      FadeOut(0xff, 0x00, 0x00, 10);
      delay(25);
    }
  }
}

void FadeIn(byte red, byte green, byte blue, uint8_t wait) {
  float r, g, b;

  for (int k = 0; k < 256; k = k + 1) {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setAll(r, g, b);
    strip.show();
    delay(wait);
  }
}

void FadeOut(byte red, byte green, byte blue, uint8_t wait) {
  float r, g, b;

  for (int k = 255; k >= 0; k = k - 2) {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setAll(r, g, b);
    strip.show();
    delay(wait);
  }
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
  strip.setPixelColor(Pixel, strip.Color(red, green, blue));
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  strip.show();
}
