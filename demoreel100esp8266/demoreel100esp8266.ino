
// FastLED setup -----FastLED has to be declared BEFORE the Webserver-----
#include "FastLED.h"
FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN      0     // for Huzzah: Pins w/o special function:  #4, #5, #12, #13, #14; // #16 does not work :( //6
#define LED_TYPE      WS2811
#define COLOR_ORDER   GRB
#define NUM_LEDS      60     
CRGB leds[NUM_LEDS];

#define BRIGHTNESS          185
#define MILLI_AMPERE      1500
#define FRAMES_PER_SECOND  120

int ledMode = 2;            // this is the starting animation

// Websever setup -----------------------------------------------------
#include <ESP8266WiFi.h>
// comes with Huzzah installation. Enter in Arduino settings:
// http://arduino.esp8266.com/package_esp8266com_index.json

const char* ssid = "LED-EFFECTS";
const char* password = "";  // set to "" for open access point w/o password or any other pw

unsigned long ulReqcount;


// Create an instance of the server on Port 80
WiFiServer server(80);

void setup() 
{
  // setup globals
  ulReqcount=0; 
  
  // prepare GPIO2
  //pinMode(2, OUTPUT);
  //digitalWrite(2, 0);
  
  // start serial
  Serial.begin(9600);
  delay(1);
  
  // AP mode
  WiFi.mode(WIFI_AP);
  WiFi.softAP(ssid, password);
  server.begin();

  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(DirectSunlight);
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(DirectSunlight);
  FastLED.setBrightness(BRIGHTNESS);
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
      case  5: sinelon(); break;
      case  6: juggle(); break;
      case  7: bpm(); break;
      case  8: justred(); break;
      case  9: justgreen(); break;
      case  10: justblue(); break;
      case  11: justpurple(); break;
      case  12: justorange(); break;
      }
      }
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

} // end of loop *************************************************************************************

