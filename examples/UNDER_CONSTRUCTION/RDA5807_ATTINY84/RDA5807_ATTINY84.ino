/*
   Test and validation of RDA5807 on ATtiny84 device.
   It is FM receiver with  

   
   ATtiny84 and RDA5807 wireup  

    | RDA5807 Function | Attiny84 REF pin | Physical pin  | 
    | ---------------- | -----------------| ------------- | 
    | SEEK_UP          |     3            |    10         | 
    | SEEK_DOWN        |     5            |     8         |
    | ENCODER_PIN_A    |     0            |    13         |
    | ENCODER_PIN_B    |     1            |    12         |  
    | SDIO / SDA       |     SDA          |     7         |
    | SCLK / CLK       |     SCL          |     9         |


   By Ricardo Lima Caratti, 2020.
*/

#include <RDA5807.h>

// Please, check the ATtiny84 physical pins 

#define SEEK_UP             3       
#define SEEK_DOWN           5

  
RDA5807 rx;

void setup()
{
  pinMode(SEEK_UP, INPUT_PULLUP);
  pinMode(SEEK_DOWN, INPUT_PULLUP);
  rx.setup();
  rx.setFrequency(10650); // It is the frequency you want to select in MHz multiplied by 100.
}


void loop()
{

  if (digitalRead(SEEK_UP) == LOW ) 
    rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP);
  if (digitalRead(SEEK_DOWN) == LOW )
    rx.seek(RDA_SEEK_WRAP,RDA_SEEK_DOWN);

  delay(100);
 }
