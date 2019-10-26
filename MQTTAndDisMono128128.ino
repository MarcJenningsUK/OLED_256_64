/*
  SimpleMQTTClient.ino
  The purpose of this exemple is to illustrate a simple handling of MQTT and Wifi connection.
  Once it connects successfully to a Wifi network and a MQTT broker, it subscribe to a topic and send a message to it.
  It will also send a message delayed 5 seconds later.
*/

// add the header with local (secret) variables.
#include <Hayfield.h>

// Add weather icons
#include "weather.h"

// Add standard libraries
#include "EspMQTTClient.h"
#include <Arduino.h>
#include <U8g2lib.h>
#include <ArduinoJson.h>
//#include "weather-min-32.h"

#ifdef U8X8_HAVE_HW_SPI
#include <SPI.h>
#endif
#ifdef U8X8_HAVE_HW_I2C
#include <Wire.h>
#endif

//U8G2_SSD1322_NHD_256X64_1_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 5, /* reset=*/ 18);  // Enable U8G2_16BIT in u8g2.h
//U8G2_SSD1327_EA_W128128_F_4W_SW_SPI u8g2(U8G2_R0, /* clock=*/ 15, /* data=*/ 2, /* cs=*/ 4, /* dc=*/ 5, /* reset=*/ 18);

U8G2_SSD1322_NHD_256X64_F_4W_HW_SPI u8g2(U8G2_R0, /* cs=*/ 33, /* dc=*/ 4, /* reset=*/ 21);  // Enable U8G2_16BIT in u8g2.h

const String vno = "0.7.0";

const String unk = "unknown                           ";
int page_current = 0;
String media_title;
String media_artist;
String media_album;
String media_state;
String lounge_temp;
String lounge_humidity;
String front_door;
String motion;
String forecastIcon;
String forecastTemperature;
String forecastPrecipitation;
String forecastHumidity;

void u8g2_prepare(void) {
  //u8g2.setFont(u8g2_font_6x10_tf);
  u8g2.setFont(u8g2_font_helvB12_tf);
  u8g2.setFontRefHeightExtendedText();
  u8g2.setDrawColor(1);
  u8g2.setFontPosTop();
  u8g2.setFontDirection(0);
}

EspMQTTClient client(
  HayfieldSSID,
  HayfieldPSK,
  HayfieldMQTTServer,  // MQTT Broker server ip
  "",   // Can be omitted if not needed
  "",   // Can be omitted if not needed
  "TestClientiuwhecoiwec",     // Client name that uniquely identify your device
  1883              // The MQTT port, default to 1883. this line can be omitted
);


void SetPlaying(String state) {
  Serial.println("Setting play icon");
  //u8g2.setFont(u8g2_font_unifont_t_symbols);
  u8g2.setFont(u8g2_font_open_iconic_play_1x_t);

  u8g2.clearBuffer();  
  if(state.equals("playing")) {
    u8g2.drawGlyph(5, 0, 0x45);  /*play*/
  }
  else if(state.equals("paused")) {
    u8g2.drawGlyph(5, 0, 0x44);  /*pause*/
  }
  else {
    u8g2.drawGlyph(5, 0, 0x4b);  /*stop*/
  }
  
  u8g2.updateDisplayArea(0, 0, 2, 16/8);
}

void SetDateTime(String date) {
  u8g2.setFont(u8g2_font_profont12_tf);
  u8g2.clearBuffer();
  u8g2.drawStr(128, 0, date.c_str());
  u8g2.updateDisplayArea(128/8, 0, 128/8, 16/8);
}

void DrawPage() {
  u8g2.clearBuffer();  
  if(page_current==0) {
    // media title
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.drawStr(0, 20, media_title.c_str());

    //media artist
    u8g2.setFont(u8g2_font_helvR10_tf);
    u8g2.drawStr(0, 40, media_artist.c_str());
  }

  if(page_current==1) {
  
    // lounge temperature
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.drawStr(0, 30, "Lounge :" );

    u8g2.setFont(u8g2_font_helvR10_tf);
    u8g2.drawStr(128, 20, (lounge_temp + " C").c_str());
    // lounge humidity
    u8g2.drawStr(128, 40, (lounge_humidity + "%").c_str());
  }
  
  if(page_current==2) {
    u8g2.setFont(u8g2_font_helvB10_tf);
    u8g2.drawStr(0, 20, "Front door :" );
    u8g2.drawStr(0, 40, "Motion :" );

    u8g2.setFont(u8g2_font_helvR10_tf);
    // front door contact status
    u8g2.drawStr(128, 20, front_door.c_str());

    // motion sensor activity
    u8g2.drawStr(128, 40, motion.c_str());
  }

  if(page_current == 3) {
    u8g2.setFont(u8g2_font_helvR10_tf);
    u8g2.drawStr(0, 20, forecastIcon.c_str() );
    u8g2.drawStr(0, 40,forecastTemperature.c_str() );
    u8g2.drawStr(128, 20, forecastPrecipitation.c_str() );
    u8g2.drawStr(128, 40, forecastHumidity.c_str() );
  }
  
  if(page_current == 4) {
    u8g2.clearBuffer();  
    u8g2.setFont(u8g2_font_helvR24_tf);
    u8g2.drawStr(0, 20, "Version ");
    u8g2.drawStr(128, 20, vno.c_str() );
  }
  
  if(page_current == 5) {
    u8g2.clearBuffer(); 
    u8g2.drawXBM( 0, 14, u8g_logo_width, u8g_logo_height, gImage_B);
    u8g2.drawXBM( 64, 14, u8g_logo_width, u8g_logo_height, gImage_C);
    u8g2.drawXBM( 128, 14, u8g_logo_width, u8g_logo_height, gImage_D);
    u8g2.drawXBM( 196, 14, u8g_logo_width, u8g_logo_height, gImage_E);
  }
  
  if(page_current == 6) {
    u8g2.clearBuffer(); 
    u8g2.drawXBM( 0, 14, u8g_logo_width, u8g_logo_height, gImage_F);
    u8g2.drawXBM( 64, 14, u8g_logo_width, u8g_logo_height, gImage_G);
    u8g2.drawXBM( 128, 14, u8g_logo_width, u8g_logo_height, gImage_H);
    u8g2.drawXBM( 196, 14, u8g_logo_width, u8g_logo_height, gImage_I);
  }
  
  if(page_current == 7) {
    u8g2.clearBuffer(); 
    u8g2.drawXBM( 0, 14, u8g_logo_width, u8g_logo_height, gImage_J);
    u8g2.drawXBM( 64, 14, u8g_logo_width, u8g_logo_height, gImage_K);
    u8g2.drawXBM( 128, 14, u8g_logo_width, u8g_logo_height, gImage_L);
    u8g2.drawXBM( 196, 14, u8g_logo_width, u8g_logo_height, gImage_M);
  }
  
  if(page_current == 8) {
    u8g2.clearBuffer(); 
    u8g2.drawXBM( 0, 14, u8g_logo_width, u8g_logo_height, gImage_N);
    u8g2.drawXBM( 64, 14, u8g_logo_width, u8g_logo_height, gImage_O);
    u8g2.drawXBM( 128, 14, u8g_logo_width, u8g_logo_height, gImage_a);
  }
  
  
  u8g2.updateDisplayArea(0, 16/8, 256/8, 40/8);
}

