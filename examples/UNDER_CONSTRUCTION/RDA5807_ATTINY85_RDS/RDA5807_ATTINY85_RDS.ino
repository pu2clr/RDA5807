/*
   Test and validation of RDA5807 on ATtiny85 device.
   It is a FM receiver with mini OLED and and two push buttons  
   
   ATtiny85 and RDA5807 wireup  

    | RDA5807 pin      | Attiny85 REF pin | Physical pin  | 
    | ----------------| -----------------| ------------- | 
    | SEEK_UP         |     PB1          |     6         | 
    | SEEK_DOWN       |     PB4          |     3         |
    | SDIO / SDA      |     SDA          |     5         |
    | SCLK / CLK      |     SCL          |     7         |
   

   By Ricardo Lima Caratti, 2020.
*/

#include <RDA5807.h>
#include <Tiny4kOLED.h>

#define SEEK_UP   PB1     
#define SEEK_DOWN PB4    

char *stationName;


RDA5807 rx;

void setup()
{
  pinMode(SEEK_UP, INPUT_PULLUP);
  pinMode(SEEK_DOWN, INPUT_PULLUP);

  oled.begin();
  oled.clear();
  oled.on();
  oled.setFont(FONT8X16);
  oled.setCursor(0, 0);
  oled.print(F("RDA5807-Attiny85"));
  oled.setCursor(0, 2);
  oled.print(F("   By PU2CLR   "));
  delay(3000);
  oled.clear();

  rx.setup();
  // rx.setVolume(8);   // Use it if necessary.
  rx.setFrequency(10390); 

  rx.setRDS(true);
  rx.setRdsFifo(true);

  showStatus();
 
}

void showStatus() {
  char faux[7];
  oled.setCursor(0, 0);
  oled.print(F("FM "));
  oled.setCursor(38, 0);
  oled.clearToEOL();
  oled.setCursor(38, 0);
  oled.print(rx.formatFrequency(rx.getRealFrequency(), faux, ','));
  oled.setCursor(95, 0);
  oled.print(F("MHz"));
  oled.setCursor(0, 2);
  oled.clearToEOL();
}

void loop()
{
  uint8_t key;
  key = digitalRead(SEEK_UP) << 1;
  key = key | digitalRead(SEEK_DOWN);
  switch
  if (digitalRead(SEEK_UP) == LOW ) {
    rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP, showStatus);
    showStatus();
  }
  if (digitalRead(SEEK_DOWN) == LOW ) {
    rx.seek(RDA_SEEK_WRAP,RDA_SEEK_DOWN, showStatus);
    showStatus();
  }
  if ( rx.getRdsReady() &&  rx.hasRdsInfo() && !rx.getErrorBlockB()) {
    stationName = rx.getRdsText0A();
    oled.setCursor(0, 2);
    if ( stationName != NULL ) {
        oled.print(stationName);  
    } else {
      oled.clearToEOL();
    }
    delay(80);
  }
  delay(5);
}
