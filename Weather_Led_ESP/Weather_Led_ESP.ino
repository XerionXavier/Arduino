/* -------------------------------------------------------------------------
FAIR WEATHER FRIEND -- a migraine headache forecaster using the Adafruit
HUZZAH ESP8266 WiFi microcontroller (can also work on other ESP8266 boards)
with forecasts provided by AccuWeather.com.  LED on pin 15 shows status:

  Steady on      = One-time initialization
  Fast flicker   = Connecting to network, polling data
  Slow blink     = Symptoms predicted in forecast (24-48 hrs)
  Blip ea. 4 sec = Symptoms not in forecast (sleeping)

Adafruit invests time and resources providing this open source code,
please support open-source hardware by purchasing products from Adafruit!

----------------------------------------------------------------------------
Configure the code below with your WiFi credentials.  Then visit
www.accuweather.com and look up the migraine (or other!) forecast for your
location.  Copy the URL into the appropriate spot in the code below.
They have other forecasts that are less grim -- hiking, golf weather, etc.
When changing the forecast type, you'll need to dig through the page's HTML
source to find a string that uniquely identifies the condition sought, while
avoiding false positives.  This code just uses string matches and is not
sophisticated in that regard.  See additional notes later in the code.

DISCLAIMER: THIS IS NOT A MEDICAL DIAGNOSTIC OR TREATMENT TOOL.
----------------------------------------------------------------------------*/

#include <ESP8266WiFi.h>

#define LED_PIN       1        // LED+ is connected here
#define POLL_INTERVAL (15 * 60) // Time between server queries (seconds)
#define FAIL_INTERVAL 30        // If error, time before reconnect (seconds)
#define READ_TIMEOUT  10000L    // Client read timeout, milliseconds

char ssid[] = "blank",
     pass[] = "1234567890",
     host[] = "www.accuweather.com",
     page[] = "/en/us/seattle-wa/98104/migraine-weather/351409";

// This structure is used during string-matching operations.  Only the
// 'string' element is initialized here; other elements are initialized
// or modified as needed in multiFind().  This code is NOT AVR-friendly;
// PROGMEM strings are not used, it's assumed this will be running on
// an ESP8266 (or ported to other non-AVR board that just normally puts
// const strings in program memory instead of RAM).
struct stringMatch {
  const char * const string;
  uint8_t            stringLength;
  uint8_t            matchedLength;
} matchList0[] = {
  { "<h3>Today</h3>"    },
  { "<h3>Tomorrow</h3>" },
  { NULL                }  // END OF LIST, don't remove this
}; // Can create add'l string match lists here if needed

WiFiClient client;

void setup(void) {
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, HIGH); // Steady on = startup
  Serial.begin(9600);
  Serial.println("Hello!");
}

// STRING-MATCH FUNCTION -----------------------------------------------------

// multiFind() scans a connected Client object for one or more strings
// (NULL-terminated stringMatch array 'list'), returns index of the first
// string matched (0 to n-1) or -1 if timeout or no match.
static int8_t multiFind(struct stringMatch *list) {
  uint32_t t;
  char     c;
  uint8_t  i;

  // Reset all stringMatch items prior to search...
  for(i=0; list[i].string; i++) {
    list[i].stringLength  = strlen(list[i].string);
    list[i].matchedLength = 0;
  }

  for(t=millis();;) {
    if(client.available()) {          // Data pending from Client?
      c = client.read();              // Read it
      if(c == 0) break;               // End of data reached, no match
      for(i=0; list[i].string; i++) { // Compare against each stringMatch item...
        if(c == list[i].string[list[i].matchedLength]) { // Matched another byte?
          if(++list[i].matchedLength ==                  // Matched whole string?
               list[i].stringLength) return i;           // WINNER, return index
        } else { // Character mismatch, reset counter to start
          list[i].matchedLength = 0;
        }
      }
      t = millis(); // Reset timeout
    } else if((millis() - t) > READ_TIMEOUT) {
      Serial.println("Timeout");
      break;
    }
  }
  return -1; // No string match, or timeout
}

void loop() {

  uint32_t t, hi, lo, pauseTime = FAIL_INTERVAL;

  // Fast blink during WiFi connection...
  analogWriteFreq(4);        // 4 Hz
  analogWrite(LED_PIN, 100); // ~10% duty cycle

  Serial.print("WiFi connecting..");
  WiFi.begin(ssid, pass);
  while(WiFi.status() != WL_CONNECTED) {
    Serial.write('.');
    delay(500);
  }
  Serial.println("OK!");

  // Slightly slower (but still quick) blink while searching
  analogWriteFreq(1);        // 1 Hz
  analogWrite(LED_PIN, 100); // ~10% dury cycle

  Serial.print("Contacting server...");
  if(client.connect(host, 80)) {
    Serial.print(F("OK\r\nRequesting data..."));
    client.print("GET ");
    client.print(page);
    client.print(" HTTP/1.1\r\nHost: ");
    client.print(host);
    client.print("\r\nConnection: Close\r\n\r\n");

    // multiFind() searches the incoming stream for a list of possible
    // string matches, returning the index of the found item (or -1 if
    // no match).  Stream position will be immediately after the found
    // item (allowing further searches to be performed from that point
    // forward), or end of stream in -1 case.
    // client.find() is the normal Arduino Stream search function, which
    // looks for a single item.  In this code, we're using multiFind()
    // to skip past some of AccuWeather's false positives, to pick a
    // starting point for a simple string search that more reliably
    // indicates migraine weather in the forecast...
    if((multiFind(matchList0) >= 0) &&
       client.find("Migraine Headache <span>Weather")) {
      // Found it -- migraine weather in next 24-28 hrs.
      Serial.println(F("FOUND"));
      hi = lo = 500; // 1 Hz, 50% duty cycle
    } else { // No match
      Serial.println(F("not found"));
      hi =   10; // Tiny blip 
      lo = 3990; // at about 1/4 Hz
    }
    // This is just one example...more complex code might need multiple
    // find() and/or multiSearch() calls with different lists as a sort
    // of decision tree.

    Serial.println("Closing server connection.");
    client.stop();
    pauseTime = POLL_INTERVAL;
  } else {
    Serial.println("failed.");
  }
  // WiFi is turned off between server queries, to save a little power if
  // you decide to make this battery-operated.
  Serial.println("Stopping WiFi.");
  WiFi.disconnect();
  analogWrite(LED_PIN, 0);

  // Delay until next server query time. The values of 'hi' and 'lo'
  // determine the LED blink speed. This code doesn't use any low-power
  // sleep techniques, as the ESP8266 doesn't appear to support PWM while
  // sleeping...it has to be blinked in software.
  Serial.print("Pausing for ");
  Serial.print(pauseTime);
  Serial.println(" seconds.");
  t = millis();
  while((millis() - t) < (pauseTime * 1000)) {
    digitalWrite(LED_PIN, HIGH);
    delay(hi);
    digitalWrite(LED_PIN, LOW);
    delay(lo);
  }
}
