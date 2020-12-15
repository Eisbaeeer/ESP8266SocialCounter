/***********************************************************************************
 * Instagram Follower Counter
 * Source: https://github.com/jegade/followercounter
 * 
 * Version 2.2
 * (Eisbaeeer 20201214)
 * Bugfix MAX_DEVICES wird nicht aus JSON gelesen
 * 
 * Version 2.1
 * (Eisbaeeer 20201119
 * + Ein- Ausblenden
 * 
 * Version 2.0
 * (Eisbaeeer 20201117)
 * + changed libs from u8g2 to MD_MAX72xx
 * + added animations
 * 
 * Version 1.5
 * (Eisbaeeer 20201102)
 * + added temperature and humidity with DHT22 sensor
 * + webpage config select mode (completely new)
 * + changed font type
 * NEU
 * Symbol bei Uhrzeitanzeige
 * Neues Einstellungsmenü (kurzer / langer Tastendruck)
 * Jede Anzeigeart kann separat angewählt werden
 * Neue Symbole
 * Schrift wird in voller Höhe angezeigt
 * Temperaturanzeige möglich (Sensor nötig)
 * Luftfeuchtigkeit möglich (Sensor nötig)
 * 
 * Version 1.4
 * (Eisbaeeer 20201026
 * + display correct counter with logo if more than 4 modules used
 * 
 * Version 1.3
 * (Eisbaeeer 20201015)
 * + push button mode on display
 * 
 * Changed by Eisbaeeer
 * Version: 1.2
 * 
 * Date: 20201012
 * + WebPage Config full support
 * + Changed binary to default export of Arduino IDE
 * + Changed startup mode to 7
 * 
 * Date: 20201011
 * + Update URL --> http://firmware.kidbuild.de/ESP8266SocialCounter.ino.nodemcu.bin
 * + YouTube Counter
 * 
 * Tastendruck
 * 1 = Instagram
 * 2 = YouTube
 * 3 = Uhrzeit
 * 4 = Zeit / Instagram
 * 5 = Zeit / YouTube
 * 6 = YoutTube / Instagram
 * 7 = Zeit / Instagram / Youtube
 * 8 = WLAN Info
 * 9 = IP-Adresse
 * 10 = Version
 * 11 = Firmware update via Internet
 * 12 = Restart
 * 20 = Factory restore
 * 
 */
 
#include <FS.h>                    //this needs to be first, or it all crashes and burns...
#include "Arduino.h"
#include <ESP8266WiFi.h>
#include "InstagramStats.h"        // InstagramStats              https://github.com/witnessmenow/arduino-instagram-stats
#include "JsonStreamingParser.h"   // Json Streaming Parser  
#include <YoutubeApi.h>            // Youtube
#include <Adafruit_Sensor.h>       // DHT Sensor
#include <DHT.h>                   // DHT Sensor
#include <DHT_U.h>                 // DHT Sensor
#include <ezButton.h>              // EZButton Lib

#include <ESP8266HTTPClient.h>     // Web Download
#include <ESP8266httpUpdate.h>     // Web Updater

#include <ArduinoJson.h>          // ArduinoJSON                 https://github.com/bblanchon/ArduinoJson

#include <DNSServer.h>            // - Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     // - Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          // WifiManager 


#include <NTPClient.h>
#include <time.h>

#include <Arduino.h>
#include <MD_MAX72xx.h>
#include <SPI.h>

#include <ESPStringTemplate.h>

#include "index.h"

#define DEBUG   // Enable or disable (default) debugging output

#ifdef DEBUG
#define PRINT(s, v)   { Serial.print(F(s)); Serial.print(v); }      // Print a string followed by a value (decimal)
#define PRINTX(s, v)  { Serial.print(F(s)); Serial.print(v, HEX); } // Print a string followed by a value (hex)
#define PRINTB(s, v)  { Serial.print(F(s)); Serial.print(v, BIN); } // Print a string followed by a value (binary)
#define PRINTC(s, v)  { Serial.print(F(s)); Serial.print((char)v); }  // Print a string followed by a value (char)
#define PRINTS(s)     { Serial.print(F(s)); }                       // Print a string
#else
#define PRINT(s, v)   // Print a string followed by a value (decimal)
#define PRINTX(s, v)  // Print a string followed by a value (hex)
#define PRINTB(s, v)  // Print a string followed by a value (binary)
#define PRINTC(s, v)  // Print a string followed by a value (char)
#define PRINTS(s)     // Print a string
#endif

// actually define hard
char instagramName[40];
char matrixIntensity[5] = "10";
char maxModules[5];
char API_KEY[40];  // your google apps API Token
char CHANNEL_ID[30]; // makes up the url of channel
char humiStat[8] = "";   // states
char tempStat[8] = "";
char instaStat[8] = "checked";
char youtuStat[8] = "checked";
char clockStat[8] = "checked";
char fadeStat[8] = "checked";
char ghostStat[8] = "";

char htmlBuffer[4096];

int menuActive = 0;    // Menu
int buttonLong = 0;  
int menuPtr = 0;            
bool displayInit = false;
int fadeInTime = 0;
int fadeInStat = 0;
int fadeOutTime = 0;
int fadeOutStat = 0;

unsigned long secondPreviousMillis = 0;
const long secondInterval = 1000;   // one second
int seconds;                          // var for counting seconds

// MD_MAX72XX Parameter
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
int MAX_DEVICES = 4;  // Default to minimum count of devices
#define CLK_PIN   D6
#define CS_PIN    D7
#define DATA_PIN  D8
//MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); // Arbitrary pins
MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, 5); // Arbitrary pins

#define EEADDR 1024 // Start location to write EEPROM data.

#define ANIMATION_DELAY 75  // milliseconds
#define MAX_FRAMES      4   // number of animation frames
#define CHAR_SPACING  1 // pixels between characters

// include the symbols
#include "symbols.h" 

uint32_t prevTimeAnim = 0;  // remember the millis() value in animations
int16_t idx;                // display index (column)
uint8_t frame;              // current animation frame
uint8_t deltaFrame;         // the animation frame offset for the next frame
String matrixIntensityString;


