#include "FastLED.h"
#define NUM_LEDS 20
#define COLOR_ORDER GRB
CRGB leds[NUM_LEDS];

//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 20;

uint16_t scale = 30;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 60000;

boolean lockLow = true;
boolean takeLowTime;

const int pirPin = 0;  //Trinket PIR pin 0
const int ldrPin = 1; //ldr analog #1. Physical Trinket GPIO pin #2
const int PIN = 3; //Trinket pin 3
const int threshold = 5;

void setup()
{
  pinMode(pirPin, INPUT);
  pinMode(ldrPin, INPUT);
  pinMode(PIN, OUTPUT);
  analogRead(ldrPin);
  (pirPin, LOW);

  FastLED.addLeds<WS2812B, PIN, GRB>(leds, NUM_LEDS).setCorrection( TypicalLEDStrip );

  FastLED.show();
  FastLED.setBrightness(255);
#define MILLI_AMPERE      2000    // IMPORTANT: set here the max milli-Amps of your power supply 5V 2A = 2000
#define FRAMES_PER_SECOND  120    // here you can control the speed. With the Access Point / Web Server the 

  for (int i = 0; i < calibrationTime; i++) {
    delay(1000);
  }
}

void loop() {

  if (analogRead(ldrPin) < threshold) {
    if (digitalRead(pirPin) == HIGH) {
      if (lockLow) {
        //makes sure we wait for a transition to LOW before any further output is made:
        lockLow = false;
        FadeIn(0xff, 0x00, 0x00, 5); //Wait until Serial print work is done before activating strip
        delay(50);
      }
      takeLowTime = true;
    }
  }

  if (digitalRead(pirPin) == LOW) {
    if (takeLowTime) {
      lowIn = millis();      //save the time of the transition from high to LOW make sure this is only done at the start of a LOW phase
      takeLowTime = false;
    }
    //if the sensor is low for more than the given pause, we assume that no more motion is going to happen
    if (!lockLow && millis() - lowIn > pause) {
      //makes sure this block of code is only executed again after a new motion sequence has been detected
      lockLow = true;
      FadeOut(0xff, 0x00, 0x00, 5);
      delay(50);
    }
  }
}

void FadeIn(byte red, byte green, byte blue, uint8_t wait) {
  float r, g, b;

  for (int k = 0; k < 256; k += 1) {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setAll(r, g, b);
    showStrip();
    delay(wait);
  }
}

void FadeOut(byte red, byte green, byte blue, uint8_t wait) {
  float r, g, b;

  for (int k = 255; k >= 0; k -= 2) {
    r = (k / 256.0) * red;
    g = (k / 256.0) * green;
    b = (k / 256.0) * blue;
    setAll(r, g, b);
    showStrip();
    delay(wait);
  }
}

void showStrip() {
  FastLED.show();
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
  leds[Pixel].r = red;
  leds[Pixel].g = green;
  leds[Pixel].b = blue;
}

void setAll(byte red, byte green, byte blue) {
  for (int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue);
  }
  showStrip();
}
