 /*----------------------------------------------------------------------------------------------------- 
   Access Point                        Web Server  e.g.
   http://192.168.4.1                  http://192.168.1.252 (depends on your network)
   

   Works with generic ESP01, Huzzah, NodeMCU ESP12E, WeMos D1 Mini, and Sparkfun Thing, ; 
   other modules not tested yet.
   Upload: Keep >Prog< pressed, press shortly >Reset<, and release >Prog<   circuit here:
   http://www.esp8266.com/wiki/lib/exe/detail.php?id=getting-started-with-the-esp8266&media=esp8266_flash_prog_board_sch.png
   or:  http://ediy.com.my/index.php/blog/item/133-upload-sketch-to-the-esp8266-esp-07-esp-12-using-arduino-ide
   
   If you run in noise / flickering issues add a 470 Ohm resistor in series to the Data Line
  
   v01:   webserver extended to 7 on and off switches
   v02:   added demo reel. Had to declare FastLED before
          the webserver. Then put the complete webserver 
          into a void webserver() and leave in the loop
          just the case statements and the FastLEDshow.
          Added power control 
   v02.1  changed to a drop down menu  
   v02.2  Added a slider for overall brightness  // OMG how many hours ...
   v03    my private extended version 
   v04    Combined Webserver and Access Point in one sketch

  Usage:  ACCESS Point:
          After upload search with your device (Phone, Tablet, etc.) for 
          new WiFi. Select ESP_FastLED_Access_Point.
          Open in your webbrowser the URL 192.168.4.1 
          Optional print a QR-Code with the URL on your lamp http://goqr.me/
          
          WEB SERVER:
          After upload open the Serial Monitor in Arduino and see what 
          IP address is returned. In my case it is 192.168.1.252
          Open this IP address in a browser (PC or phone)
 
   Gyro Gearloose J. Bruegl, Feb 2016


   ToDo:  Create more sliders for CRGB and CHSV control

/*------------------------------------------------------------------------------------------------------
HTTP 1.1 Webserver for ESP8266 adapted to Arduino IDE

From Stefan Thesen 04/2015
https://blog.thesen.eu/http-1-1-webserver-fuer-esp8266-als-accesspoint/
https://blog.thesen.eu/stabiler-http-1-1-wlan-webserver-mit-dem-esp8266-microcontroller/

Running stable for days                                    // in combination w FastLED I get timeouts
(in difference to all samples I tried)

Does HTTP 1.1 with defined connection closing.
Reconnects in case of lost WiFi.
Handles empty requests in a defined manner.
Handle requests for non-exisiting pages correctly.

This demo allows to switch two functions:
Function 1 creates serial output and toggels GPIO2         // GPIO switch not used here
Function 2 just creates serial output.

Serial output can e.g. be used to steer an attached
Arduino, Raspberry etc.
-----------------------------------------------------------------------------------------------------*/

// FastLED setup ----------   FastLED has to be declared BEFORE the Webserver     ---------------------
#include "FastLED.h"
FASTLED_USING_NAMESPACE

#if defined(FASTLED_VERSION) && (FASTLED_VERSION < 3001000)
#warning "Requires FastLED 3.1 or later; check github for latest code."
#endif

#define DATA_PIN      0     // for Huzzah: Pins w/o special function:  #4, #5, #12, #13, #14; // #16 does not work :( 
//#define CLK_PIN     12
//#define LED_TYPE    APA102
//#define COLOR_ORDER BGR
#define LED_TYPE      WS2812B
#define COLOR_ORDER   GRB
#define NUM_LEDS      60     
CRGB leds[NUM_LEDS];

//#define BRIGHTNESS       128
int BRIGHTNESS =           128;   // this is half brightness 
int new_BRIGHTNESS =       128;   // shall be initially the same as brightness

#define MILLI_AMPERE      2000    // IMPORTANT: set here the max milli-Amps of your power supply 5V 2A = 2000
#define FRAMES_PER_SECOND  120    // here you can control the speed. With the Access Point / Web Server the 
                                  // animations run a bit slower. 

int ledMode = 2;                  // this is the starting animation


