/*

  WiFlyRSSIGraph.ino
  http://www.semifluid.com
  
  Copyright (c) 2014, Steven A. Cholewiak
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
  ================
*/

#include <WiFlyHQ.h>
#include <SoftwareSerial.h>
SoftwareSerial debugSerial(2,3);

WiFly WiFly;

// External SECRET.ino file with the network details:
/* Change these to match your WiFi network */
//const char mySSID[] = "SSID";
//const char myPassword[] = "PASSWORD";
extern const char mySSID[];
extern const char myPassword[];

#include <U8glib.h>

byte ledBacklight = 100;
const byte lcdLED = 6;                   // LED Backlight
const byte lcdA0 = 7;                    // Data and command selections. L: command  H : data
const byte lcdRESET = 8;                 // Low reset
const byte lcdCS = 9;                    // SPI Chip Select (internally pulled up), active low
const byte lcdMOSI = 11;                 // SPI Data transmission
const byte lcdSCK = 13;                  // SPI Serial Clock

const byte lcdWidth = 128;
const byte lcdHeight = 64;

const byte numOfSamples = lcdWidth;
unsigned int RSSIreadings[numOfSamples];
const boolean linesNotDots = true;

// SW SPI:
//U8GLIB_MINI12864_2X u8g(lcdSCK, lcdMOSI, lcdCS, lcdA0, lcdRESET);
// HW SPI:
U8GLIB_MINI12864_2X u8g(lcdCS, lcdA0, lcdRESET);

void collectData(void) {
  for (int i=0; i<numOfSamples-1; i++) // Draw using lines
    RSSIreadings[i] = RSSIreadings[i+1];//RSSIreadings[i];
  RSSIreadings[numOfSamples-1] = abs(WiFly.getRSSI());
}

void print2CenteredStrings(char* theStringA, char* theStringB) {
  u8g.setFontPosTop();          // Set upper left position for the string draw procedure
  int h = u8g.getFontAscent() - u8g.getFontDescent();
  int wA = u8g.getStrWidth(theStringA);
  int wB = u8g.getStrWidth(theStringB);
  u8g.drawStr(lcdWidth/2-wA/2, lcdHeight/2-h, theStringA);
  u8g.drawStr(lcdWidth/2-wB/2, lcdHeight/2+1, theStringB);
}

void draw2CenteredStrings(char* theStringA, char* theStringB) {
  u8g.firstPage(); 
  do {
    print2CenteredStrings(theStringA,theStringB);
  } 
  while( u8g.nextPage() );
}


void showData(void) {
  if (linesNotDots == true) {
    for (int i=1; i<numOfSamples; i++) // Draw using lines
      u8g.drawLine(i-1,RSSIreadings[i-1]>>1,i,RSSIreadings[i]>>1);
  } 
  else {
    for (int i=0; i<numOfSamples; i++) // Draw using points
      u8g.drawPixel(i,RSSIreadings[i]>>1);
  }
  
  // Show RSSI val
  char buf[32];
  sprintf(buf, "-%02i", RSSIreadings[numOfSamples-1]);
  int h = u8g.getFontAscent() - u8g.getFontDescent();
  int w = u8g.getStrWidth(buf);
  u8g.drawStr(128-w, (RSSIreadings[numOfSamples-1]>>1)+1, buf);
}

void draw(void) {
  u8g.firstPage(); 
  do {
    showData();
  } 
  while( u8g.nextPage() );
}

void setup() {
  char buf[32];

  // Turn on LED backlight
  analogWrite(lcdLED, ledBacklight);

  // Setup U8glib
  u8g.begin();
  u8g.setFont(u8g_font_5x7);    // Set font for the console window
  draw2CenteredStrings("WiFlyRSSIGraph","www.semifluid.com");
  delay(500);

  Serial.begin(9600);
  
  while (!WiFly.begin(&Serial, &debugSerial)) {
    draw2CenteredStrings("!!! ERROR !!!", "Failed to start WiFly");
  }

  if (!WiFly.isAssociated()) {
    strcpy(buf,mySSID);
    draw2CenteredStrings("Joining network",buf);
    WiFly.setSSID(mySSID);
    WiFly.setPassphrase(myPassword);
    WiFly.enableDHCP();

    if (WiFly.join()) {
      WiFly.getSSID(buf, sizeof(buf));
      draw2CenteredStrings("Joined wifi network:",buf);
    } 
    else {
      draw2CenteredStrings("!!! ERROR !!!", "Failed to join wifi net");
    }
  } 
  else {
    WiFly.getSSID(buf, sizeof(buf));
    draw2CenteredStrings("Already joined network:",buf);
  }

  WiFly.setDeviceID("WiFly");
  
  WiFly.getSSID(buf, sizeof(buf));
  draw2CenteredStrings("SSID:",buf);
  delay(500);
  WiFly.getDeviceID(buf, sizeof(buf));
  draw2CenteredStrings("DeviceID:",buf);
  delay(500);
  WiFly.getIP(buf, sizeof(buf));
  draw2CenteredStrings("IP Address:",buf);
  delay(500);
  WiFly.getNetmask(buf, sizeof(buf));
  draw2CenteredStrings("Netmask Address:",buf);
  delay(500);
  WiFly.getGateway(buf, sizeof(buf));
  draw2CenteredStrings("Gateway Address:",buf);
  delay(500);
  WiFly.getMAC(buf, sizeof(buf));
  draw2CenteredStrings("MAC Address:",buf);
  delay(500);

  WiFly.setTimeAddress("129.6.15.30"); // time-c.nist.gov
  WiFly.setTimePort(123);
  WiFly.setTimezone(22);  // CEST UTC + 2 hours
  WiFly.setTimeEnable(5);
  WiFly.time();

  if (WiFly.isConnected()) {
    draw2CenteredStrings("Still Connected","Closing Old Connection");
    WiFly.close();
  }
  
  delay(100);
}

void loop() {
  collectData();
  draw();
}


