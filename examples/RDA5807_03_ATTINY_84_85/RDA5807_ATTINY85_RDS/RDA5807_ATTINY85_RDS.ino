/*
   Test and validation of RDA5807 on ATtiny85 device.
   It is a FM receiver with mini OLED and and two push buttons.
   This sketch implements FM RDS function.  
   
   ATtiny85 and RDA5807 wireup  

    | RDA5807 pin      | Attiny85 REF pin | Physical pin  | 
    | ----------------| -----------------| ------------- | 
    | SEEK_UP         |     PB1          |     6         | 
    | SEEK_DOWN       |     PB4          |     3         |
    | SDIO / SDA      |     SDA          |     5         |
    | SCLK / CLK      |     SCL          |     7         |
   
    Compiling and uploading: 
    1) Install the ATtiny Core in Arduino IDE - Insert the URL http://drazzy.com/package_drazzy.com_index.json on board manager. 
                                                To do that, go to Preferences, enter the above URL in "Additional Boards Manager URLs. 
                                                To setup ATtiny85 on Arduino IDE, go to Tools Menu, Board, Board Manager and install 
                                                "ATTinyCore by Spence Konde". 
    2) Setup: Chip: ATtiny85;  Clock Source: 4MHz (Internal); LTO Enabled; millis() / macros() Enabled; 

    ATTENTION: if you select Clock source 8 MHz for some reason the system will work very slow. Maybe a bug. 

   By Ricardo Lima Caratti, 2023.
*/

#include <RDA5807.h>
#include <EEPROM.h>
#include <Tiny4kOLED.h>

#define SEEK_UP   PB1     
#define SEEK_DOWN PB4    

#define VALID_DATA 85

char *stationName;
uint16_t currentFrequency;

RDA5807 rx;

void setup()
{
  pinMode(SEEK_UP, INPUT_PULLUP);
  pinMode(SEEK_DOWN, INPUT_PULLUP);

  oled.begin();
  oled.clear();
  oled.on();
  oled.setFont(FONT8X16);
  // Remove the Splash if you want.
  // Begin Splash
  oled.setCursor(0, 0);
  oled.print(F("RDA5807-Attiny85"));
  oled.setCursor(0, 2);
  oled.print(F("   By PU2CLR   "));
  delay(2000);
  oled.clear();

  // End Splash
  rx.setup();
  rx.setVolume(8);  

  // Restore the latest frequency saved into the EEPROM
  if (EEPROM.read(0) == VALID_DATA ) {
    currentFrequency = EEPROM.read(1) << 8;
    currentFrequency |= EEPROM.read(2);
  } else {
    currentFrequency = 10390; // default value
  } 
  rx.setFrequency(currentFrequency); 
  rx.setRDS(true);
  rx.setRdsFifo(true);
  showStatus();
}

void showStatus() {
  char faux[8];
  oled.setCursor(0, 0);
  oled.print(F("FM "));
  oled.setCursor(38, 0);
  oled.clearToEOL();
  oled.setCursor(38, 0);
  oled.print(rx.formatCurrentFrequency(faux, ',')); 
  oled.setCursor(95, 0);
  oled.print(F("MHz"));
  oled.setCursor(0, 2);
  oled.clearToEOL();
}


void loop()
{
  uint8_t bkey;
  bkey = digitalRead(SEEK_UP) << 1;     // Checks Seek Up push button
  bkey = bkey | digitalRead(SEEK_DOWN); // Checks Seek Down push button
  if ( bkey != 0b11) { // if one of them is pressed
    if (bkey == 0b01) 
      rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP, showStatus);
    else
      rx.seek(RDA_SEEK_WRAP,RDA_SEEK_DOWN, showStatus);
    showStatus();
    delay(200);
    // Saves the current frequency if it has changed. 
    currentFrequency = rx.getFrequency();
    EEPROM.update(0, VALID_DATA); // Says that a valid frequency will be saved  
    EEPROM.update(1, currentFrequency  >> 8);   // stores the current Frequency HIGH byte 
    EEPROM.update(2, currentFrequency & 0xFF);  // stores the current Frequency LOW byte 
  }

  if ( rx.getRdsReady() &&  rx.hasRdsInfo() && rx.getRdsFlagAB() == 0 )  {
    stationName = rx.getRdsText0A();
    oled.setCursor(0, 2);
    if ( stationName != NULL ) 
        oled.print(stationName); 
    else 
      oled.clearToEOL();
    delay(70);
  }
  delay(5);
}
