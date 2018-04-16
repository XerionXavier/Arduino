// FastLED setup ----------   FastLED has to be declared BEFORE the Webserver     ---------------------
#include "FastLED.h"
FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN      0     // for Huzzah: Pins w/o special function:  #4, #5, #12, #13, #14; // #16 does not work :( 
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define NUM_LEDS      60
CRGB leds[NUM_LEDS];

bool gReverseDirection = false;
static uint16_t dist;         // A random number for our noise generator.
uint16_t scale = 30;          // Wouldn't recommend changing this on the fly, or the animation will be really blocky.
uint8_t maxChanges = 48;      // Value for blending between palettes.

uint8_t frequency = 100;                                       // controls the interval between strikes
uint8_t flashes = 6;                                          //the upper limit of flashes per strike
unsigned int dimmer = 1;

uint8_t ledstart;                                             // Starting location of a flash
uint8_t ledlen;

CRGBPalette16 currentPalette(CRGB::Black);
CRGBPalette16 targetPalette(CloudColors_p); //pasted in from noise fastled end

//#define BRIGHTNESS       160
int BRIGHTNESS =           160;   // this is half brightness
int new_BRIGHTNESS =       160;   // shall be initially the same as brightness

#define MILLI_AMPERE      1800    // IMPORTANT: set here the max milli-Amps of your power supply 5V 2A = 2000
#define FRAMES_PER_SECOND  120    // here you can control the speed. With the Access Point / Web Server the 
// animations run a bit slower.

int ledMode = 14;                  // this is the starting animation

// Fire2012 by Mark Kriegsman, July 2012
// as part of "Five Elements" shown here: http://youtu.be/knWiGsmgycY
////
// This basic one-dimensional 'fire' simulation works roughly as follows:
// There's a underlying array of 'heat' cells, that model the temperature
// at each point along the line.  Every cycle through the simulation,
// four steps are performed:
//  1) All cells cool down a little bit, losing heat to the air
//  2) The heat from each cell drifts 'up' and diffuses a little
//  3) Sometimes randomly new 'sparks' of heat are added at the bottom
//  4) The heat from each cell is rendered as a color into the leds array
//     The heat-to-color mapping uses a black-body radiation approximation.
//
// Temperature is in arbitrary units from 0 (cold black) to 255 (white hot).
//
// This simulation scales it self a bit depending on NUM_LEDS; it should look
// "OK" on anywhere from 20 to 100 LEDs without too much tweaking.
//
// I recommend running this simulation at anywhere from 30-100 frames per second,
// meaning an interframe delay of about 10-35 milliseconds.
//
// Looks best on a high-density LED setup (60+ pixels/meter).
//
//
// There are two main parameters you can play with to control the look and
// feel of your fire: COOLING (used in step 1 above), and SPARKING (used
// in step 3 above).
//
// COOLING: How much does the air cool as it rises?
// Less cooling = taller flames.  More cooling = shorter flames.
// Default 50, suggested range 20-100
#define COOLING  90

// SPARKING: What chance (out of 255) is there that a new spark will be lit?
// Higher chance = more roaring fire.  Lower chance = more flickery fire.
// Default 120, suggested range 50-200.
#define SPARKING 100


// Select EITHER ACCESS-Point  OR  WEB SERVER setup

// ACCESS-Point setup ------------------------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>

const char* ssid = "LedMe";
const char* password = "1234567890";  // set to "" for open access point w/o password; or any other pw (min length = 8 characters)

unsigned long ulReqcount;