// EZButton
const int SHORT_PRESS_TIME = 200; // 1000 milliseconds
const int LONG_PRESS_TIME  = 400; // 1000 milliseconds
ezButton button(0);  // D3 pin to pushbutton
unsigned long pressedTime  = 0;
unsigned long releasedTime = 0;

const long interval = 3000*1000;  // alle 50 Minuten prüfen
//const long interval = 3000*200;  // alle 10 Minuten prüfen
unsigned long previousMillis = millis() - 2980*1000; 
unsigned long lastPressed = millis();
unsigned long animationMillis = 0;  // helper for animation
int animationInterval = 75;                  // interval for every animation move
int frameCount = 0;                          // start animation frame

WiFiClientSecure client;
WiFiClient updateclient;

YoutubeApi api(API_KEY, client);

InstagramStats instaStats(client);
ESP8266WebServer server(80);

char time_value[20];

int textsize = 0;
int displayPtr = 1;     // pointer display items
int follower;
int fade = 0;     // Fade Status

// obsolete because ezbutton
int mode = 7;
// int modetoggle = 0;

// Variables will change:
int buttonPushCounter = 0;   // counter for the number of button presses
int buttonState = 1;         // current state of the button
int lastButtonState = 1;     // previous state of the button

#define VERSION "2.2"
#define USE_SERIAL Serial

// DHT sensor
#define DHTPIN 4    //D2
#define DHTTYPE    DHT22     // DHT 22 (AM2302)
DHT_Unified dht(DHTPIN, DHTTYPE);

//flag for saving data
bool shouldSaveConfig = false;

