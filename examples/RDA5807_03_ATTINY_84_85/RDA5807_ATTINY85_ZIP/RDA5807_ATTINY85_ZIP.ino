/*

This sketch uses two buttons to find stations. It is a FM receiver without visual interface. 
   
ATtiny85 and RDA5807 wireup  

| RDA5807 pin      | Attiny85 REF pin | Physical pin | 
| ----------------| -----------------| ------------- | 
| SEEK_UP         |     PB1          |     6         | 
| SEEK_DOWN       |     PB4          |     3         |
| SDIO / SDA      |     SDA          |     5         |
| SCLK / CLK      |     SCL          |     7         |

1) Install the ATtiny Core in Arduino IDE - Insert the URL http://drazzy.com/package_drazzy.com_index.json on board manager. 
                                            To do that, go to Preferences, enter the above URL in "Additional Boards Manager URLs. 
                                            To setup ATtiny85 on Arduino IDE, go to Tools Menu, Board, Board Manager and install 
                                            "ATTinyCore by Spence Konde". 
Board setup: Select Chip = ATtiny85;  Clock Source = 4MHz (Internal); LTO = Enabled; millis() / macros() = Enabled; 
ATTENTION: if you select Clock source 8 MHz, for some reason, the system will work very slow. Maybe a bug. Not sure. 

By Ricardo Lima Caratti, 2020.
*/

#include <RDA5807.h>
#define SEEK_UP   PB1     
#define SEEK_DOWN PB4    
RDA5807 rx;
void setup()
{
  pinMode(SEEK_UP, INPUT_PULLUP);
  pinMode(SEEK_DOWN, INPUT_PULLUP);
  rx.setup();
  rx.setFrequency(10390); 
}

void loop()
{
  if (digitalRead(SEEK_UP) == LOW )  rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP);
  if (digitalRead(SEEK_DOWN) == LOW ) rx.seek(RDA_SEEK_WRAP,RDA_SEEK_DOWN);
  delay(100);
}