void webserver() {   /// complete web server /////////////////////////////////////////////////////////
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) 
  {
    return;
  }
  
  // Wait until the client sends some data
  Serial.println("new client");
  unsigned long ultimeout = millis()+250;
  while(!client.available() && (millis()<ultimeout) )
  {
    delay(1);
  }
  if(millis()>ultimeout) 
  { 
    Serial.println("client connection time-out!");
    return; 
  }
  
  // Read the first line of the request
  String sRequest = client.readStringUntil('\r');
  //Serial.println(sRequest);
  client.flush();
  
  // stop client, if request is empty
  if(sRequest=="")
  {
    Serial.println("empty request! - stopping client");
    client.stop();
    return;
  }
  
  // get path; end of path is either space or ?
  // Syntax is e.g. GET /?pin=MOTOR1STOP HTTP/1.1
  String sPath="",sParam="", sCmd="";
  String sGetstart="GET ";
  int iStart,iEndSpace,iEndQuest;
  iStart = sRequest.indexOf(sGetstart);
  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
    iEndSpace = sRequest.indexOf(" ",iStart);
    iEndQuest = sRequest.indexOf("?",iStart);
    
    // are there parameters?
    if(iEndSpace>0)
    {
      if(iEndQuest>0)
      {
        // there are parameters
        sPath  = sRequest.substring(iStart,iEndQuest);
        sParam = sRequest.substring(iEndQuest,iEndSpace);
      }
      else
      {
        // NO parameters
        sPath  = sRequest.substring(iStart,iEndSpace);
      }
    }
  }
  
  ///////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it
  ///////////////////////////////////////////////////////////////////////////////
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.println(sCmd);
    }
  }
  
  
  ///////////////////////////
  // format the html response
  ///////////////////////////
  String sResponse,sHeader;
  
  ////////////////////////////
  // 404 for non-matching path
  ////////////////////////////
  if(sPath!="/")
  {
    sResponse="<html><head><title>404 Not Found</title></head><body><h1>Not Found</h1><p>The requested URL was not found on this server.</p></body></html>";
    
    sHeader  = "HTTP/1.1 404 Not found\r\n";
    sHeader += "Content-Length: ";
    sHeader += sResponse.length();
    sHeader += "\r\n";
    sHeader += "Content-Type: text/html\r\n";
    sHeader += "Connection: close\r\n";
    sHeader += "\r\n";
  }
  ///////////////////////
  // format the html page
  ///////////////////////
  else
  {
    ulReqcount++;
    sResponse  = "<html><head><title>LED Effects</title></head><body>";
    sResponse += "<font color=\"#FFFFF0\"><body bgcolor=\"#000000\">";  // first is background, second is font color
    sResponse += "<FONT SIZE=-1>"; 
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>ESP8266 access point<br>";
    sResponse += " for DemoReel100</h1>";

/*  this creates a list with ON / OFF buttons 
    // </a>&nbsp is a non-breaking space; moves next character over
    sResponse += "<p>Rainbow &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION1ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION1OFF\"><button>--OFF--</button></a><br>";
    sResponse += "<p>Rainbow Glitter<a href=\"?pin=FUNCTION2ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION2OFF\"><button>--OFF--</button></a><br>";
    sResponse += "<p>Confetti &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION3ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION3OFF\"><button>--OFF--</button></a><br>";
    sResponse += "<p>Sinelon &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION4ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION4OFF\"><button>--OFF--</button></a><br>";
    sResponse += "<p>Juggle&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION5ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION5OFF\"><button>--OFF--</button></a></p>";
    sResponse += "<p>BPM&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION6ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION6OFF\"><button>--OFF--</button></a></p>";
    sResponse += "<p>Function 7&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION7ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION7OFF\"><button>--OFF--</button></a></p><br>";
*/
//  This is a nice drop down menue
    sResponse += "<FONT SIZE=+1>";
    sResponse += "<form>";
    sResponse += "Select Animation<br>";
    sResponse += "<select name=\"sCmd\" size=\"15\" >";
    sResponse += "<option value=\"FUNCTION1OFF\"selected>All OFF</option>";
    sResponse += "<option value=\"FUNCTION1ON\">Rainbow</option>";
    sResponse += "<option value=\"FUNCTION2ON\">Rainbow Glitter</option>";
    sResponse += "<option value=\"FUNCTION3ON\">Confetti</option>";
    sResponse += "<option value=\"FUNCTION4ON\">Sinelon</option>";
    sResponse += "<option value=\"FUNCTION5ON\">Juggle</option>";
    sResponse += "<option value=\"FUNCTION6ON\">BPM</option><br>";
    sResponse += "<option value=\"FUNCTION7ON\">JustRed</option><br>";
    sResponse += "<option value=\"FUNCTION8ON\">JustGreen</option><br>";
    sResponse += "<option value=\"FUNCTION9ON\">JustBlue</option><br>";
    sResponse += "<option value=\"FUNCTION10ON\">JustPurple</option><br>";
    sResponse += "<option value=\"FUNCTION11ON\">JustOrange</option><br>";
    sResponse += "</select>";
    sResponse += "<br><br>";
    sResponse += "<input type= submit>";
    sResponse += "</form>";
    sResponse += "<FONT SIZE=-1>";

    //////////////////////
    // react on parameters
    //////////////////////
    if (sCmd.length()>0)
    {
      // write received command to html page
      sResponse += "Command: " + sCmd + "<BR>";
      
      // switch GPIO
      if(sCmd.indexOf("FUNCTION1ON")>=0)
      {
        Serial.println("1 ON");
        ledMode = 2;
      }
      else if(sCmd.indexOf("FUNCTION1OFF")>=0)
      {
        Serial.println("1 OFF");
        ledMode = 1;
      }

      if(sCmd.indexOf("FUNCTION2ON")>=0)
      {
         Serial.println("2 ON");
        ledMode = 3;
      }
      else if(sCmd.indexOf("FUNCTION2OFF")>=0)
      {
        Serial.println("2 OFF");
        ledMode = 1;
      }

      if(sCmd.indexOf("FUNCTION3ON")>=0)
      {
         Serial.println("3 ON");
        ledMode = 4;

      }
      else if(sCmd.indexOf("FUNCTION3OFF")>=0)
      {
        Serial.println("3 OFF");
        ledMode = 1;

      }
      if(sCmd.indexOf("FUNCTION4ON")>=0)
      {
        Serial.println("4 ON");
        ledMode = 5;

      }
      else if(sCmd.indexOf("FUNCTION4OFF")>=0)
      {
        Serial.println("4 OFF");
        ledMode = 1;

      }
      if(sCmd.indexOf("FUNCTION5ON")>=0)
      {
         Serial.println("5 ON");
        ledMode = 6;

      }
      else if(sCmd.indexOf("FUNCTION5OFF")>=0)
      {
        Serial.println("5 OFF");
        ledMode = 1;

      }

      if(sCmd.indexOf("FUNCTION6ON")>=0)
      {
         Serial.println("6 ON");
        ledMode = 7;

      }
      else if(sCmd.indexOf("FUNCTION6OFF")>=0)
      {
        Serial.println("6 OFF");
        ledMode = 1;

      }
      if(sCmd.indexOf("FUNCTION7ON")>=0)
      {
        Serial.println("7 ON");
        ledMode = 8;

      }
      else if(sCmd.indexOf("FUNCTION7OFF")>=0)
      {
         Serial.println("7 OFF");
        ledMode = 1;

      }

      if(sCmd.indexOf("FUNCTION8ON")>=0)
      {
        Serial.println("8 ON");
        ledMode = 9;

      }
      else if(sCmd.indexOf("FUNCTION8OFF")>=0)
      {
         Serial.println("8 OFF");
        ledMode = 1;

      }
      if(sCmd.indexOf("FUNCTION9ON")>=0)
      {
        Serial.println("9 ON");
        ledMode = 10;

      }
      else if(sCmd.indexOf("FUNCTION9OFF")>=0)
      {
         Serial.println("9 OFF");
        ledMode = 1;

      }
      if(sCmd.indexOf("FUNCTION10ON")>=0)
      {
        Serial.println("10 ON");
        ledMode = 11;

      }
      else if(sCmd.indexOf("FUNCTION10OFF")>=0)
      {
         Serial.println("10 OFF");
        ledMode = 1;

      }

      if(sCmd.indexOf("FUNCTION11ON")>=0)
      {
        Serial.println("11 ON");
        ledMode = 12;

      }
      else if(sCmd.indexOf("FUNCTION11OFF")>=0)
      {
         Serial.println("11 OFF");
        ledMode = 1;

      }

    } 
    
    // end sCmd.length()>0
    
    //sResponse += "<FONT SIZE=-2>";
    //sResponse += "<BR>clicks on page ="; 
    //sResponse += ulReqcount;
    //sResponse += "<BR>";
    //sResponse += "Gyro Gearloose 02/2016<BR><BR>";
    //sResponse += "<font color=\"#FF0000\">";
    //sResponse += "DemoReel 100 by Mark Kriegsman<BR>";
    //sResponse += "Webserver by Stefan Thesen<BR>";
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
  }
