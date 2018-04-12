#include <Adafruit_NeoPixel.h>

Adafruit_NeoPixel strip = Adafruit_NeoPixel(60, 6, NEO_GRB + NEO_KHZ800);

//the time we give the sensor to calibrate (10-60 secs according to the datasheet)
int calibrationTime = 20;

//the time when the sensor outputs a low impulse
long unsigned int lowIn;

//the amount of milliseconds the sensor has to be low
//before we assume all motion has stopped
long unsigned int pause = 30000;

boolean lockLow = true;
boolean takeLowTime;

int pirPin = 2;
int ledPin = 6;
int lightPin = 0;
int threshold = 120;

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
  strip.setBrightness(80);                // Set LED brightness 0-255

  //give the sensor some time to calibrate
  Serial.print("calibrating sensor ");
  for (int i = 0; i < calibrationTime; i++) {
    Serial.print(".");
    delay(1000);
  }
  Serial.println(" done");
  Serial.println("SENSOR ACTIVE");
  Serial.println(analogRead(lightPin));
  delay(50);
}

////////////////////////////
//LOOP
void loop() {

  if (analogRead(lightPin) < threshold && (digitalRead(pirPin) == HIGH)) {
    if (lockLow) {
      //makes sure we wait for a transition to LOW before any further output is made:
      lockLow = false;
      Serial.println("---");
      Serial.print("motion detected at ");
      Serial.print(millis() / 1000);
      Serial.println(" sec");
      Serial.println(analogRead(lightPin));
      ColorWipe(strip.Color(255, 0, 0), 80); // Red
      delay(50);
    }
    takeLowTime = true;
  }

  if (digitalRead(pirPin) == LOW) {
    if (takeLowTime) {
      lowIn = millis();          //save the time of the transition from high to LOW
      takeLowTime = false;       //make sure this is only done at the start of a LOW phase
    }
    //if the sensor is low for more than the given pause,
    //we assume that no more motion is going to happen
    if (!lockLow && millis() - lowIn > pause) {
      //makes sure this block of code is only executed again after
      //a new motion sequence has been detected
      lockLow = true;
      Serial.print("motion ended at ");      //output
      Serial.print((millis() - pause) / 1000);
      Serial.println(" sec");
      Serial.println(analogRead(lightPin));
      ReverseColorWipe(strip.Color(0, 0, 0), 100); // Black
      delay(50);
    }
  }
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