// Create an instance of the server on Port 80
WiFiServer server(80);
//IPAddress apIP(192, 168, 10, 1);                                        // if you want to configure another IP address
void setup()
{
  // setup globals
  ulReqcount = 0;

  // AP mode
  WiFi.mode(WIFI_AP);
  //  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));          // if you want to configure another IP address
  WiFi.softAP(ssid, password);
  server.begin();
  // end ACCESS-Point setup ---------------------------------------------------------------------------------------------------

 /* 
    // WEB SERVER setup ---------------------------------------------------------------------------------------------------------
    #include <ESP8266WiFi.h>
    // comes with Huzzah installation. Enter in Arduino settings:
    // http://arduino.esp8266.com/package_esp8266com_index.json

    const char* ssid = "WiFi_24";
    const char* password = "Cft6vgy7";

    // NETWORK: Static IP details...
    IPAddress ip(192, 168, 1, 222);
    IPAddress gateway(192, 168, 1, 1);
    IPAddress subnet(255, 255, 255, 0);

    unsigned long ulReqcount;
    unsigned long ulReconncount;

    WiFiServer server(80);  // Create an instance of the server on Port 80

    void setup()
    {

    ulReqcount = 0;       // setup globals for Webserver
    ulReconncount = 0;

    // prepare GPIO2      // not necessary for FastLED
    pinMode(2, OUTPUT);
    digitalWrite(2, 0);

    // start serial
    Serial.begin(9600);
    delay(1);

    // inital connect
    WiFi.mode(WIFI_STA);
    
    WiFiStart();
    // end WEB SERVER setup -----------------------------------------------------------------------------------------------------
*/

  // now the settings for FastLED
  delay(1000);          // sanity delay for LEDs
  FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(DirectSunlight);         // for WS2812 (Neopixel)
  FastLED.setBrightness(BRIGHTNESS);
  dist = random16(12345);          // A semi-random number for our noise generator PASTED from nouse fastled
  set_max_power_in_volts_and_milliamps(5, MILLI_AMPERE);
}

uint8_t gHue = 0; // rotating "base color" used by many of the patterns


void WiFiStart()
{

  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);
  // Static IP Setup Info Here...
  //WiFi.config(ip, gateway, subnet);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  // Start the server
  server.begin();
  Serial.println("Server started");

  // Print the IP address
  Serial.println(WiFi.localIP());
}

void loop() {
  webserver();

  if (ledMode != 999) {

    switch (ledMode) {
      case  1: all_off(); break;
      case  2: rainbow(); break;
      case  3: rainbowWithGlitter(); break;
      case  4: confetti(); break;
      case  5: fire2012(); break;
      case  6: juggle(); break;
      case  7: bpm(); break;
      case  8: justred(); break;
      case  9: justgreen(); break;
      case  10: justblue(); break;
      case  11: justpurple(); break;
      case  12: justwhite(); break;
      case  13: fillnoise8(); break;
      case  14: noise16_1(); break;
      case  15: noise16_2(); break;
      case  16: noise16_3(); break;
      case  17: lightning(); break;
      case  18: blur(); break;
      case  19: meteor(0xff,0xbf,0x00,5, 85, true, 35); break;
      case  20: rainbowCycle(100); break;
    }
  }
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);

  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;
  } // slowly cycle the "base color" through the rainbow

  EVERY_N_MILLISECONDS(10) {
    nblendPaletteTowardPalette(currentPalette, targetPalette, maxChanges);  // Blend towards the target palette
    //fillnoise8();                                                           // Update the LED array with noise at the new location
  }

  EVERY_N_SECONDS(5) {             // Change the target palette to a random one every 5 seconds.
    targetPalette = CRGBPalette16(CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)), CHSV(random8(), 192, random8(128, 255)), CHSV(random8(), 255, random8(128, 255)));
  }

} // end of loop ************************************************************************************************************