// ---------------------------------------------------------------------------------------------
// SETUP
// ---------------------------------------------------------------------------------------------
void setup() {
  // Serial debugging
  Serial.begin(115200);

 // EZButton
 button.setDebounceTime(50); // set debounce time to 50 milliseconds
 
  // Set Reset-Pin to Input Mode
  //pinMode(TOGGLE_PIN, INPUT);
  
  if (SPIFFS.begin()) {
    if (SPIFFS.exists("/config.json")) {
      //file exists, reading and loading
      File configFile = SPIFFS.open("/config.json", "r");
      if (configFile) {
        Serial.println("opened config file");
        size_t size = configFile.size();
        // Allocate a buffer to store contents of the file.
        std::unique_ptr<char[]> buf(new char[size]);

        //ACHTUNG FORMAT
        //infoReset();
        //
        //ACHTUNG FORMAT

        configFile.readBytes(buf.get(), size);
        DynamicJsonDocument json(4096);
        deserializeJson(json, buf.get());
        serializeJson(json,Serial);
       
        strcpy(instagramName, json["instagramName"]);
        strcpy(maxModules, json["maxModules"]);
        strcpy(API_KEY, json["API_KEY"]);
        strcpy(CHANNEL_ID, json ["CHANNEL_ID"]);
        strcpy(humiStat, json ["humiStat"]);
        strcpy(tempStat, json ["tempStat"]);
        strcpy(instaStat, json ["instaStat"]);
        strcpy(youtuStat, json ["youtuStat"]);
        strcpy(clockStat, json ["clockStat"]);
        strcpy(ghostStat, json ["ghostStat"]);
        strcpy(fadeStat, json ["fadeStat"]);
        
        
        JsonVariant jsonMatrixIntensity = json["matrixIntensity"];
        if (!jsonMatrixIntensity.isNull()) {
            strcpy(matrixIntensity, json["matrixIntensity"]);
        } 
                
        JsonVariant jsonMode = json["mode"];
        if (!jsonMode.isNull()) { 
          mode = jsonMode.as<int>();
        } else {

        }
      }
    } else {
      
    }
  } else {
    Serial.println("failed to mount FS");
  }
  
  WiFiManager wifiManager;

  // Requesting Instagram and Intensity for Display
  WiFiManagerParameter custom_instagram("Instagram", "Instagram", instagramName, 40);
  WiFiManagerParameter custom_intensity("Helligkeit", "Helligkeit 0-15", matrixIntensity, 5);
  WiFiManagerParameter custom_modules("Elemente", "Anzahl Elemente 4-8", maxModules, 5);
  WiFiManagerParameter custom_API_KEY("API_KEY", "YouTube API KEY", API_KEY, 40);
  WiFiManagerParameter custom_CHANNEL_ID("CHANNEL_ID", "YoutTube Channel ID", CHANNEL_ID, 30);

  // Add params to wifiManager
  wifiManager.addParameter(&custom_instagram);
  wifiManager.addParameter(&custom_API_KEY);
  wifiManager.addParameter(&custom_intensity);
  wifiManager.addParameter(&custom_modules);
  wifiManager.addParameter(&custom_CHANNEL_ID);

#ifdef DEBUG
  Serial.begin(115200);
#endif
  
  // MD_MAX72XX Paramter
  mx.begin();
  resetMatrix();
  printText(0, MAX_DEVICES-1, "setup");
    
  // NTP settings
  configTime(0, 0, "pool.ntp.org", "time.nist.gov");
  setenv("TZ", "CET-1CEST,M3.5.0,M10.5.0/3", 0);  // https://github.com/nayarsystems/posix_tz_db 
   
  //set config save notify callback
  wifiManager.setSaveConfigCallback(saveConfigCallback);
  wifiManager.autoConnect("FollowerCounter");

  server.on("/", handleRoot);
  server.on("/format", getFormat);
  server.on("/update", getUpdate);
  server.on("/reset", getReset);
  server.on("/config", getConfig);
  server.begin();
 
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
    
  //read updated parametersu
  strcpy(instagramName, custom_instagram.getValue());
  strcpy(matrixIntensity, custom_intensity.getValue());
  strcpy(maxModules,custom_modules.getValue());
  strcpy(API_KEY,custom_API_KEY.getValue());
  strcpy(CHANNEL_ID,custom_CHANNEL_ID.getValue());

  String matrixIntensityString = matrixIntensity;
  Serial.print("MAX_INTENSITY: ");
  Serial.println(MAX_INTENSITY);
  Serial.print("matrixIntensity: ");
  Serial.println(matrixIntensityString.toInt());
  int intensity = atoi(matrixIntensity);
  MAX_DEVICES = atoi(maxModules);
  Serial.print("Anzahl Module: ");
  Serial.println(MAX_DEVICES);
  
  // Set MD_MAX again!
  //MD_MAX72XX mx = MD_MAX72XX(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES); // Arbitrary pins
  //mx.begin();
  resetMatrix();
  mx.control(MD_MAX72XX::INTENSITY, intensity);
   
  //save the custom parameters to FS
  if (shouldSaveConfig) {
      saveConfig();
    //end save
  }

  // Required for instagram api
  client.setInsecure();

  // Youtube Debug settings
  api._debug = false;

  // DHT sensor stuff
  // Initialize device.
  dht.begin();
  Serial.println(F("DHTxx Unified Sensor Example"));
  // Print temperature sensor details.
  sensor_t sensor;
  dht.temperature().getSensor(&sensor);
  Serial.println(F("------------------------------------"));
  Serial.println(F("Temperature Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("°C"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("°C"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("°C"));
  Serial.println(F("------------------------------------"));
  // Print humidity sensor details.
  dht.humidity().getSensor(&sensor);
  Serial.println(F("Humidity Sensor"));
  Serial.print  (F("Sensor Type: ")); Serial.println(sensor.name);
  Serial.print  (F("Driver Ver:  ")); Serial.println(sensor.version);
  Serial.print  (F("Unique ID:   ")); Serial.println(sensor.sensor_id);
  Serial.print  (F("Max Value:   ")); Serial.print(sensor.max_value); Serial.println(F("%"));
  Serial.print  (F("Min Value:   ")); Serial.print(sensor.min_value); Serial.println(F("%"));
  Serial.print  (F("Resolution:  ")); Serial.print(sensor.resolution); Serial.println(F("%"));
  Serial.println(F("------------------------------------"));

  // lets start
  //printString(0,8,"start...",2);
  printText(0, MAX_DEVICES-1, "start...");
  prevTimeAnim = millis();
    
}

// ---------------------------------------------------------------------------------------------
// MAIN Program Loop
// ---------------------------------------------------------------------------------------------
void loop() {
  
  server.handleClient();
  
  // EZButton
  button.loop(); // MUST call the loop() function first

  if(button.isPressed())
    pressedTime = millis();

  if(button.isReleased()) {
    releasedTime = millis();

    long pressDuration = releasedTime - pressedTime;

    if ( pressDuration < SHORT_PRESS_TIME ) {
      Serial.println("A short press is detected");
      Serial.print("menuActive: ");
      Serial.println(menuActive);
      menuActive = 1;
      menuPtr++;
      Serial.print("menuActive: ");
      Serial.println(menuActive);
      Serial.print("menuPtr: ");
      Serial.println(menuPtr);
      buttonLong = 0;
      settingsMenu();
    }

    if ( pressDuration > LONG_PRESS_TIME ) {
      Serial.println("A long press is detected");
      buttonLong = 1;
      mx.clear();
      settingsMenu();
      
    }
  }

  unsigned long currentMillis = millis();  
  
  // Update follower count and do messure temp and humidity
  if (currentMillis - previousMillis >= interval) {
    previousMillis = currentMillis;
    
    Serial.println(instagramName);
    
    if ( strcmp (youtuStat,"checked") == 0 ) {
    Serial.println("requesting youtube counters..."); 
    youtube();
    }

    if ( strcmp (instaStat,"checked") == 0 ) {
    Serial.println("requesting instagram counter..."); 
    instagram();
    }

    getSensor();
  }


  // ---------------------------------------------------
  // Zeige Werte auf Display
  // ---------------------------------------------------

  // BEGIN draw animation if active
  if (displayInit) {
    animation();
  } 
  // END draw animations


  // Eine Sekunde
  unsigned long secondCurrentMillis = millis();
  if (secondCurrentMillis - secondPreviousMillis >= secondInterval) {
      secondPreviousMillis = secondCurrentMillis;
      seconds++;
  }   
  
  if ( currentMillis % 15000 == 0 ) {
        animation();
        if ( (strcmp (ghostStat,"") == 0) && (strcmp (fadeStat,"") == 0) ) {
          DisplayValues(); 
        }
  }
  
  if ( currentMillis % 250 == 0 ) {
      fadeInTime++;
      fadeOutTime--;
  }

  if ( fadeInStat == 1 ) {
//    if ( strcmp (ghostStat,"") == 0) {
      if ( strcmp (fadeStat,"checked") == 0 ) {
        int intensity = atoi(matrixIntensity);
       if ( fadeInTime <= intensity ) {
            mx.control(MD_MAX72XX::INTENSITY, fadeInTime);
       } else {
        fadeInStat = 0;
       }
      }
   }
 // }

 if (fadeOutStat == 1 ) {
 // if ( strcmp (ghostStat,"") == 0) {
      if ( strcmp (fadeStat,"checked") == 0 ) {
        if ( fadeOutTime >= 0 ) {
            mx.control(MD_MAX72XX::INTENSITY, fadeOutTime);
       } else {
        fadeOutStat = 0;
        mx.clear();
        DisplayValues();
       }
      }
  }
// }
  
}



// *****************************************************
// ENDE Loop
// *****************************************************

void DisplayValues (void) { 
    if ( menuActive == 0 ) {
      Serial.print("menuActive: ");
      Serial.println(menuActive);  
      Serial.print("Disp. Pointer: ");
      Serial.println(displayPtr);
      Serial.print("Intensity: ");
      Serial.println(matrixIntensity);
   
  if ( displayPtr == 1 ) {
     if ( strcmp (humiStat,"checked") == 0 ) {
         Serial.println("Humidity");
         printHumidity();
         fadeInTime = 0;
         fadeInStat = 1;
      } else {
        displayPtr++;
      }
  }

  if ( displayPtr == 2 ) {
      if ( strcmp (tempStat,"checked") == 0 ) {
         Serial.println("Temperature");
         printTemperature();
         fadeInTime = 0;
         fadeInStat = 1;
      } else {
        displayPtr++;
      }
  }

  if ( displayPtr == 3 ) {
      if ( strcmp (instaStat,"checked") == 0 ) {
        Serial.println("Instagram");
         printCurrentFollower();
         fadeInTime = 0;
         fadeInStat = 1;
      } else {
        displayPtr++;
      }
  }

  if ( displayPtr == 4 ) {
      if ( strcmp (youtuStat,"checked") == 0 ) {
         Serial.println("Youtube");       
         printYoutubeFollower();
         fadeInTime = 0;
         fadeInStat = 1;
      } else {
        displayPtr++;
      }
  }

  if ( displayPtr == 5 ) {
      if ( strcmp (clockStat,"checked") == 0 ) {
         Serial.println("Clock");    
         printTime();
         fadeInTime = 0;
         fadeInStat = 1;
      } else {
        displayPtr++;
      }
  }

  if ( displayPtr >= 5 ) {
      displayPtr = 0;
    }
    displayPtr++;  
  }
  // END Display Scrolling Values
}


//callback notifying us of the need to save config
void saveConfigCallback () {
  Serial.println("Should save config");
  shouldSaveConfig = true;
}

void resetMatrix(void) {
  int intensity = atoi(matrixIntensity);
  mx.control(MD_MAX72XX::INTENSITY, intensity);
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
  mx.clear();
}



void printText(uint8_t modStart, uint8_t modEnd, char *pMsg)
// Print the text string to the LED matrix modules specified.
// Message area is padded with blank columns after printing.
{
  uint8_t   state = 0;
  uint8_t   curLen;
  uint16_t  showLen;
  uint8_t   cBuf[8];
  int16_t   col = ((modEnd + 1) * COL_SIZE) - 1;

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);

  do     // finite state machine to print the characters in the space available
  {
    switch(state)
    {
      case 0: // Load the next character from the font table
        // if we reached end of message, reset the message pointer
        if (*pMsg == '\0')
        {
          showLen = col - (modEnd * COL_SIZE);  // padding characters
          state = 2;
          break;
        }

        // retrieve the next character form the font file
        showLen = mx.getChar(*pMsg++, sizeof(cBuf)/sizeof(cBuf[0]), cBuf);
        curLen = 0;
        state++;
        // !! deliberately fall through to next state to start displaying

      case 1: // display the next part of the character
        mx.setColumn(col--, cBuf[curLen++]);

        // done with font character, now display the space between chars
        if (curLen == showLen)
        {
          showLen = CHAR_SPACING;
          state = 2;
        }
        break;

      case 2: // initialize state for displaying empty columns
        curLen = 0;
        state++;
        // fall through

      case 3:  // display inter-character spacing or end of message padding (blank columns)
        mx.setColumn(col--, 0);
        curLen++;
        if (curLen == showLen)
          state = 0;
        break;

      default:
        col = -1;   // this definitely ends the do loop
    }
  } while (col >= (modStart * COL_SIZE));

  mx.control(modStart, modEnd, MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
}

void animation() {

    //if (fadeOutStat != 1 ) {

    if ( menuActive == 0) {       // Animation in Menu forbidden

   unsigned long animationCurrentMillis = millis();
   
   if (!displayInit) {
   mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::OFF);
   // initialize
   idx = 0;
   frame = 0;
   deltaFrame = 1;
   displayInit = true;
   frameCount = 0;
    }

    if (animationCurrentMillis - animationMillis > animationInterval) {
    // save the last time you run anmiation
    animationMillis = animationCurrentMillis; 
    
      // run animation with dispaly width
    
    //for ( int i = 0; i < modules*8+18; i++ ) 
    if ( (frameCount < MAX_DEVICES*8+18) && (strcmp (ghostStat,"checked") == 0) ) {   
        frameCount++;
        
  // now run the animation
  // clear old graphic
  for (uint8_t i=0; i<DATA_WIDTH; i++)
    mx.setColumn(idx-DATA_WIDTH+i, 0);
  // move reference column and draw new graphic
  idx++;
  for (uint8_t i=0; i<DATA_WIDTH; i++)
    mx.setColumn(idx-DATA_WIDTH+i, pacman[frame][i]);

  // advance the animation frame
  frame += deltaFrame;
  if (frame == 0 || frame == MAX_FRAMES-1)
    deltaFrame = -deltaFrame;

  // check if we are completed and set initialise for next time around
  mx.control(MD_MAX72XX::UPDATE, MD_MAX72XX::ON);
    } else {                          // Wenn die Animation fertig ist, nächsten Wert anzeigen
      displayInit = false;
      frameCount = 0;
      //DisplayValues();
      fadeOutTime = atoi(matrixIntensity);
      fadeOutStat = 1;
      
      if ( (strcmp (ghostStat,"checked" ) == 0) && ( strcmp (fadeStat,"" ) == 0) ) { 
        DisplayValues();   
      }
      
    }
  }
 }
}
//}

void handleRoot() {
  //Serial.print("HTML Buffer:");
  //Serial.println(htmlBuffer);
  ESPStringTemplate webpage(htmlBuffer, sizeof(htmlBuffer));
  TokenStringPair pair[1];
  TokenStringPair apipair[1];
  TokenStringPair chanpair[1];
  TokenStringPair humipair[1];
  TokenStringPair temppair[1];
  TokenStringPair instapair[1];
  TokenStringPair youtupair[1];
  TokenStringPair clockpair[1];
  TokenStringPair ghostpair[1];
  TokenStringPair fadepair[1];
  
  pair[0].setPair("%INSTAGRAM%", instagramName);
  apipair[0].setPair("%YOUTUBEAPI%", API_KEY);
  chanpair[0].setPair("%YOUTUBECHA%", CHANNEL_ID);
  humipair[0].setPair("%HUMISTAT%", humiStat);
  temppair[0].setPair("%TEMPSTAT%", tempStat);
  instapair[0].setPair("%INSTASTAT%", instaStat);
  youtupair[0].setPair("%YOUTUSTAT%", youtuStat);
  clockpair[0].setPair("%CLOCKSTAT%", clockStat);
  ghostpair[0].setPair("%GHOSTSTAT%", ghostStat);
  fadepair[0].setPair("%FADESTAT%", fadeStat);
    
  webpage.add_P(_PAGE_HEAD);
  webpage.add_P(_PAGE_START);
  
  webpage.add_P(_PAGE_CONFIG_HUMI, humipair,1);
  webpage.add_P(_PAGE_CONFIG_TEMP, temppair,1);
  webpage.add_P(_PAGE_CONFIG_INSTA, instapair,1);
  webpage.add_P(_PAGE_CONFIG_YOUTU, youtupair,1);
  webpage.add_P(_PAGE_CONFIG_CLOCK, clockpair,1);
  webpage.add_P(_PAGE_CONFIG_GHOST, ghostpair,1);
  webpage.add_P(_PAGE_CONFIG_FADE, fadepair,1);
  
  webpage.add_P(_PAGE_CONFIG_NAME, pair,1);
  webpage.add_P(_PAGE_CONFIG_YOUKEY, apipair,1);
  webpage.add_P(_PAGE_CONFIG_YOUCHA, chanpair,1);

  TokenStringPair intensityPair[1]; 
  intensityPair[0].setPair("%INTENSITY%",matrixIntensity );
  
  webpage.add_P(_PAGE_CONFIG_INTENSITY, intensityPair, 1);
  webpage.add_P(_PAGE_FOOTER);
  webpage.add_P(_PAGE_ACTIONS);
  webpage.add_P(_PAGE_SOURCE);

  server.send(200, "text/html", htmlBuffer);
}

void redirectBack() {
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void getConfig() {

  // instagramName
  String instagramNameString = server.arg("instagramname");
  instagramNameString.toCharArray(instagramName,40);

  // API_KEY
  String youtubeApiString = server.arg("youtubeapi");
  youtubeApiString.toCharArray(API_KEY,40);

  // Channel_ID
  String youtubeChaString = server.arg("youtubeCha");
  youtubeChaString.toCharArray(CHANNEL_ID,30);

  // HumidityCheckbox
  String humidityString = server.arg("humidity");
  humidityString.toCharArray(humiStat,8);

  // TemperatureCheckbox
  String temperatureString = server.arg("temperature");
  temperatureString.toCharArray(tempStat,8);

  // InstaCheckbox
  String instaString = server.arg("insta");
  instaString.toCharArray(instaStat,8);

  // YouTubeCheckbox
  String youtubeString = server.arg("youtu");
  youtubeString.toCharArray(youtuStat,8);

  // ClockCheckbox
  String clockString = server.arg("clock");
  clockString.toCharArray(clockStat,8);

  // GhostCheckbox
  String ghostString = server.arg("ghost");
  ghostString.toCharArray(ghostStat,8);

  // FadeCheckbox
  String fadeString = server.arg("fade");
  fadeString.toCharArray(fadeStat,8);

  // Intensity
  String intensityString = server.arg("intensity");
  String matrixIntensityString = intensityString;
  matrixIntensityString.toCharArray(matrixIntensity,40);

 // u8g2.setContrast(16*matrixIntensityString.toInt());
 // u8g2.refreshDisplay();

#ifdef DEBUG
      Serial.println("Save config with values:");
      Serial.print("Instagram Name: ");
      Serial.println(instagramNameString);
      Serial.print("YoutTube API Key: ");
      Serial.println(youtubeApiString);
      Serial.print("YoutTube Channel ID: ");
      Serial.println(youtubeChaString);
      Serial.print("Helligkeit: ");
      Serial.println(matrixIntensityString);
      Serial.println("--- Statis ---");
      Serial.print("HumiCheckbox: ");
      Serial.println(humiStat);  
      Serial.print("TempCheckbox: ");
      Serial.println(tempStat);    
      Serial.print("InstaCheckbox: ");
      Serial.println(instaStat);    
      Serial.print("YoutubeCheckbox: ");
      Serial.println(youtuStat);    
      Serial.print("ClockCheckbox: ");
      Serial.println(clockStat);    
      Serial.print("GhostCheckbox: ");
      Serial.println(ghostStat);    
      Serial.print("fadeCheckbox: ");
      Serial.println(fadeStat);    
 #endif

      // update display with new settings
      settingsMenu();  
      saveConfig();
      redirectBack();
      mx.control(MD_MAX72XX::INTENSITY, matrixIntensityString.toInt());
      
  }
 
void getReset() {
  redirectBack();
  restartX();
}

void getUpdate() {
  redirectBack();
  updateFirmware();
}

void getFormat() {
  redirectBack();
  infoReset();
}



void saveConfig() {
    DynamicJsonDocument json(4096);
    json["instagramName"] = instagramName;
    json["matrixIntensity"] = matrixIntensity;
    json["maxModules"] = maxModules;
    json["mode"] = mode;
    json["API_KEY"] = API_KEY;
    json["CHANNEL_ID"] = CHANNEL_ID;
    json["humiStat"] = humiStat;
    json["tempStat"] = tempStat;
    json["instaStat"] = instaStat;
    json["youtuStat"] = youtuStat;
    json["clockStat"] = clockStat;
    json["ghostStat"] = ghostStat;
    json["fadeStat"] = fadeStat;    
    
    File configFile = SPIFFS.open("/config.json", "w");
    
    if (!configFile) {
      Serial.println("failed to open config file for writing");
    }

    serializeJson(json, Serial);
    serializeJson(json, configFile); 
}

void infoWlan() {
  if (WiFi.status() ==  WL_CONNECTED ) {
    // WLAN Ok
    //printString(0,8,"WIFI OK",2);
    printText(0, MAX_DEVICES-1, "WIFI OK");
    delay(2000);
  } else {
    // WLAN Error
    //printString(0,8,"WIFI Error",2);
    printText(0, MAX_DEVICES-1, "WIFI Error");
    delay(2000);
  }
}

void infoIP() {
  String localIP = WiFi.localIP().toString();
  //printString(0,8,"IP:",2);
  char copy[localIP.length()+1];
  localIP.toCharArray(copy, localIP.length()+1);
  printText(0, MAX_DEVICES-1, "IP");
  delay(2000);
  //printString(0,8,localIP,2);
  printText(0, MAX_DEVICES-1, copy);
  delay(2000);
  //printString(0,8,localIP.substring(8),2);
  //printText(0, MAX_DEVICES-1, copy.substring(8));
  //delay(2000);
}

void infoVersion() {
  char versionString[8];
  sprintf(versionString,"v:%s", VERSION);
  //printString(0,8,versionString ,2);
  printText(0, MAX_DEVICES-1, versionString);
  delay(2000);
}

void infoReset() {
    Serial.println("Format System");
    //printString(0,8,"format",2);
    printText(0, MAX_DEVICES-1, "format...");
    // Reset Wifi-Setting
    WiFiManager wifiManager;
    wifiManager.resetSettings();
    // Format Flash
    SPIFFS.format();
    // Restart
    ESP.reset();
}

void restartX() {
  //printString(0,8,"reboot",2);
  printText(0, MAX_DEVICES-1, "reboot..");
  delay(1000);
  ESP.reset();
}


void youtube() {
  if(api.getChannelStatistics(CHANNEL_ID))
    {
      Serial.println("---YouTube-Stats------");
      Serial.print("Subscriber Count: ");
      Serial.println(api.channelStats.subscriberCount);
      Serial.print("View Count: ");
      Serial.println(api.channelStats.viewCount);
      Serial.print("Comment Count: ");
      Serial.println(api.channelStats.commentCount);
      Serial.print("Video Count: ");
      Serial.println(api.channelStats.videoCount);
      // Probably not needed :)
      //Serial.print("hiddenSubscriberCount: ");
      //Serial.println(api.channelStats.hiddenSubscriberCount);
      Serial.println("------------------------");
  }
}

void instagram() {
    InstagramUserStats response = instaStats.getUserStats(instagramName);
    Serial.println("---Instagram-Stats------");
    Serial.print("Insta-follower: ");
    Serial.println(response.followedByCount);
    int currentCount = response.followedByCount;
    if (currentCount > 0 ) {
        follower = currentCount;
     }
}

void getSensor() {
  // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  }
  else {
    Serial.print(F("Temperature: "));
    Serial.print(event.temperature);
    Serial.println(F("°C"));
  }
  // Get humidity event and print its value.
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    Serial.print(F("Humidity: "));
    Serial.print(event.relative_humidity);
    Serial.println(F("%"));
  }
}

void update_started() {
  //printString(0,8,"update …",2);
  printText(0, MAX_DEVICES-1, "update...");
  USE_SERIAL.println("CALLBACK:  HTTP update process started");
}

void update_finished() {
  //printString(0,8,"done",2);
  printText(0, MAX_DEVICES-1, "done");
  USE_SERIAL.println("CALLBACK:  HTTP update process finished");
}

void update_progress(int cur, int total) {
  char progressString[10];
  float percent = ((float)cur   / (float)total )  * 100;
  sprintf(progressString, " %s",  String(percent).c_str()  );
  //printString(0,8,progressString,2);
  printText(0, MAX_DEVICES-1, progressString);
  USE_SERIAL.printf("CALLBACK:  HTTP update process at %d of %d bytes...\n", cur, total);
}

void update_error(int err) {
  char errorString[8];
  sprintf(errorString, "Err %d", err);
  //printString(0,8,errorString,2);
  printText(0, MAX_DEVICES-1, errorString);
  USE_SERIAL.printf("CALLBACK:  HTTP update fatal error code %d\n", err);
}

void updateFirmware() {
     ESPhttpUpdate.setLedPin(LED_BUILTIN, LOW);
    // Add optional callback notifiers
    ESPhttpUpdate.onStart(update_started);
    ESPhttpUpdate.onEnd(update_finished);
    ESPhttpUpdate.onProgress(update_progress);
    ESPhttpUpdate.onError(update_error);

    t_httpUpdate_return ret = ESPhttpUpdate.update(updateclient, "http://firmware.kidbuild.de/ESP8266SocialCounter_2x.ino.nodemcu.bin");

    switch (ret) {
      case HTTP_UPDATE_FAILED:
        USE_SERIAL.printf("HTTP_UPDATE_FAILD Error (%d): %s\n", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
        break;

      case HTTP_UPDATE_NO_UPDATES:
        USE_SERIAL.println("HTTP_UPDATE_NO_UPDATES");
        break;

      case HTTP_UPDATE_OK:
        USE_SERIAL.println("HTTP_UPDATE_OK");
        break;
    }
}

void clockSymbol() {
  for (uint8_t i = 0; i < 8; i++)      // count of pixels of image (8)
    mx.setColumn(((MAX_DEVICES-1)*8)+i, clockImg[i]);   // position of image (0)
}

void tempSymbol() {
  for (uint8_t i = 0; i < 7; i++)     // count of pixels of image (8)
    mx.setColumn(((MAX_DEVICES-1)*8)+i, tempImg[i]);   // position of image (0)
}

void youtubeLogo() {
  if (MAX_DEVICES > 4 ) {
  for (uint8_t i = 0; i < 11; i++)        // count of pixels of image (8)
    mx.setColumn(29+i, youtubeImg[i]);    // position of image (0)
  } else {
    for (uint8_t i = 0; i < 11; i++)        // count of pixels of image (8)
    mx.setColumn(21+i, youtubeImg[i]);    // position of image (0)
  }
}

void instaLogo() {
  for (uint8_t i = 0; i < 8; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-1)*8+i, instaImg[i]);    // position of image (0)
}

void humidityIcon() {
  for (uint8_t i = 0; i < 6; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-1)*8+i, humiImg[i]);    // position of image (0)
}