// Select EITHER ACCESS-Point  OR  WEB SERVER setup 
/*
// ACCESS-Point setup ------------------------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
// comes with Huzzah installation. Enter in Arduino settings:
// http://arduino.esp8266.com/package_esp8266com_index.json

//const char* ssid = "ESP_FastLED_Access_Point";
//const char* password = "";  // set to "" for open access point w/o password; or any other pw (min length = 8 characters)

unsigned long ulReqcount;

// Create an instance of the server on Port 80
WiFiServer server(80);
//IPAddress apIP(192, 168, 10, 1);                                        // if you want to configure another IP address
void setup() 
{
  // setup globals
  ulReqcount=0; 
  
  // prepare GPIO2       // not necessary for FastLED
  pinMode(2, OUTPUT);
  digitalWrite(2, 0);
  // start serial
  Serial.begin(9600);
  delay(1);
  
  // AP mode
  WiFi.mode(WIFI_AP);
//  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));          // if you want to configure another IP address
  WiFi.softAP(ssid, password);
  server.begin();
// end ACCESS-Point setup ---------------------------------------------------------------------------------------------------
*/


// WEB SERVER setup ---------------------------------------------------------------------------------------------------------
#include <ESP8266WiFi.h>
// comes with Huzzah installation. Enter in Arduino settings:
// http://arduino.esp8266.com/package_esp8266com_index.json

const char* ssid = "blank";   
const char* password = "1234567890";

unsigned long ulReqcount;
unsigned long ulReconncount;

WiFiServer server(80);  // Create an instance of the server on Port 80

void setup() 
{

  ulReqcount=0;         // setup globals for Webserver
  ulReconncount=0;
  
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


// now the settings for FastLED   
  delay(2000);          // sanity delay for LEDs
  FastLED.addLeds<LED_TYPE,DATA_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(DirectSunlight);           // for WS2812 (Neopixel)
  //FastLED.addLeds<LED_TYPE,DATA_PIN,CLK_PIN,COLOR_ORDER>(leds, NUM_LEDS).setCorrection(DirectSunlight); // for APA102 (Dotstar)
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
      }
      }
  show_at_max_brightness_for_power();
  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND); 