void webserver() {   /// complete web server (same for access point) ////////////////////////////////////////////////////////
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client)
  {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis() + 250;
  while (!client.available() && (millis() < ultimeout) )
  {
    delay(1);
  }
  if (millis() > ultimeout)
  {
    Serial.println("client connection time-out!");
    return;
  }

  // Read the first line of the request

  String sRequest = client.readStringUntil('\r');
  Serial.println(sRequest);
  client.flush();

  // stop client, if request is empty
  if (sRequest == "")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }

  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath = "", sParam = "", sCmd = "";
  String sGetstart = "GET ";
  int iStart, iEndSpace, iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  //  Serial.print("iStart ");
  //  Serial.println(iStart);

  if (iStart >= 0)
  {
    iStart += +sGetstart.length();
    //  Serial.print("iStart + sGetstart ");
    //  Serial.println(iStart);
    iEndSpace = sRequest.indexOf(" ", iStart);
    //  Serial.print("iEndSpace ");
    //  Serial.println(iEndSpace);
    iEndQuest = sRequest.indexOf("?", iStart);
    //  Serial.print("iEndQuest ");
    //  Serial.println(iEndQuest);
    // are there parameters?
    if (iEndSpace > 0)
    {
      if (iEndQuest > 0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart, iEndQuest);
        sParam = sRequest.substring(iEndQuest, iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart, iEndSpace);
      }
    }
  }

  //////////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it //
  //////////////////////////////////////////////////////////////////////////////////
  if (sParam.length() > 0)
  {
    int iEqu = sParam.indexOf("=");
    if (iEqu >= 0)
    {
      sCmd = sParam.substring(iEqu + 1, sParam.length());
      Serial.print("We are in output Parameters, value is: ");
      Serial.println(sCmd);
      char carray[4];                                // values 0..255 = 3 digits; array = digits + 1
      sCmd.toCharArray(carray, sizeof(carray));      // convert char to the array
      new_BRIGHTNESS = atoi(carray);                 // atoi() converts an ascii character array to an integer
      if (new_BRIGHTNESS == 0) {
        new_BRIGHTNESS = BRIGHTNESS;  // if something else is selected (no change in brightness)
      }
      BRIGHTNESS = new_BRIGHTNESS;                 // works not this way
      FastLED.setBrightness(new_BRIGHTNESS);      // that's how the new value is assigned
      Serial.print("new Brightness: ");
      Serial.println(new_BRIGHTNESS);
    }
  }

  //////////////////////////////
  // format the html response //
  //////////////////////////////
  String sResponse, sHeader;

  ///////////////////////////////
  // 404 for non-matching path //
  ///////////////////////////////
  if (sPath != "/")
  {
    sResponse = "<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";

    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  //////////////////////////
  // format the html page //
  //////////////////////////
  else
  {
    ulReqcount++;
    sResponse  = "<html><head><title>LED Me</title>";
    sResponse += "<style type=\"text/css\">";
    sResponse += ".form-control {";
    sResponse += " width:55px;";
    sResponse += "}";
    sResponse += "</style>";
    sResponse += "</head><body>";
    sResponse += "<link rel='stylesheet' href='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/css/bootstrap.min.css'";
    sResponse += "<script src='https://ajax.googleapis.com/ajax/libs/jquery/3.1.1/jquery.min.js'></script>";
    sResponse += "<script src='https://maxcdn.bootstrapcdn.com/bootstrap/3.3.7/js/bootstrap.min.js'></script>";
    sResponse += "<FONT SIZE=-1>";
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<div class=\"container\">";
    sResponse += "<div class=\"well\">";
    sResponse += "<h1 class=\"text-muted\">Led Me</h1><br>";

    //  This is a nice drop down menu
    sResponse += "<FONT SIZE=+2>";
    sResponse += "<form class=\"form-inline\">";
    //sResponse += "<div class=\"form-group col-md-2\">";
    sResponse += "<select class=\"form-control form-control-sm\" name=\"sCmd\" size=\"5\">";
    sResponse += "<option value=\"FUNCTION1OFF\"selected>All Off</option>";
    sResponse += "<option value=\"FUNCTION1ON\">Rainbow</option>";
    sResponse += "<option value=\"FUNCTION2ON\">Rainbow Glitter</option>";
    sResponse += "<option value=\"FUNCTION3ON\">Confetti</option>";
    sResponse += "<option value=\"FUNCTION4ON\">Fire2012</option>";
    sResponse += "<option value=\"FUNCTION5ON\">Juggle</option>";
    sResponse += "<option value=\"FUNCTION6ON\">BPM</option><br>";
    sResponse += "<option value=\"FUNCTION7ON\">JustRed</option><br>";
    sResponse += "<option value=\"FUNCTION8ON\">JustGreen</option><br>";
    sResponse += "<option value=\"FUNCTION9ON\">JustBlue</option><br>";
    sResponse += "<option value=\"FUNCTION10ON\">JustPurple</option><br>";
    sResponse += "<option value=\"FUNCTION11ON\">JustWhite</option><br>";
    sResponse += "<option value=\"FUNCTION12ON\">Fillnoise8</option><br>";
    sResponse += "<option value=\"FUNCTION13ON\">Noise16_1</option><br>";
    sResponse += "<option value=\"FUNCTION14ON\">Noise16_2</option><br>";
    sResponse += "<option value=\"FUNCTION15ON\">Noise16_3</option><br>";
    sResponse += "<option value=\"FUNCTION16ON\">Lightning</option><br>";
    sResponse += "<option value=\"FUNCTION17ON\">Blur</option><br>";
    sResponse += "<option value=\"FUNCTION18ON\">Meteor</option><br>";
    sResponse += "<option value=\"FUNCTION19ON\">RainbowCycle</option><br>";
    sResponse += "</select>";
    sResponse += "<br><br>";
    sResponse += "<button type=\"submit\" class=\"btn btn-primary\">Select</button>";
    sResponse += "</form>";
    sResponse += "<FONT SIZE=-1>";

    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
    // Slider          this works, however I got http://192.168.4.1/sCmd?FUNCTION_200=80  and the page was not found
    //                 I needed to take the FUNCTION_200=80 apart and call only FUNCTION_200 and assign
    //                 the value (=80) in "react on parameters" (line 512) to new_BRIGHTNESS

    sResponse += "</p>";
    sResponse += "<form action=\"?sCmd\" >";    // ?sCmd forced the '?' at the right spot
    sResponse += "<BR><p class=\"text-muted\">Brightness&nbsp;";  // perhaps we can show here the current value
    sResponse += round(new_BRIGHTNESS / 2.5);   // this is just a scale depending on the max value; round for better readability
    sResponse += " %</p>";
    //sResponse += "<BR>";
    sResponse += "<input style=\"width:200px; height:50px\" type=\"range\" name=\"=FUNCTION_200\" id=\"cmd\" value=\"";   // '=' in front of FUNCTION_200 forced the = at the right spot
    sResponse += BRIGHTNESS;
    sResponse += "\" min=10 max=250 step=10 onchange=\"showValue(points)\" />";
    //sResponse += "<BR>";
    sResponse += "<button type=\"submit\" class=\"btn btn-primary\">Select</button>";
    sResponse += "</form>";
    //sResponse += "<p>";
    // ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    sResponse += "<FONT SIZE=-1>";


    /////////////////////////
    // react on parameters //
    /////////////////////////
    if (sCmd.length() > 0)
    {
      // write received command to html page
      sResponse += "<p class=\"text-muted\">Command: " + sCmd + "</p><BR>";

      // switch the animiation (based on your choice in the case statement (main loop)
      if (sCmd.indexOf("FUNCTION1ON") >= 0)
      {
        Serial.println("1 ON");
        ledMode = 2;
      }
      else if (sCmd.indexOf("FUNCTION1OFF") >= 0)
      {
        Serial.println("1 OFF");
        ledMode = 1;
      }

      if (sCmd.indexOf("FUNCTION2ON") >= 0)
      {
        Serial.println("2 ON");
        ledMode = 3;
      }
      else if (sCmd.indexOf("FUNCTION2OFF") >= 0)
      {
        Serial.println("2 OFF");
        ledMode = 1;
      }

      if (sCmd.indexOf("FUNCTION3ON") >= 0)
      {
        Serial.println("3 ON");
        ledMode = 4;

      }
      else if (sCmd.indexOf("FUNCTION3OFF") >= 0)
      {
        Serial.println("3 OFF");
        ledMode = 1;

      }
      if (sCmd.indexOf("FUNCTION4ON") >= 0)
      {
        Serial.println("4 ON");
        ledMode = 5;

      }
      else if (sCmd.indexOf("FUNCTION4OFF") >= 0)
      {
        Serial.println("4 OFF");
        ledMode = 1;

      }
      if (sCmd.indexOf("FUNCTION5ON") >= 0)
      {
        Serial.println("5 ON");
        ledMode = 6;

      }
      else if (sCmd.indexOf("FUNCTION5OFF") >= 0)
      {
        Serial.println("5 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION6ON") >= 0)
      {
        Serial.println("6 ON");
        ledMode = 7;

      }
      else if (sCmd.indexOf("FUNCTION6OFF") >= 0)
      {
        Serial.println("6 OFF");
        ledMode = 1;

      }
      if (sCmd.indexOf("FUNCTION7ON") >= 0)
      {
        Serial.println("7 ON");
        ledMode = 8;

      }
      else if (sCmd.indexOf("FUNCTION7OFF") >= 0)
      {
        Serial.println("7 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION8ON") >= 0)
      {
        Serial.println("8 ON");
        ledMode = 9;

      }
      else if (sCmd.indexOf("FUNCTION8OFF") >= 0)
      {
        Serial.println("8 OFF");
        ledMode = 1;

      }
      if (sCmd.indexOf("FUNCTION9ON") >= 0)
      {
        Serial.println("9 ON");
        ledMode = 10;

      }
      else if (sCmd.indexOf("FUNCTION9OFF") >= 0)
      {
        Serial.println("9 OFF");
        ledMode = 1;

      }
      if (sCmd.indexOf("FUNCTION10ON") >= 0)
      {
        Serial.println("10 ON");
        ledMode = 11;

      }
      else if (sCmd.indexOf("FUNCTION10OFF") >= 0)
      {
        Serial.println("10 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION11ON") >= 0)
      {
        Serial.println("11 ON");
        ledMode = 12;

      }
      else if (sCmd.indexOf("FUNCTION11OFF") >= 0)
      {
        Serial.println("11 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION12ON") >= 0)
      {
        Serial.println("12 ON");
        ledMode = 13;

      }
      else if (sCmd.indexOf("FUNCTION12OFF") >= 0)
      {
        Serial.println("12 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION13ON") >= 0)
      {
        Serial.println("13 ON");
        ledMode = 14;

      }
      else if (sCmd.indexOf("FUNCTION13OFF") >= 0)
      {
        Serial.println("13 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION14ON") >= 0)
      {
        Serial.println("14 ON");
        ledMode = 15;

      }
      else if (sCmd.indexOf("FUNCTION14OFF") >= 0)
      {
        Serial.println("14 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION15ON") >= 0)
      {
        Serial.println("15 ON");
        ledMode = 16;

      }
      else if (sCmd.indexOf("FUNCTION15OFF") >= 0)
      {
        Serial.println("15 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION16ON") >= 0)
      {
        Serial.println("16 ON");
        ledMode = 17;

      }
      else if (sCmd.indexOf("FUNCTION16OFF") >= 0)
      {
        Serial.println("16 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION17ON") >= 0)
      {
        Serial.println("17 ON");
        ledMode = 18;

      }
      else if (sCmd.indexOf("FUNCTION17OFF") >= 0)
      {
        Serial.println("17 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION18ON") >= 0)
      {
        Serial.println("18 ON");
        ledMode = 19;

      }
      else if (sCmd.indexOf("FUNCTION18OFF") >= 0)
      {
        Serial.println("18 OFF");
        ledMode = 1;

      }

      if (sCmd.indexOf("FUNCTION19ON") >= 0)
      {
        Serial.println("19 ON");
        ledMode = 20;

      }
      else if (sCmd.indexOf("FUNCTION19OFF") >= 0)
      {
        Serial.println("19 OFF");
        ledMode = 1;

      }

      // to BRIGHTNESS control

      if (sCmd.indexOf("FUNCTION_200=20") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=30") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=40") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=50") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=60") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=70") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=80") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=90") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=100") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=110") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=120") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=130") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=140") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=150") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=160") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=170") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=180") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=190") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=200") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=210") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=220") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=230") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=240") >= 0) { }
      if (sCmd.indexOf("FUNCTION_200=250") >= 0) { }
      //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    } // end sCmd.length()>0

    sResponse += "</div>";
    sResponse += "</div>";
    sResponse += "</div>";
    sResponse += "</body></html>";
    sHeader  = "HTTP/1.1 200 OK\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }

  // Send the response to the client
  client.print(sHeader);
  client.print(sResponse);


  // and stop the client
  client.stop();
  Serial.println("Client disonnected");
}  // end of web server