void uncheckedIcon() {
  for (uint8_t i = 0; i < 8; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-3)*8+i, uncheckedImg[i]);    // position of image (0)
}

void checkedIcon() {
  for (uint8_t i = 0; i < 8; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-3)*8+i, checkedImg[i]);    // position of image (0)
}

void rebootIcon() {
  for (uint8_t i = 0; i < 7; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-1)*8+i, rebootImg[i]);    // position of image (0)
}

void downloadIcon() {
  for (uint8_t i = 0; i < 7; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-1)*8+i, downloadImg[i]);    // position of image (0)
}

void trashIcon() {
  for (uint8_t i = 0; i < 8; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-1)*8+i, trashImg[i]);    // position of image (0)
}

void infoIcon() {
  for (uint8_t i = 0; i < 4; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-1)*8+i, infoImg[i]);    // position of image (0)
}

void fadeSymbol() {
  for (uint8_t i = 0; i < 7; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-1)*8+i, fadeImg[i]);    // position of image (0)
}

void ghostSymbol() {
  for (uint8_t i = 0; i < 7; i++)        // count of pixels of image (8)
    mx.setColumn((MAX_DEVICES-1)*8+i, ghostImg[i]);    // position of image (0)
}

void printTime() {
  time_t now = time(nullptr);
  String time = String(ctime(&now));
  time.trim();
  
  if (MAX_DEVICES > 4 ) {
    String mvTime = "   ";
    mvTime = mvTime + time.substring(11,16);
    mvTime.toCharArray(time_value, 10);
    printText(0, MAX_DEVICES-1, time_value);
    clockSymbol();
  } else {
    time = time.substring(11,16);
    time.toCharArray(time_value, 10);
    printText(0, MAX_DEVICES-1, time_value);
  }
}

