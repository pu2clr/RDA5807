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
#include <Tiny4kOLED.h>


// Please, check the ATtiny84 physical pins 

#define SEEK_UP             3       
#define SEEK_DOWN           5
#define ENCODER_PIN_A       0
#define ENCODER_PIN_B       1  
  
unsigned char encoder_pin_a;
unsigned char encoder_prev = 0;
unsigned char encoder_pin_b;
long elapsedTimeEncoder = millis();

RDA5807 rx;

void setup()
{
  pinMode(SEEK_UP, INPUT_PULLUP);
  pinMode(SEEK_DOWN, INPUT_PULLUP);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  oled.begin();
  oled.clear();
  oled.on();
  oled.setFont(FONT8X16);
  oled.setCursor(0, 0);
  oled.print(F("RDA5807-Attiny84A"));
  oled.setCursor(0, 2);
  oled.print(F("   By PU2CLR   "));
  delay(3000);
  oled.clear();

  rx.setup();
  rx.setVolume(6);
  rx.setFrequency(10650); // It is the frequency you want to select in MHz multiplied by 100.
  showStatus();
}

void showStatus() {
  oled.setCursor(0, 0);
  oled.print(F("FM "));
  oled.setCursor(38, 0);
  oled.print("      ");
  oled.setCursor(38, 0);
  oled.print(rx.formatCurrentFrequency());
  oled.setCursor(95, 0);
  oled.print(F("MHz"));
}

void loop()
{
  if ((millis() - elapsedTimeEncoder) > 5)
  {
    encoder_pin_a = digitalRead(ENCODER_PIN_A);
    encoder_pin_b = digitalRead(ENCODER_PIN_B);
    if ((!encoder_pin_a) && (encoder_prev)) // has ENCODER_PIN_A gone from high to low?
    {                                       // if so,  check ENCODER_PIN_B. It is high then clockwise (1) else counter-clockwise (-1)
      if (encoder_pin_b) 
         rx.setFrequencyUp();
      else
         rx.setFrequencyDown();
      showStatus();   
    }
    encoder_prev = encoder_pin_a;
    elapsedTimeEncoder = millis(); // keep elapsedTimeEncoder updated
  }
 
  if (digitalRead(SEEK_UP) == LOW ) {
    rx.seek(RDA_SEEK_WRAP,RDA_SEEK_UP, showStatus);
    showStatus();
  }
  if (digitalRead(SEEK_DOWN) == LOW ) {
    rx.seek(RDA_SEEK_WRAP,RDA_SEEK_DOWN, showStatus);
    showStatus();
  }
  delay(1);
 }