/// END of complete web server //////////////////////////////////////////////////////////////////

// LED animations ###############################################################################

void showStrip() {
   FastLED.show();
}

void setPixel(int Pixel, byte red, byte green, byte blue) {
   leds[Pixel].r = red;
   leds[Pixel].g = green;
   leds[Pixel].b = blue;
}

void setAll(byte red, byte green, byte blue) {
  for(int i = 0; i < NUM_LEDS; i++ ) {
    setPixel(i, red, green, blue); 
  }
  showStrip();
}

void all_off() {
  fill_solid(leds, NUM_LEDS, CRGB::Black);
  //  show_at_max_brightness_for_power();
  //  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND);
  FastLED.show();
  FastLED.delay(1000 / FRAMES_PER_SECOND);
}

void rainbowCycle(int SpeedDelay) {
  byte *c;
  uint16_t i, j;
  for(j=0; j<256; j++) {
    c=Wheel(((i * 256 / NUM_LEDS) + j) & 255); // color selection used to be in the "i" loop
    for(i=0; i< NUM_LEDS; i++) {
      setPixel(i, *c, *(c+1), *(c+2));
    }
    showStrip();
    delay(SpeedDelay);
  }
}

byte * Wheel(byte WheelPos) {
  static byte c[3];
  if(WheelPos < 85) {
   c[0]=WheelPos * 3;
   c[1]=255 - WheelPos * 3;
   c[2]=0;
  } else if(WheelPos < 170) {
   WheelPos -= 85;
   c[0]=255 - WheelPos * 3;
   c[1]=0;
   c[2]=WheelPos * 3;
  } else {
   WheelPos -= 170;
   c[0]=0;
   c[1]=WheelPos * 3;
   c[2]=255 - WheelPos * 3;
  }
  return c;
}