void printYoutubeFollower() {
    String move = "     ";
    String youtubecount = String(api.channelStats.subscriberCount);
    move = move + youtubecount;

    if (api.channelStats.subscriberCount > 0 ) {
      // If more then 4 modules are used
    if ( MAX_DEVICES > 4 ) {
         char copy[move.length()+1];
         move.toCharArray(copy, move.length()+1); 
         printText(0, MAX_DEVICES-1, copy);
         youtubeLogo();
        
    } else if ( api.channelStats.subscriberCount > 9999  ) {
        char copy[youtubecount.length()+1];
        youtubecount.toCharArray(copy, youtubecount.length()+1);
        printText(0, MAX_DEVICES-1, copy);
      } else {
        char copy[move.length()+1];
         move.toCharArray(copy, move.length()+1); 
         printText(0, MAX_DEVICES-1, copy);
         youtubeLogo();
      }
    }
}

void printCurrentFollower() {
    String move = "   ";
    String instacount = String(follower);
    move =  move + instacount;

    if (follower > 0 ) {
    // If more then 4 modules are used
    if (MAX_DEVICES > 4 ) {
        char copy[move.length()+1];
        move.toCharArray(copy, move.length()+1);
        printText(0, MAX_DEVICES-1, copy);
        instaLogo();
    } else if ( follower > 9999  ) {
        char copy[instacount.length()+1];
        instacount.toCharArray(copy, instacount.length()+1);
        printText(0, MAX_DEVICES-1, copy);
      } else {
        char copy[move.length()+1];
        move.toCharArray(copy, move.length()+1);
        printText(0, MAX_DEVICES-1, copy);
        instaLogo();
      }
    }
}

