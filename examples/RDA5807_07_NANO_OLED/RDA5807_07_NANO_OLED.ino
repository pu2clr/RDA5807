/*
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
  |                           | Volume Up                 |      4        |
  |                           | Volume Down               |      5        |
  |                           | Mute                      |      6         |
  |                           | Bass                      |      7        |
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

#define VOLUME_UP 4
#define VOLUME_DOWN 5
#define AUDIO_MUTE  6
#define AUDIO_BASS  7
#define SEEK_STATION 14 // A0 = D14

// Enconder PINs
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

bool bBass = false;
bool bMute = false;

volatile int encoderCount = 0;
uint8_t seekDirection = 1; // 0 = Down; 1 = Up. This value is set by the last encoder direction.
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

void showStatus()
{
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
  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if (encoderCount == 1)
    {
      rx.setFrequencyUp();
      seekDirection = RDA_SEEK_UP;
    }
    else
    {
      rx.setFrequencyDown();
      seekDirection = RDA_SEEK_DOWN;
    }
    showStatus();
    encoderCount = 0;
  }
  else if (digitalRead(VOLUME_UP) == LOW)
    rx.setVolumeUp();
  else if (digitalRead(VOLUME_DOWN) == LOW)
    rx.setVolumeDown();
  else if (digitalRead(AUDIO_MUTE) == LOW)
    rx.setMute(bMute = !bMute);  
  else if (digitalRead(SEEK_STATION) == LOW) {
    rx.seek(RDA_SEEK_WRAP, seekDirection, showStatus); // showFrequency will be called by the seek function during the process.
    delay(200);
  }
  else if (digitalRead(AUDIO_BASS) == LOW)
    rx.setBass(bBass = !bBass);
  delay(50);
}