void rainbow()
{
  // FastLED's built-in rainbow generator
  EVERY_N_MILLISECONDS( 20 ) {
    gHue++;  // slowly cycle the "base color" through the rainbow
  }
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);

}

void rainbowWithGlitter()
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter)
{
  if ( random8() < chanceOfGlitter) {
    leds[ random16(NUM_LEDS) ] += CRGB::White;
  }
}

void confetti()
{
  // random colored speckles that blink in and fade smoothly
  fadeToBlackBy( leds, NUM_LEDS, 10);
  int pos = random16(NUM_LEDS);
  leds[pos] += CHSV( gHue + random8(64), 200, 255);
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);
  //  FastLED.show();
  //  FastLED.delay(1000/FRAMES_PER_SECOND);
}

void fire2012()
{
  // Array of temperature readings at each simulation cell
  static byte heat[NUM_LEDS];

  // Step 1.  Cool down every cell a little
  for ( int i = 0; i < NUM_LEDS; i++) {
    heat[i] = qsub8( heat[i],  random8(0, ((COOLING * 10) / NUM_LEDS) + 2));
  }

  // Step 2.  Heat from each cell drifts 'up' and diffuses a little
  for ( int k = NUM_LEDS - 1; k >= 2; k--) {
    heat[k] = (heat[k - 1] + heat[k - 2] + heat[k - 2] ) / 3;
  }

  // Step 3.  Randomly ignite new 'sparks' of heat near the bottom
  if ( random8() < SPARKING ) {
    int y = random8(7);
    heat[y] = qadd8( heat[y], random8(160, 255) );
  }

  // Step 4.  Map from heat cells to LED colors
  for ( int j = 0; j < NUM_LEDS; j++) {
    CRGB color = HeatColor( heat[j]);
    int pixelnumber;
    if ( gReverseDirection ) {
      pixelnumber = (NUM_LEDS - 1) - j;
    } else {
      pixelnumber = j;
    }
    leds[pixelnumber] = color;
  }
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 92;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for ( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue + (i * 2), beat - gHue + (i * 10));
  }
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);

}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for ( int i = 0; i < 8; i++) {
    leds[beatsin16(i + 7, 0, NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);

}

void justred()
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Red); //Set all to red
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);
  //  FastLED.show();
  //  FastLED.delay(1000/FRAMES_PER_SECOND);
}