void printTemperature() {
      // Get temperature event and print its value.
  sensors_event_t event;
  dht.temperature().getEvent(&event);
  if (isnan(event.temperature)) {
    Serial.println(F("Error reading temperature!"));
  } else {
    // convert and put it on display
    String hitze = "   ";                           // define string
    hitze = hitze + String(event.temperature,1);    // convert to one digit after dot
    hitze = hitze + "^C";                           // add °C
      // If more then 4 modules are used
    if (MAX_DEVICES > 4 ) {
    char copy[hitze.length()+1];
    hitze.toCharArray(copy, hitze.length()+1);  
    printText(0, MAX_DEVICES-1, copy);
    tempSymbol();                           // put symbol on display
  } else {
    hitze = String(event.temperature,1);    // convert to one digit after dot
    hitze = hitze+"^C";                             // add °C
    char copy[hitze.length()+1];
    hitze.toCharArray(copy, hitze.length()+1);
    printText(0, MAX_DEVICES-1, copy);
  }
 }
}

void printHumidity() {
  // Get humidity event and print its value.
  sensors_event_t event;
  dht.humidity().getEvent(&event);
  if (isnan(event.relative_humidity)) {
    Serial.println(F("Error reading humidity!"));
  }
  else {
    // convert and put it on display
    String feuchte = "   ";                           // define string
    feuchte = feuchte + String(event.relative_humidity,1);    // convert to one digit after dot
    feuchte = feuchte+"%";                          // add %
    // If more then 4 modules are used
    if (MAX_DEVICES > 4 ) {
    char copy[feuchte.length()+1];
    feuchte.toCharArray(copy, feuchte.length()+1);  
    printText(0, MAX_DEVICES-1, copy);
    humidityIcon();                                 // put symbol on display
    } else {
      feuchte = String(event.relative_humidity,1);    // convert to one digit after dot
      feuchte = feuchte+"%";                          // add %
      char copy[feuchte.length()+1];
      feuchte.toCharArray(copy, feuchte.length()+1);  
      printText(0, MAX_DEVICES-1, copy);
    }
  } 
}

