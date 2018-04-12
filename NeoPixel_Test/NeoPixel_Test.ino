#include <Adafruit_NeoPixel.h>
#define LEDPIN 6 // connect the Data from the strip to this pin on the Arduino
#define NUMBER_PIXELS 60 // the number of pixels in your LED strip
Adafruit_NeoPixel strip = Adafruit_NeoPixel(NUMBER_PIXELS, LEDPIN, NEO_GRB + NEO_KHZ800);

int delayTime = 100;

void setup() {
  strip.begin(); // initialize the strip
}

void loop() {
  theaterChase(strip.Color(127, 0, 0), delayTime); // red
  theaterChase(strip.Color(0, 127, 0), delayTime); // green
  //theaterChase(strip.Color(0, 0, 127), delayTime); // blue
}

// Theatre-style crawling lights with a spacing of 3
void theaterChase(uint32_t color, uint8_t wait) {
  for (int j=0; j<10; j++) {  //do 10 cycles of chasing
    for (int q=0; q < 4; q++) {
      for (int i=0; i < strip.numPixels(); i=i+4) {
        strip.setPixelColor(q+i, color);    //turn every third pixel on
      }
      strip.show();
      delay(wait);
      for (int i=0; i < strip.numPixels(); i=i+3) {
        strip.setPixelColor(q+i, 0);        //turn every third pixel off
      }
    }
  }
}
