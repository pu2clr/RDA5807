/*
  Under construction... 
  Test and validation of RDA5807 on UNO/NANO or other ATMEGA328 based device.
  It is FM receiver with  
  
   Nano and RDA5807 wireup  

  Wire up on Arduino UNO, Nano or Pro mini

  | Device name               | Device Pin / Description  |  Arduino Pin  |
  | --------------------------| --------------------      | ------------  |
  | OLED - I2C                |                           |               |
  |                           |  SDA                      |     A4        | 
  |                           |  SCLK                     |     A5        |  
  | --------------------------| ------------------------- | --------------|
  | RDA5807                   |                           |               | 
  |                           | SDA/SDIO                  |     A4        |
  |                           | SCLK (Clock)              |     A5        |
  | --------------------------| --------------------------| --------------|
  | Buttons                   |                           |               |
  |                           | Volume Up                 |      8        |
  |                           | Volume Down               |      9        |
  |                           | Mute                      |     10        | 
  |                           | Bass                      |     11        |
  |                           | SEEK (encoder button)     |     D14/A0    |
  | --------------------------| --------------------------|---------------| 
  | Encoder                   |                           |               |
  |                           | A                         |       2       |
  |                           | B                         |       3       |

   By Ricardo Lima Caratti, 2020.
*/

#include <RDA5807.h>
#include <Tiny4kOLED.h>
#include "Rotary.h"

// Please, check the ATtiny84 physical pins 

#define VOLUME_UP           8       
#define VOLUME_DOWN         9
#define AUDIO_MUTE         10
#define AUDIO_BASS         11
#define SEEK_STATION       14 // A0 = D14

// Enconder PINs
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

volatile int encoderCount = 0;

long elapsedTimeEncoder = millis();

// Encoder control
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

RDA5807 rx;

void setup()
{
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  // Push button pin
  pinMode(VOLUME_UP, INPUT_PULLUP);
  pinMode(VOLUME_DOWN, INPUT_PULLUP);
  pinMode(AUDIO_MUTE, INPUT_PULLUP);
  pinMode(AUDIO_BASS, INPUT_PULLUP);
  pinMode(SEEK_STATION, INPUT_PULLUP);

  oled.begin();
  oled.clear();
  oled.on();
  oled.setFont(FONT8X16);
  oled.setCursor(0, 0);
  oled.print("RDA5807-OLED");
  oled.setCursor(0, 2);
  oled.print("   By PU2CLR   ");
  delay(3000);
  oled.clear();

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);


  rx.setup();
  rx.setVolume(6);
  rx.setFrequency(10650); // It is the frequency you want to select in MHz multiplied by 100.
  showStatus();
}

/*
    Reads encoder via interrupt
    Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
*/
void rotaryEncoder()
{ // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
    encoderCount = (encoderStatus == DIR_CW) ? 1 : -1;
}


void showStatus() {
  oled.setCursor(0, 0);
  oled.print("FM ");
  oled.setCursor(38, 0);
  oled.print("      ");
  oled.setCursor(38, 0);
  oled.print(rx.getRealFrequency() / 100.0);
  oled.setCursor(95, 0);
  oled.print("MHz");
}

void loop()
{


  delay(1);
 }