void settingsMenu(void) {
   
  // Menu System
  if ( menuActive == 1 ) {
    
    if ( menuPtr == 1 ) {
      mx.clear();
      humidityIcon();
      if ( strcmp (humiStat,"checked") == 0 ) {
          checkedIcon();
          if ( buttonLong == 1 ) {
            humiStat[0] = 0;
            Serial.print("humiStat: ");
            Serial.println(humiStat);
            buttonLong = 0;
            humidityIcon();
            uncheckedIcon();
          }
      } else if ( strcmp (humiStat,"") == 0 ) {
          uncheckedIcon();
          if ( buttonLong == 1 ) {
            humiStat[0] = 'c';
            humiStat[1] = 'h';
            humiStat[2] = 'e';
            humiStat[3] = 'c';
            humiStat[4] = 'k';
            humiStat[5] = 'e';
            humiStat[6] = 'd';
            humiStat[7] = 0;
            Serial.print("humiStat: ");
            Serial.println(humiStat);
            buttonLong = 0;
            humidityIcon();
            checkedIcon();
          }
      } 
}

    if ( menuPtr == 2 ) {
      mx.clear();
      tempSymbol();
      if ( strcmp (tempStat,"checked") == 0 ) {
          checkedIcon();
          if ( buttonLong == 1 ) {
            tempStat[0] = 0;
            buttonLong = 0;
            tempSymbol();
            uncheckedIcon();
          }
      } else if ( strcmp (tempStat,"") == 0 ) {
          uncheckedIcon();
          if ( buttonLong == 1 ) {
            tempStat[0] = 'c';
            tempStat[1] = 'h';
            tempStat[2] = 'e';
            tempStat[3] = 'c';
            tempStat[4] = 'k';
            tempStat[5] = 'e';
            tempStat[6] = 'd';
            tempStat[7] = 0;
            buttonLong = 0;
            tempSymbol();
            checkedIcon();
          }
      } 
    }

    if ( menuPtr == 3 ) {
      mx.clear();
      instaLogo();
      if ( strcmp (instaStat,"checked") == 0 ) {
          checkedIcon();
          if ( buttonLong == 1 ) {
            instaStat[0] = 0;
            buttonLong = 0;
            instaLogo();
            uncheckedIcon();
          }
      } else if ( strcmp (instaStat,"") == 0 ) {
          uncheckedIcon();
          if ( buttonLong == 1 ) {
            instaStat[0] = 'c';
            instaStat[1] = 'h';
            instaStat[2] = 'e';
            instaStat[3] = 'c';
            instaStat[4] = 'k';
            instaStat[5] = 'e';
            instaStat[6] = 'd';
            instaStat[7] = 0;
            buttonLong = 0;
            instaLogo();
            checkedIcon();
          }
      }
    }

    if ( menuPtr == 4 ) {
      mx.clear();
      youtubeLogo();
      if ( strcmp (youtuStat,"checked") == 0 ) {
          checkedIcon();
          if ( buttonLong == 1 ) {
            youtuStat[0] = 0;
            buttonLong = 0;
            youtubeLogo();
            uncheckedIcon();
          }
      } else if ( strcmp (youtuStat,"") == 0 ) {
          uncheckedIcon();
          if ( buttonLong == 1 ) {
            youtuStat[0] = 'c';
            youtuStat[1] = 'h';
            youtuStat[2] = 'e';
            youtuStat[3] = 'c';
            youtuStat[4] = 'k';
            youtuStat[5] = 'e';
            youtuStat[6] = 'd';
            youtuStat[7] = 0;
            buttonLong = 0;
            youtubeLogo();
            checkedIcon();
          }
      }      
    }

    if ( menuPtr == 5 ) {
      mx.clear();
      clockSymbol(); 
      if ( strcmp (clockStat,"checked") == 0 ) {
          checkedIcon();
          if ( buttonLong == 1 ) {
            clockStat[0] = 0;
            buttonLong = 0;
            clockSymbol(); 
            uncheckedIcon();
          }
      } else if ( strcmp (clockStat,"") == 0 ) {
          uncheckedIcon();
          if ( buttonLong == 1 ) {
            clockStat[0] = 'c';
            clockStat[1] = 'h';
            clockStat[2] = 'e';
            clockStat[3] = 'c';
            clockStat[4] = 'k';
            clockStat[5] = 'e';
            clockStat[6] = 'd';
            clockStat[7] = 0;
            buttonLong = 0;
            clockSymbol(); 
            checkedIcon();
          }
      }          
    }

    if ( menuPtr == 6 ) {
      mx.clear();
      fadeSymbol(); 
      if ( strcmp (fadeStat,"checked") == 0 ) {
          checkedIcon();
          if ( buttonLong == 1 ) {
            fadeStat[0] = 0;
            buttonLong = 0;
            fadeSymbol();
            uncheckedIcon();
          }
      } else if ( strcmp (fadeStat,"") == 0 ) {
          uncheckedIcon();
          if ( buttonLong == 1 ) {
            fadeStat[0] = 'c';
            fadeStat[1] = 'h';
            fadeStat[2] = 'e';
            fadeStat[3] = 'c';
            fadeStat[4] = 'k';
            fadeStat[5] = 'e';
            fadeStat[6] = 'd';
            fadeStat[7] = 0;
            buttonLong = 0;
            fadeSymbol(); 
            checkedIcon();
          }
      }          
    }

    if ( menuPtr == 7 ) {
      mx.clear();
      ghostSymbol(); 
      if ( strcmp (ghostStat,"checked") == 0 ) {
          checkedIcon();
          if ( buttonLong == 1 ) {
            ghostStat[0] = 0;
            buttonLong = 0;
            ghostSymbol(); 
            uncheckedIcon();
          }
      } else if ( strcmp (ghostStat,"") == 0 ) {
          uncheckedIcon();
          if ( buttonLong == 1 ) {
            ghostStat[0] = 'c';
            ghostStat[1] = 'h';
            ghostStat[2] = 'e';
            ghostStat[3] = 'c';
            ghostStat[4] = 'k';
            ghostStat[5] = 'e';
            ghostStat[6] = 'd';
            ghostStat[7] = 0;
            buttonLong = 0;
            ghostSymbol(); 
            checkedIcon();
          }
      }          
    }

    if ( menuPtr == 8 ) {
      mx.clear();
      rebootIcon();
      if ( buttonLong == 1 ) {
      buttonLong = 0;
      restartX();
      }
    }

    if ( menuPtr == 9 ) {
      mx.clear();
      downloadIcon();
      if ( buttonLong == 1 ) {
      buttonLong = 0;
      updateFirmware();
      }
    }

    if ( menuPtr == 10 ) {
      mx.clear();
      trashIcon();
      if ( buttonLong == 1 ) {
          buttonLong = 0;
          infoReset();
      }
    } 

    if ( menuPtr == 11 ) {
      mx.clear();
      infoIcon();
      if ( buttonLong == 1 ) {
          buttonLong = 0;
          infoWlan();
          infoIP();
          infoVersion();
          settingsMenu();
       }
    }
    
    if ( menuPtr == 12 ) {
      mx.clear();
      printText(0, MAX_DEVICES-1, "save?");
      if ( buttonLong == 1 ) {
      buttonLong = 0;
      menuActive = 0;
      //printString(0,8,"done...",2);
      printText(0, MAX_DEVICES-1, "done...");
      delay(1000);
      saveConfig();
      }
    }

    if ( menuPtr == 13 ) {
      mx.clear();
      printText(0, MAX_DEVICES-1, "back?");
      if ( buttonLong == 1 ) {
      buttonLong = 0;
      menuActive = 0;
      printText(0, MAX_DEVICES-1, "wait...");
      delay(1000);
      }
    }

    if ( menuPtr > 13 ) {
      menuPtr = 1;
      settingsMenu();
    }
 }
}
