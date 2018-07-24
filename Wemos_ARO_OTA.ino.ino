
#include "FastLED.h"
#include <ArduinoOSC.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <elapsedMillis.h>



const IPAddress ip(192, 168, 1, 253);
const char* host = "192.168.1.7";
const int recv_port = 6969;
const int send_port = 16969;
const int DEBUG_ON=0;
const int ID = 2;



// How many leds in your strip?
#define NUM_LEDS 12

// For led chips like Neopixels, which have a data line, ground, and power, you just
// need to define DATA_PIN.  For led chipsets that are SPI based (four wires - data, clock,
// ground, and power), like the LPD8806 define both DATA_PIN and CLOCK_PIN
#define DATA_PIN 0
//#define CLOCK_PIN 13

// Define the array of leds
CRGB leds[NUM_LEDS];
int contador = 0;
int hue = 0;
int globalR=255;
int globalG=0;
int globalB=0;
elapsedMillis timeLastPing;

//--------------------------------------------------------------
WiFiUDP udp;
ArduinoOSCWiFi osc;
const char* ssid = "EysebergLab";
const char* pwd = "perropanda";
const IPAddress gateway(192, 168, 1, 1);
const IPAddress subnet(255, 255, 255, 0);

void ping()
{
    if ( timeLastPing > 5000)
    {
      OSCMessage msg;
      //set destination ip address & port no
      msg.beginMessage(host, send_port);
      //set argument
      msg.setOSCAddress("/PING");
      msg.addArgInt32(ID);
      osc.send(msg);
      timeLastPing = 0;
      if (DEBUG_ON)
        Serial.println("ping");
    }
}

void callback(OSCMessage& m)
{
  /*
    //create new osc message
    OSCMessage msg;
    //set destination ip address & port no
    msg.beginMessage(host, send_port);
    //set argument
    msg.setOSCAddress(m.getOSCAddress());
    msg.addArgInt32(m.getArgAsInt32(0));
    msg.addArgInt32(m.getArgAsInt32(1));
    msg.addArgInt32(m.getArgAsInt32(2));
    osc.send(msg);
    */
    String address = m.getOSCAddress();
    if (DEBUG_ON)
    {
      Serial.print("#");
      Serial.print(address);
      Serial.println("#");
    }    
    if (address == "/RGB")
    {
      globalR = m.getArgAsInt32(0);
      globalG = m.getArgAsInt32(1);
      globalB = m.getArgAsInt32(2);

      if (DEBUG_ON)
      {
        Serial.println("RECIBIDO RGB");
        Serial.println(m.getArgAsInt32(0));
        Serial.println(m.getArgAsInt32(1));
        Serial.println(m.getArgAsInt32(2));
      }
    }
}

void setupWifi()
{
    if (DEBUG_ON)
      Serial.begin(115200);
    
    WiFi.disconnect(true);
    WiFi.begin(ssid, pwd);
    WiFi.config(ip, gateway, subnet);

    for (int i = 0 ; i < NUM_LEDS ; i=i+2)
      leds[i]= CRGB::Red;
    FastLED.show();
    int ini = 1;
    
    while (WiFi.waitForConnectResult() != WL_CONNECTED)
    {  
      for (int i = ini ; i < NUM_LEDS ; i=i+2)
        leds[i]= CRGB::Green;
      ini++;
      ini = ini % 2;
      FastLED.show();
      if (DEBUG_ON)
         Serial.println("Connection Failed! Rebooting...");
      delay(5000);
      ESP.restart();
    }

    osc.begin(udp, recv_port);
    osc.addCallback("/RGB", &callback);
    
    //ArduinoOTA.setHostname("wemos_1");
    
    ArduinoOTA.onStart([]() {
      String type;
      if (ArduinoOTA.getCommand() == U_FLASH)
        type = "sketch";
      else // U_SPIFFS
        type = "filesystem";
  
      // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
      if (DEBUG_ON)
        Serial.println("Start updating " + type);
    });
    ArduinoOTA.onEnd([]() {
      if (DEBUG_ON)
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      if (DEBUG_ON)
          Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
       if (DEBUG_ON)
       {
        Serial.printf("Error[%u]: ", error);
       
        if (error == OTA_AUTH_ERROR) Serial.println("Auth Failed");
        else if (error == OTA_BEGIN_ERROR) Serial.println("Begin Failed");
        else if (error == OTA_CONNECT_ERROR) Serial.println("Connect Failed");
        else if (error == OTA_RECEIVE_ERROR) Serial.println("Receive Failed");
        else if (error == OTA_END_ERROR) Serial.println("End Failed");
       }
    });
    ArduinoOTA.begin();
    if (DEBUG_ON)
    {
      Serial.println("Ready");
      Serial.print("IP address: ");
      Serial.println(WiFi.localIP());
    }    
}


void setup() { 
  
  FastLED.addLeds<WS2812B, DATA_PIN, GRB>(leds, NUM_LEDS);
  for (int i = 0 ; i < NUM_LEDS ; i++)
    leds[i] = CRGB::Black;

  setupWifi();

}


void dibujaCircular()
{
  for (int i = 0; i < 12 ; i++)
    leds[i] = CRGB::Black;

  //leds[contador]= CHSV(hue++,255,255);
  leds[contador]= CRGB(globalR,globalG,globalB);
  contador++;
  if (contador ==  NUM_LEDS)
    contador = 0;
  delay(30);
  FastLED.show();
  
  }

  
void loop() { 
  ArduinoOTA.handle();
  osc.parse();
  dibujaCircular();
  ping();
  return;

  
  // Turn the LED on, then pause
  contador++;
  leds[contador]=CRGB::Red;
  
  for (int i = 0; i < 12 ; i++)
  {
    leds[i] = CHSV(hue++,255,255);
    FastLED.show();
    delay(20);
  }
  //delay(500);
  // Now turn the LED off, then pause
  for (int i = 0; i < 12 ; i++)
  {
    leds[i] = CRGB::Black;
  }
  FastLED.show();
}