void setup()
{
  u8g2.begin();
  Serial.begin(9600);

  // Optionnal functionnalities of EspMQTTClient : 
  //client.enableDebuggingMessages(); // Enable debugging messages sent to serial output
  //client.enableHTTPWebUpdater(); // Enable the web updater. User and password default to values of MQTTUsername and MQTTPassword. These can be overrited with enableHTTPWebUpdater("user", "password").
  //client.enableLastWillMessage("TestClient/lastwill", "I am going offline");  // You can activate the retain flag by setting the third parameter to true
  
  u8g2_prepare();
  
  u8g2.setColorIndex(1);
  u8g2.setFontMode(1);
  u8g2.setDrawColor(2);
  u8g2.clearBuffer();  
  u8g2.drawStr(0, 24, "Loading...");
  u8g2.drawStr(0, 40, "Just a sec.");

  u8g2.sendBuffer();  
}


// This function is called once everything is connected (Wifi and MQTT)
// WARNING : YOU MUST IMPLEMENT IT IF YOU USE EspMQTTClient
void onConnectionEstablished()
{
  // Subscribe to "mytopic/test" and display received message to Serial
  client.subscribe("ESP32/Media", [](const String & payload) {
    Serial.println(payload);
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if(error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    
    media_title = doc["title"].as<String>();
    media_artist = doc["artist"].as<String>();
    media_album = doc["album"].as<String>();
    DrawPage();

    String newState = doc["state"].as<String>();
    if(media_state != newState) {
      media_state = newState;
      SetPlaying(media_state);
    }
  });
  
//  client.subscribe("ESP32/MediaState", [](const String & payload) {
//    Serial.println(payload);
//    Serial.println(payload.substring(0,32));
//    SetPlaying(payload.substring(0,32));
//  });


  client.subscribe("ESP32/shown", [](const String & payload) {
    Serial.print("Switching to page #");
    Serial.println(payload);

    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if(error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    int pgnum = doc["page"];
    
    page_current = pgnum; //payload.toInt();
    DrawPage();
  });
  
  client.subscribe("ESP8266/Temperature", [](const String & payload) {
    Serial.println(payload);
    lounge_temp = payload.substring(0, 50);
    DrawPage();
  });
  
  client.subscribe("ESP8266/Humidity", [](const String & payload) {
    Serial.println(payload);
    lounge_humidity = payload.substring(0, 50);
    DrawPage();
  });

  client.subscribe("smartthings/Multipurpose Sensor/contact", [](const String & payload) {
    Serial.println(payload);
    front_door = payload.substring(0, 10);
    DrawPage();
  });
  
  client.subscribe("smartthings/Motion Sensor/motion", [](const String & payload) {
    Serial.println(payload);
    motion = payload.substring(0, 10);
    DrawPage();
  });
  
  client.subscribe("ESP32/datetimeshorter", [](const String & payload) {
    Serial.println(payload);
    Serial.println(payload.substring(0,50));
    SetDateTime(payload.substring(0,50));
  });
  
  client.subscribe("ESP32/forecast", [](const String & payload) {
    Serial.println(payload);
    
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, payload);
    if(error) {
      Serial.print(F("deserializeJson() failed: "));
      Serial.println(error.c_str());
      return;
    }
    String icon = doc["icon"];
    String temp = doc["temp"];
    String humidity = doc["humidity"];
    String precip = doc["precip"];
    
    forecastIcon = icon; 
    forecastTemperature = temp;
    forecastHumidity = humidity;
    forecastPrecipitation = precip;
    
    DrawPage();
  });
  
  SetPlaying(unk.c_str());

  for(int i=5; i < 9; i++) {
    page_current = i;
    DrawPage();
    delay(1000);
  }
  
  page_current = 0;
  DrawPage();
}

int incomingByte = 0;

void loop()
{
//      client.publish("ESP32/MediaPlayPause", "pause");
  
  client.loop();
}