/// END of complete web server //////////////////////////////////////////////////////////////////

// LED animations ###############################################################################
void all_off() {
  fill_solid(leds, NUM_LEDS, CRGB::Black); 
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND);   
  //FastLED.show();
  //FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void rainbow() 
{
  // FastLED's built-in rainbow generator
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow
  fill_rainbow( leds, NUM_LEDS, gHue, 7);
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void rainbowWithGlitter() 
{
  // built-in FastLED rainbow, plus some random sparkly glitter
  rainbow();
  addGlitter(80);
}

void addGlitter( fract8 chanceOfGlitter) 
{
  if( random8() < chanceOfGlitter) {
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
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void sinelon()
{
  // a colored dot sweeping back and forth, with fading trails
  fadeToBlackBy( leds, NUM_LEDS, 20);
  int pos = beatsin16(13,0,NUM_LEDS);
  leds[pos] += CHSV( gHue, 255, 192);
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void bpm()
{
  // colored stripes pulsing at a defined Beats-Per-Minute (BPM)
  uint8_t BeatsPerMinute = 102;
  CRGBPalette16 palette = PartyColors_p;
  uint8_t beat = beatsin8( BeatsPerMinute, 64, 255);
  for( int i = 0; i < NUM_LEDS; i++) { //9948
    leds[i] = ColorFromPalette(palette, gHue+(i*2), beat-gHue+(i*10));
  }
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void juggle() {
  // eight colored dots, weaving in and out of sync with each other
  fadeToBlackBy( leds, NUM_LEDS, 20);
  byte dothue = 0;
  for( int i = 0; i < 8; i++) {
    leds[beatsin16(i+7,0,NUM_LEDS)] |= CHSV(dothue, 200, 255);
    dothue += 32;
  }
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void justred() 
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Red); //Set all to red
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void justgreen() 
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Green); //Set all to green
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void justblue() 
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Blue); //Set all to blue
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void justpurple() 
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Purple); //Set all to purple
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}

void justorange() 
{
  // Fill LED with solid color
  fill_solid( leds, NUM_LEDS, CRGB::Orange); //Set all to orange
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
}