//  FastLED.show();  
//  FastLED.delay(1000/FRAMES_PER_SECOND); 
  EVERY_N_MILLISECONDS( 20 ) { gHue++; } // slowly cycle the "base color" through the rainbow

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
  Serial.println(sRequest);
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
//  Serial.print("iStart "); 
//  Serial.println(iStart); 

  if (iStart>=0)
  {
    iStart+=+sGetstart.length();
//  Serial.print("iStart + sGetstart "); 
//  Serial.println(iStart); 
    iEndSpace = sRequest.indexOf(" ",iStart);
//  Serial.print("iEndSpace "); 
//  Serial.println(iEndSpace); 
    iEndQuest = sRequest.indexOf("?",iStart);
//  Serial.print("iEndQuest "); 
//  Serial.println(iEndQuest);     
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
  
  //////////////////////////////////////////////////////////////////////////////////
  // output parameters to serial, you may connect e.g. an Arduino and react on it //
  //////////////////////////////////////////////////////////////////////////////////
  if(sParam.length()>0)
  {
    int iEqu=sParam.indexOf("=");
    if(iEqu>=0)
    {
      sCmd = sParam.substring(iEqu+1,sParam.length());
      Serial.print("We are in output Parameters, value is: ");
      Serial.println(sCmd);
      char carray[4];                                // values 0..255 = 3 digits; array = digits + 1
      sCmd.toCharArray(carray, sizeof(carray));      // convert char to the array
      new_BRIGHTNESS = atoi(carray);                 // atoi() converts an ascii character array to an integer
      if (new_BRIGHTNESS == 0) {new_BRIGHTNESS = BRIGHTNESS; }   // if something else is selected (no change in brightness)
      BRIGHTNESS = new_BRIGHTNESS;                 // works not this way 
         FastLED.setBrightness(new_BRIGHTNESS);      // that's how the new value is assigned  
      Serial.print("new Brightness: ");
      Serial.println(new_BRIGHTNESS);
    }
  }
  
  //////////////////////////////
  // format the html response //
  //////////////////////////////
  String sResponse,sHeader;
  
  ///////////////////////////////
  // 404 for non-matching path //
  ///////////////////////////////
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
  //////////////////////////
  // format the html page //
  //////////////////////////
  else
  {
    ulReqcount++;
    sResponse  = "<html><head><title>ESP_FastLED_Access_Point</title></head><body>";
//    sResponse += "<font color=\"#FFFFF0\"><body bgcolor=\"#000000\">";  
    sResponse += "<font color=\"#FFFFF0\"><body bgcolor=\"#151B54\">";  
    sResponse += "<FONT SIZE=-1>"; 
    sResponse += "<meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=yes\">";
    sResponse += "<h1>ESP FastLED DemoReel 100<br>";
    sResponse += " Light Controller</h1>";

/*  this creates a list with ON / OFF buttons 
    // &nbsp is a non-breaking space; moves next character over
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
//    sResponse += "Select Animation<br>";
    sResponse += "<p>Select:</p>";
    sResponse += "<select name=\"sCmd\" size=\"7\" >";
    sResponse += "<option value=\"FUNCTION1OFF\">All OFF</option>";
    sResponse += "<option value=\"FUNCTION1ON\"selected>Rainbow</option>";
    sResponse += "<option value=\"FUNCTION2ON\">Rainbow Glitter</option>";
    sResponse += "<option value=\"FUNCTION3ON\">Confetti</option>";
    sResponse += "<option value=\"FUNCTION4ON\">Sinelon</option>";
    sResponse += "<option value=\"FUNCTION5ON\">Juggle</option>";
    sResponse += "<option value=\"FUNCTION6ON\">BPM</option><br>";
    sResponse += "</select>";
    sResponse += "<br><br>";
    sResponse += "<input type= submit>";
    sResponse += "</form>";
//    sResponse += "<FONT SIZE=-1>";

// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Slider          this works, however I got http://192.168.4.1/sCmd?FUNCTION_200=80  and the page was not found
//                 I needed to take the FUNCTION_200=80 apart and call only FUNCTION_200 and assign
//                 the value (=80) in "react on parameters" (line 512) to new_BRIGHTNESS

sResponse += "</p>";
sResponse += "<form action=\"?sCmd\" >";    // ?sCmd forced the '?' at the right spot  
sResponse += "<BR>Brightness &nbsp;&nbsp";  // perhaps we can show here the current value
sResponse += round(new_BRIGHTNESS /2.5);    // this is just a scale depending on the max value; round for better readability
sResponse += " %";
sResponse += "<BR>";
sResponse += "<input style=\"width:200px; height:50px\" type=\"range\" name=\"=FUNCTION_200\" id=\"cmd\" value=\"";   // '=' in front of FUNCTION_200 forced the = at the right spot
sResponse += BRIGHTNESS;
sResponse += "\" min=10 max=250 step=10 onchange=\"showValue(points)\" />";
sResponse += "<BR><BR>";
sResponse += "<input type=\"submit\">";
sResponse += "</form>";
sResponse += "<p>";
// ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

 sResponse += "<FONT SIZE=-1>";


    /////////////////////////
    // react on parameters //
    /////////////////////////
    if (sCmd.length()>0)
    {
      // write received command to html page
 //     sResponse += "Command: " + sCmd + "<BR>";
      
      // switch the animiation (based on your choice in the case statement (main loop) 
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

//+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// oh well, I was a bit frustrated. Came up with the idea to make
// 10 digits increments and let the URL (not) react on it.
// However, I was able to assign a new_BRIGHTNESS value; 
// what after all serves the purpose. Maybe someone comes up with 
// a more ellegant way - HOPEFULLY
// (more than 400 have downloaded my code but nobody felt the need  
// to help. wtf - this is my very first attempt on HTML ! 
// Guys, I'm a simple electrician, so PLEASE help  :(          )

// do not call a new page when the slider is moved, but assign the new value
// to BRIGHTNESS (this is done in "output parameters to serial", line 314

      if(sCmd.indexOf("FUNCTION_200=20")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=30")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=40")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=50")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=60")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=70")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=80")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=90")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=100")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=110")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=120")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=130")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=140")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=150")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=160")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=170")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=180")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=190")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=200")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=210")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=220")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=230")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=240")>=0) { }
      if(sCmd.indexOf("FUNCTION_200=250")>=0) { }
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++

    } // end sCmd.length()>0
   
    
//    sResponse += "<FONT SIZE=-2>";
//    sResponse += "<BR>clicks on page ="; 
//    sResponse += ulReqcount;
    sResponse += "<BR>";
    sResponse += "<BR>";
    sResponse += "Powered by FastLED<BR><BR>";
    sResponse += "<FONT SIZE=-2>";
    sResponse += "<font color=\"#FFDE00\">";
    sResponse += "DemoReel 100 by Mark Kriegsman<BR>";
    sResponse += "Webserver by Stefan Thesen<BR>";
    sResponse += "<font color=\"#FFFFF0\">";
    sResponse += "Gyro Gearloose &nbsp;&nbsp;Feb 2016<BR>";
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
void all_off() {
  fill_solid(leds, NUM_LEDS, CRGB::Black); 
//  show_at_max_brightness_for_power();
//  delay_at_max_brightness_for_power(1000/FRAMES_PER_SECOND);   
  FastLED.show();
  FastLED.delay(1000/FRAMES_PER_SECOND); 
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
  uint8_t BeatsPerMinute = 62;
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
///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/*  HTML code to try changes on w3schools.com  http://www.w3schools.com/tags/tryit.asp?filename=tryhtml_a_href

copy this pure HTML code and see how it looks before you upload the code to your ESP
However, as we create a text with sResponse += in the normal HTML code \" have to be added at the right places. 
Believe me, there is a lot of trial and error involved ....

<!DOCTYPE html>
<html>
<font color =   #fffff0><body bgcolor=\"#000000\">   //FFFFFF0 has to be written in fffff0
<FONT SIZE=-1>
<h1>ESP8266 control <br>
for DemoReel100</h1>
<body>

<p>Rainbow &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION1ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION1OFF\"><button>--OFF--</button></a><br>

<p>Rainbow Glitter<a href=\"?pin=FUNCTION2ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION2OFF\"><button>--OFF--</button></a><br>

<p>Confetti &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION3ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION3OFF\"><button>--OFF--</button></a><br>

<p>Sinelon &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION4ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION4OFF\"><button>--OFF--</button></a><br>

<p>Juggle&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION5ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION5OFF\"><button>--OFF--</button></a></p>

<p>BPM&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION6ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION6OFF\"><button>--OFF--</button></a></p>

<p>Function 7&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;<a href=\"?pin=FUNCTION7ON\"><button>--ON--</button></a>&nbsp;<a href=\"?pin=FUNCTION7OFF\"><button>--OFF--</button></a></p><br>

<FONT SIZE=+1>
<form>
<p>Select Animation</p>
<select name=\"sCmd\" size=\"7\" >
<option value=\"FUNCTION1OFF\"selected>All OFF</option>
<option value=\"FUNCTION1ON\">Rainbow</option>
<option value=\"FUNCTION2ON\">Rainbow Glitter</option>
<option value=\"FUNCTION3ON\">Confetti</option>
<option value=\"FUNCTION4ON\">Sinelon</option>
<option value=\"FUNCTION5ON\">Juggle</option>
<option value=\"FUNCTION6ON\">BPM</option><br>
</select>
<br><br>
<input type= submit>
</form>
<FONT SIZE=-1>

<FONT SIZE=-2>
<BR>clicks on page =
 - connections to page =
<BR>
<font color= #ff0000>
DemoReel 100 by Mark Kriegsman<BR>
Webserver by Stefan Thesen<BR>

</body>
</html>

// slider
<html>


   <title>BRIGHTNESS</title></head>
   <body><center><h1>Brightness</h1></center>
   <br /><br />
   <form action="/" name="dimmer" oninput="outputUpdate(cmd.value" 
                     method="POST">
   <font color=red><b>ON </b></font><input style="width:550px; height:50px" 
                     type="range" name="cmd" id="cmd" value="
   status
   " min=1 max=128 step=1 />
   <font color=red><b>OFF</b></font></form>
   <br><center>Dimmer Value: <b>         
      status
       pts</b></center>
   <script type="text/javascript">
   function outputUpdate(dim {document.query}Selector("#cmd".value = dim;)
   document.forms[0].submit();}</script>
   </body></html>


*/
