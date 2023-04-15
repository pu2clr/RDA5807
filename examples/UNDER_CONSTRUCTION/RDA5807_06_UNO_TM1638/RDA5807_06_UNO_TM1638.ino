/*

  UNDER CONSTRUCTION...

  This sketch drive the RDA5807 FM receiver and TM1638 (seven-segment display control)
  About TM1638 see: https://github.com/danja/TM1638lite


  Wire up on Arduino UNO, RDA5807 and TM1638

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


  Prototype documentation: https://pu2clr.github.io/RDA5807/
  By PU2CLR, Ricardo,  Apr  2023.
*/

#include <TM1638lite.h>
#include <RDA5807.h>

#include "Rotary.h"


// TM1638 DISPLAY PINS
#define TM1638_STB 4
#define TM1638_CLK 7
#define TM1638_DIO 8


// TM1638 - Buttons controllers
#define S1_FREQ_UP 1        // S1 - Increments the frequency (100kHz step) 
#define S2_FREQ_DOWN 2      // S2 - Decrements the frequency (100kHz Down) 
#define S3_SEEK_STATION 4   // S3 - Seeks the next station available (The direction is defined by the last action of frequency changing)
#define S4_VOL_UP 8         // S4 - Increments the audio volume
#define S5_VOL_DOWN 16      // S5 - Decrements the audio volume
#define S6_AUDIO_MUTE 32    // S6 - Turnn the audio On or Off
#define S7_AUDIO_STEREO 64  // S7 - Sets the Stereo On or Off
#define S8_STEP 128       // S8 - Sets the RDS on
#define DEFAULT_VOLUME 6

#define STORE_TIME 10000  // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).

const uint8_t app_id = 43;  // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();

uint8_t seekDirection = 1;  // 0 = Down; 1 = Up. This value is set by the last encoder direction or S1_FREQ_UP and S2_FREQ_DOWN.

bool bSt = true;
bool bMute = false;

// Encoder control variables

uint16_t currentFrequency;
uint16_t previousFrequency;


uint8_t rssi = 0;
uint8_t volume = DEFAULT_VOLUME;

// Devices class declarations

TM1638lite tm(TM1638_STB, TM1638_CLK, TM1638_DIO);
RDA5807 rx;

void setup() {

  tm.reset();
  showSplash();
  rx.setup();
}

/**
 * Shows the static content on  display
 */
void showSplash() {
  const char *s7 = " RDA5807";
  for (int i = 0; i < 8; i++) {
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
void clearStatusDisplay() {
  tm.displayASCII(0, ' ');
  tm.displayASCII(1, ' ');
  tm.displayASCII(2, ' ');
}

/**
 * Shows frequency information on Display
 */
void showFrequency() {
  char freq[10];
  rx.convertToChar(currentFrequency, freq, 5, 1, '.');
  sprintf(freq, "%5.5u", currentFrequency);
  for (int i = 3; i < 8; i++)
    tm.displayASCII(i, freq[i - 3]);
}

/**
 * This function is called by the seek function process.
 */
void showFrequencySeek(uint16_t freq) {
  currentFrequency = freq;
  showFrequency();
}

/**
 * Show some basic information on display
 */
void showStatus() {
  tm.reset();
  showFrequency();
}

/**
 *   Shows the current RSSI and SNR status
 */
void showRSSI() {
  uint8_t rssiAux = rx.getRssi();

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

  tm.displayASCII(0, 'S');
  tm.displayHex(1, rssiAux);
}


void loop() {
  // Check if the encoder has moved.
  uint8_t tm_button = tm.readButtons();
  delay(50);
  switch (tm_button) {
    case S1_FREQ_UP:
      rx.setFrequencyUp();
      seekDirection = 1;
      currentFrequency = rx.getFrequency();
      showFrequency();
      break;
    case S2_FREQ_DOWN:
      rx.setFrequencyDown();
      seekDirection = 0;
      currentFrequency = rx.getFrequency();
      showFrequency();
      break;
    case S3_SEEK_STATION:
      rx.seek(RDA_SEEK_WRAP, seekDirection, showFrequency);  // showFrequency will be called by the seek function during the process.
      break;
    case S4_VOL_UP:
      rx.setVolumeUp();
      break;
    case S5_VOL_DOWN:
      rx.setVolumeDown();
      break;
    case S6_AUDIO_MUTE:
      rx.setMute(bMute = !bMute);
      break;
    case S7_AUDIO_STEREO:
      rx.setMono( bSt = !bSt);
      break;
    case S8_STEP:
      // TO DO
      rx.setSpace();
      break;
  }
  delay(5);
}