void justgreen()
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Green); //Set all to green
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);

}

void justblue()
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Blue); //Set all to blue
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);

}

void justpurple()
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Purple); //Set all to purple
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);

}

void justwhite()
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::White); //Set all to orange
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000 / FRAMES_PER_SECOND);

}

void fillnoise8() {
  for (int i = 0; i < NUM_LEDS; i++) {                                     // Just ONE loop to fill up the LED array as all of the pixels change.
    uint8_t index = inoise8(i * scale, dist + i * scale) % 255;            // Get a value from the noise function. I'm using both x and y axis.
    leds[i] = ColorFromPalette(currentPalette, index, 255, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }
  dist += beatsin8(10, 1, 4);                                              // Moving along the distance (that random number we started out with). Vary it a bit with a sine wave.
  // In some sketches, I've used millis() instead of an incremented counter. Works a treat.
} // fillnoise8()

void noise16_1() {                                            // moves a noise up and down while slowly shifting to the side

  uint16_t scale = 1000;                                      // the "zoom factor" for the noise
  CRGBPalette16 palette = OceanColors_p;

  for (uint16_t i = 0; i < NUM_LEDS; i++) {

    uint16_t shift_x = beatsin8(5);                           // the x position of the noise field swings @ 17 bpm
    uint16_t shift_y = millis() / 100;                        // the y position becomes slowly incremented


    uint16_t real_x = (i + shift_x) * scale;                  // the x position of the noise field swings @ 17 bpm
    uint16_t real_y = (i + shift_y) * scale;                  // the y position becomes slowly incremented
    uint32_t real_z = millis() * 20;                          // the z position becomes quickly incremented

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;   // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                         // map LED color based on noise data
    uint8_t bri   = noise;

    leds[i] = ColorFromPalette(currentPalette, index, bri, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }

} // noise16_1()

void noise16_2() {                                            // just moving along one axis = "lavalamp effect"

  uint8_t scale = 1000;                                       // the "zoom factor" for the noise
  CRGBPalette16 palette = ForestColors_p;


  for (uint16_t i = 0; i < NUM_LEDS; i++) {

    uint16_t shift_x = millis() / 10;                         // x as a function of time
    uint16_t shift_y = 0;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = 4223;

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 8;    // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                          // map led color based on noise data
    uint8_t bri   = noise;

    leds[i] = ColorFromPalette(currentPalette, index, bri, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.

  }

}// noise16_2()

void noise16_3() {                                            // no x/y shifting but scrolling along

  uint8_t scale = 1000;                                       // the "zoom factor" for the noise
  CRGBPalette16 palette = CloudColors_p;

  for (uint16_t i = 0; i < NUM_LEDS; i++) {

    uint16_t shift_x = 4223;                                  // no movement along x and y
    uint16_t shift_y = 1234;

    uint32_t real_x = (i + shift_x) * scale;                  // calculate the coordinates within the noise field
    uint32_t real_y = (i + shift_y) * scale;                  // based on the precalculated positions
    uint32_t real_z = millis() * 2;                           // increment z linear

    uint8_t noise = inoise16(real_x, real_y, real_z) >> 7;    // get the noise data and scale it down

    uint8_t index = sin8(noise * 3);                          // map led color based on noise data
    uint8_t bri   = noise;

    leds[i] = ColorFromPalette(currentPalette, index, bri, LINEARBLEND);   // With that value, look up the 8 bit colour palette value and assign it to the current LED.
  }

} // noise16_3()

void lightning() {

  ledstart = random8(NUM_LEDS);                               // Determine starting location of flash
  ledlen = random8(NUM_LEDS - ledstart);                      // Determine length of flash (not to go beyond NUM_LEDS-1)

  for (int flashCounter = 0; flashCounter < random8(3, flashes); flashCounter++) {
    if (flashCounter == 0) dimmer = 5;                        // the brightness of the leader is scaled down by a factor of 5
    else dimmer = random8(1, 3);                              // return strokes are brighter than the leader

    fill_solid(leds, NUM_LEDS, CRGB::Black);

    fill_solid(leds + ledstart, ledlen, CHSV(255, 0, 255 / dimmer));

    FastLED.show();                       // Show a section of LED's
    delay(random8(4, 10));                                    // each flash only lasts 4-10 milliseconds
    fill_solid(leds + ledstart, ledlen, CHSV(255, 0, 0));     // Clear the section of LED's
    FastLED.show();

    if (flashCounter == 0) delay (200);                       // longer delay until next flash after the leader

    delay(50 + random8(100));                                 // shorter delay between strokes
  } // for()

  delay(random8(frequency) * 100);                            // delay between strikes

} // lightning()

void blur() {

  uint8_t blurAmount = dim8_raw( beatsin8(3, 64, 192) );      // A sinewave at 3 Hz with values ranging from 64 to 192.
  blur1d( leds, NUM_LEDS, blurAmount);                        // Apply some blurring to whatever's already on the strip, which will eventually go black.

  uint8_t  i = beatsin8(  9, 0, NUM_LEDS);
  uint8_t  j = beatsin8( 7, 0, NUM_LEDS);
  uint8_t  k = beatsin8(  5, 0, NUM_LEDS);

  // The color of each point shifts over time, each at a different speed.
  uint16_t ms = millis();
  leds[(i + j) / 2] = CHSV( ms / 29, 200, 255);
  leds[(j + k) / 2] = CHSV( ms / 41, 200, 255);
  leds[(k + i) / 2] = CHSV( ms / 73, 200, 255);
  leds[(k + i + j) / 3] = CHSV( ms / 53, 200, 255);

  FastLED.show();

} // blur()

void meteor(byte red, byte green, byte blue, byte meteorSize, byte meteorTrailDecay, boolean meteorRandomDecay, int SpeedDelay) {  
  setAll(0,0,0);
  
  for(int i = 0; i < NUM_LEDS+NUM_LEDS; i++) {
    
    
    // fade brightness all LEDs one step
    for(int j=0; j<NUM_LEDS; j++) {
      if( (!meteorRandomDecay) || (random(10)>5) ) {
        fadeToBlack(j, meteorTrailDecay );        
      }
    }
    
    // draw meteor
    for(int j = 0; j < meteorSize; j++) {
      if( ( i-j <NUM_LEDS) && (i-j>=0) ) {
        setPixel(i-j, red, green, blue);
      } 
    }
   
    showStrip();
    delay(SpeedDelay);
  }
}

void fadeToBlack(int ledNo, byte fadeValue) {
 #ifdef ADAFRUIT_NEOPIXEL_H 
    // NeoPixel
    uint32_t oldColor;
    uint8_t r, g, b;
    int value;
    
    oldColor = strip.getPixelColor(ledNo);
    r = (oldColor & 0x00ff0000UL) >> 16;
    g = (oldColor & 0x0000ff00UL) >> 8;
    b = (oldColor & 0x000000ffUL);

    r=(r<=10)? 0 : (int) r-(r*fadeValue/256);
    g=(g<=10)? 0 : (int) g-(g*fadeValue/256);
    b=(b<=10)? 0 : (int) b-(b*fadeValue/256);
    
    strip.setPixelColor(ledNo, r,g,b);
 #endif
 #ifndef ADAFRUIT_NEOPIXEL_H
   // FastLED
   leds[ledNo].fadeToBlackBy( fadeValue );
 #endif  
}
