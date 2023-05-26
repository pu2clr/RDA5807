/*
  Test and validation of RDA5807 on ATtiny85 device  using I2C OLED 91" (128x32).
  It is a FM receiver with mini OLED and and two push buttons  
   
  ATtiny85 and RDA5807 wireup  

  | RDA5807 pin     | Attiny85 REF pin | Physical pin  | 
  | ----------------| -----------------| ------------- | 
  | SEEK_UP         |     PB1          |     6         | 
  | SEEK_DOWN       |     PB4          |     3         |
  | SDIO / SDA      |     SDA          |     5         |
  | SCLK / CLK      |     SCL          |     7         |
   
  See the RDA5807_ATTINY85_RDS  

  1) Install the ATtiny Core in Arduino IDE - Insert the URL http://drazzy.com/package_drazzy.com_index.json on board manager. 
                                               To do that, go to Preferences, enter the above URL in "Additional Boards Manager URLs. 
                                               To setup ATtiny85 on Arduino IDE, go to Tools Menu, Board, Board Manager and install 
                                               "ATTinyCore by Spence Konde". 
  Board setup: Select Chip = ATtiny85;  Clock Source = 4MHz (Internal); LTO = Enabled; millis() / macros() = Enabled; 
  ATTENTION: if you select Clock source 8 MHz, for some reason, the system will work very slow. Maybe a bug. Not sure. 


  By Ricardo Lima Caratti, 2023.
*/

#include <RDA5807.h>
#include <Tiny4kOLED.h>

#define SEEK_UP   PB1     
#define SEEK_DOWN PB4    

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
  showStatus();
 
}

void showStatus() {
  oled.setCursor(0, 0);
  oled.print(F("FM "));
  oled.setCursor(38, 0);
  oled.print(F("      "));
  oled.setCursor(38, 0);
  oled.print(rx.formatCurrentFrequency());
  oled.setCursor(95, 0);
  oled.print(F("MHz"));
}

void loop()
{
  if (digitalRead(SEEK_UP) == LOW ) {
    rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP, showStatus);
    showStatus();
  }
  if (digitalRead(SEEK_DOWN) == LOW ) {
    rx.seek(RDA_SEEK_WRAP,RDA_SEEK_DOWN, showStatus);
    showStatus();
  }
  delay(100);
}
