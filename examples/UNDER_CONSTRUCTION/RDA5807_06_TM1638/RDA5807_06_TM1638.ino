/*

  UNDER CONSTRUCTION...

  Wire up on Arduino UNO, Pro mini and RDA5807
  | Device name               | Device Pin / Description      |  Arduino Pin  |
  | ----------------          | ----------------------------- | ------------  |
  |    TM1638                 |                               |               |
  |                           | STB                           |    4          |
  |                           | CLK                           |    7          |
  |                           | DIO                           |    8          |
  |                           | VCC                           |    3.3V       |
  |                           | GND                           |    GND        |
  |    RDA5807                |                               |               |
  |                           | SDIO (pin 18)                 |     A4        |
  |                           | SCLK (pin 17)                 |     A5        |
  |                           | SEN (pin 16)                  |    GND        |
  |    Encoder                |                               |               |
  |                           | A                             |       2       |
  |                           | B                             |       3       |
  |                           | Encoder button                |      A0       |

  Prototype documentation: https://pu2clr.github.io/RDA5807/
  By PU2CLR, Ricardo,  Apr  2023.
*/

#include <TM1638lite.h>
#include <RDA5807.h>

#include "Rotary.h"

// Enconder PINs
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3
#define ENCODER_PUSH_BUTTON 14 // Used to select the enconder control (BFO or VFO)

// TM1638 - Buttons controllers
#define S1_FREQ_UP 1       // S1 Band switch button
#define S2_FREQ_DOWN 2     // S2 FM/AM/SSB
#define S3_SEEK_STATION 4  // S3 Used to select the banddwith. Values: 1.2, 2.2, 3.0, 4.0, 0.5, 1.0 kHz
#define S4_VOL_UP 8        // S4 Seek
#define S5_VOL_DOWN 16     // S5 Switch AGC ON/OF
#define S6_AUDIO_MUTE 32   // S6 Increment or decrement frequency step (1, 5 or 10 kHz)
#define S7_AUDIO_STEREO 64 // S7 Volume Control
#define S8_FM_RDS 128      // S8 External AUDIO MUTE circuit control

#define DEFAULT_VOLUME 6

#define STORE_TIME 10000  // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).

const uint8_t app_id = 43;  // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();

uint8_t seekDirection = 1;  // 0 = Down; 1 = Up. This value is set by the last encoder direction or S1_FREQ_UP and S2_FREQ_DOWN.

bool bSt = true;
bool bRds = false;

// Encoder control variables
volatile int encoderCount = 0;
uint16_t currentFrequency;
uint16_t previousFrequency;


uint8_t rssi = 0;
uint8_t volume = DEFAULT_VOLUME;

// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
TM1638lite tm(TM1638_STB, TM1638_CLK, TM1638_DIO);
RDA5807 rx;

void setup()
{
  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP);
  tm.reset();

  showSplash();

  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup();
}

/**
 * Shows the static content on  display
 */
void showSplash()
{
  const char *s7 = " RDA5807";
  for (int i = 0; i < 8; i++)
  {
    tm.setLED(i, 1);
    delay(200);
    tm.displayASCII(i, s7[i]);
    tm.setLED(i, 0);
    delay(200);
  }
  delay(1000);
  tm.reset();
}

/**
 * Clear the first three 7 seg. display
 */
void clearStatusDisplay()
{
  tm.displayASCII(0, ' ');
  tm.displayASCII(1, ' ');
  tm.displayASCII(2, ' ');
}

/**
 * Reads encoder via interrupt. It uses Rotary.h and Rotary.cpp
 */
void rotaryEncoder()
{ // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
    encoderCount = (encoderStatus == DIR_CW) ? 1 : -1;
}

/**
 * Shows frequency information on Display
 */
void showFrequency()
{
  char bufferDisplay[15];
  char tmp[15];
  // It is better than use dtostrf or String to save space.
  sprintf(tmp, "%5.5u", currentFrequency);
  bufferDisplay[0] = (tmp[0] == '0') ? ' ' : tmp[0];
  bufferDisplay[1] = tmp[1];
  if (rx.isCurrentTuneFM())
  {
    bufferDisplay[2] = tmp[2];
    bufferDisplay[3] = '.';
    bufferDisplay[4] = tmp[3];
  }
  else
  {
    if (currentFrequency < 1000)
    {
      bufferDisplay[1] = ' ';
      bufferDisplay[2] = tmp[2];
      bufferDisplay[3] = tmp[3];
      bufferDisplay[4] = tmp[4];
    }
    else
    {
      bufferDisplay[2] = tmp[2];
      bufferDisplay[3] = tmp[3];
      bufferDisplay[4] = tmp[4];
    }
  }
  bufferDisplay[5] = '\0';
  for (int i = 3; i < 8; i++)
    tm.displayASCII(i, bufferDisplay[i - 3]);
}

/**
 * This function is called by the seek function process.
 */
void showFrequencySeek(uint16_t freq)
{
  currentFrequency = freq;
  showFrequency();
}

/**
 * Show some basic information on display
 */
void showStatus()
{
  tm.reset();
  showFrequency();
  showMode();
}

/**
 *   Shows the current RSSI and SNR status
 */
void showRSSI()
{
  uint8_t rssiAux;
  if (isCommand())
    return; // do not show the RSSI during command status
  if (currentMode == FM)
  {
    tm.setLED(0, rx.getCurrentPilot()); // Indicates Stereo or Mono
  }
  // It needs to be calibrated. You can do it better.
  // RSSI: 0 to 127 dBuV
  if (rssi < 2)
    rssiAux = 4;
  else if (rssi < 4)
    rssiAux = 5;
  else if (rssi < 12)
    rssiAux = 6;
  else if (rssi < 25)
    rssiAux = 7;
  else if (rssi < 50)
    rssiAux = 8;
  else if (rssi >= 50)
    rssiAux = 9;

  clearStatusDisplay();

  tm.displayASCII(0, 'S');
  tm.displayHex(1, rssiAux);
}

/**
 * Show cmd on display. It means you are setting up something.
 */
inline void showCommandStatus(uint8_t v)
{
  tm.setLED(1, v);
}

/**
 * Find a station. The direction is based on the last encoder move clockwise or counterclockwise
 */
void doSeek()
{
  rx.seekStationProgress(showFrequencySeek, seekDirection);
  currentFrequency = rx.getFrequency();
}

void loop()
{
  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if ((seekDirection = (encoderCount == 1)))
      rx.frequencyUp();
    else
      rx.frequencyDown();
    currentFrequency = rx.getFrequency();
    showFrequency();
    encoderCount = 0;
  }
  else // checks actions from buttons
  {
    uint8_t tm_button = tm.readButtons();
    delay(50);
    switch (tm_button)
    {
    case S1_FREQ_UP:
      break;
    case S2_FREQ_DOWN:
      break;
    case S3_SEEK_STATION:
      break;
    case S4_VOL_UP:
      break;
    case S5_VOL_DOWN:
      break;
    case S6_AUDIO_MUTE:
      break;
    case S7_AUDIO_STEREO:
      break;
    case S8_FM_RDS:
      break;
    default:
    }

  }
  delay(5);
}
