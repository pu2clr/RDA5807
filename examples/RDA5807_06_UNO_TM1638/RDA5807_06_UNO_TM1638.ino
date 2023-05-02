/*
  This sketch drives the RDA5807 FM receiver and TM1638 (seven-segment display control)
  About TM1638 see: https://github.com/danja/TM1638lite


  Wire up on Arduino UNO, RDA5807 and TM1638

  | Device name               | Device Pin / Description      |  Arduino Pin  |
  | ----------------          | ----------------------------- | ------------  |
  |    TM1638                 |                               |               |
  |                           | STB                           |    4          |
  |                           | CLK                           |    5          |
  |                           | DIO                           |    6          |
  |                           | VCC                           |    3.3V       |
  |                           | GND                           |    GND        |
  |    RDA5807                |                               |               |
  |                           | SDIO (pin 18)                 |     A4        |
  |                           | SCLK (pin 17)                 |     A5        |
  |                           | SEN (pin 16)                  |    GND        |

  The TM1638 painel controls

  | TM1638 button |  Description                             | 
  | ------------- | ---------------------------------------- |
  | S1            | Increments the frequency (100kHz step)   |
  | S2            | Decrements the frequency (100kHz Down)   |
  | S3            | Seeks the next station available (The direction is defined by the last action of frequency changing) |
  | S4            | Increments the audio volume |
  | S5            | Decrements the audio volume |
  | S6            | Turnn the audio On or Off |
  | S7            | Sets the Stereo On or Off |
  | S8            | Sets bass  On or Off |



  Prototype documentation: https://pu2clr.github.io/RDA5807/
  By PU2CLR, Ricardo,  Apr  2023.
*/

#include <TM1638lite.h>
#include <RDA5807.h>
#include <EEPROM.h>


// TM1638 DISPLAY PINS
#define TM1638_STB 4
#define TM1638_CLK 5
#define TM1638_DIO 6


// TM1638 - Buttons controllers
#define S1_FREQ_DOWN 1      // S1 - Increments the frequency (100kHz step)
#define S2_FREQ_UP 2        // S2 - Decrements the frequency (100kHz Down)
#define S3_SEEK_STATION 4   // S3 - Seeks the next station available (The direction is defined by the last action of frequency changing)
#define S4_VOL_DOWN 8       // S4 - Increments the audio volume
#define S5_VOL_UP 16        // S5 - Decrements the audio volume
#define S6_AUDIO_MUTE 32    // S6 - Turnn the audio On or Off
#define S7_AUDIO_STEREO 64  // S7 - Sets the Stereo On or Off
#define S8_BASS 128         // S8 - Toggles Bass on or off
#define DEFAULT_VOLUME 6

#define STORE_TIME 10000    // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
#define PUSH_MIN_DELAY 300  // Minimum waiting time after an action

#define MIN_ELAPSED_RSSI_TIME 1500


const uint8_t app_id = 43;  // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();

long elapsedRSSI = millis();

uint8_t seekDirection = 1;  // 0 = Down; 1 = Up. This value is set by the last encoder direction or S1_FREQ_DOWN and S2_FREQ_UP.

bool bSt = true;
bool bMute = false;
bool bBass = false;

// Encoder control variables

uint16_t currentFrequency;
uint16_t previousFrequency;



uint8_t rssi = 0;
uint8_t volume = DEFAULT_VOLUME;

// Devices class declarations

TM1638lite tm(TM1638_STB, TM1638_CLK, TM1638_DIO);
RDA5807 rx;

void setup() {

  uint8_t tm_button;
  tm.reset();
  delay(100);
  tm_button = tm.readButtons();

  // If you want to reset the eeprom, keep the ENCODER PUSH BUTTON  pressed during statup
  if (tm_button == S3_SEEK_STATION) {
    EEPROM.write(eeprom_address, 0);
    for (int i = 0; i < 8; i++) {
      tm.setLED(i, 1);
      tm.displayHex(i, 0);
      delay(150);
    }
    delay(2000);
  }

  showSplash();
  rx.setup();

  if (EEPROM.read(eeprom_address) == app_id) {
    readAllReceiverInformation();
  } else {
    // Default values
    rx.setVolume(DEFAULT_VOLUME);
    rx.setMono(!bSt);  // Force stereo
    rx.setMute(bMute);
    currentFrequency = previousFrequency = 10390;
  }

  rx.setFrequency(currentFrequency);  // It is the frequency you want to select in MHz multiplied by 100.
  rx.setSeekThreshold(50);            // Sets RSSI Seek Threshold (0 to 127)
  rx.setAFC(true);

  rx.setGpio(3,1);  //  Mono/Stereo indicator. When Stereo, the GPIO03 (pin 15 of the RDA5807FP) becomes high 
  
  showStatus();
}


/**
* Saves the latest receiver information (Only if it was chamged)
*/
void saveAllReceiverInformation() {
  EEPROM.update(eeprom_address, app_id);
  EEPROM.update(eeprom_address + 1, rx.getVolume());           // stores the current Volume
  EEPROM.update(eeprom_address + 2, currentFrequency >> 8);    // stores the current Frequency HIGH byte for the band
  EEPROM.update(eeprom_address + 3, currentFrequency & 0xFF);  // stores the current Frequency LOW byte for the band
  EEPROM.update(eeprom_address + 4, (uint8_t)bMute);
  EEPROM.update(eeprom_address + 5, (uint8_t)bSt);
  EEPROM.update(eeprom_address + 6, bBass);
}

/**
  Rescues the last receiver information saved into eeprom
*/
void readAllReceiverInformation() {
  rx.setVolume(EEPROM.read(eeprom_address + 1));
  currentFrequency = EEPROM.read(eeprom_address + 2) << 8;
  currentFrequency |= EEPROM.read(eeprom_address + 3);
  previousFrequency = currentFrequency;

  bMute = (bool)EEPROM.read(eeprom_address + 4);
  rx.setMute(bMute);

  bSt = (bool)EEPROM.read(eeprom_address + 5);
  rx.setMono(!bSt);

  bBass = EEPROM.read(eeprom_address + 6);
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
  rx.convertToChar(currentFrequency, freq, 5, 3, '.');

  for (int i = 3; i < 8; i++)
    tm.displayASCII(i, freq[i - 3]);
}

/**
 * This function is called by the seek function process.
 */
void showFrequencySeek() {
  currentFrequency = rx.getFrequency();
  showFrequency();
}

/**
 * Show some basic information on display
 */
void showStatus() {
  tm.reset();
  showFrequency();
  tm.setLED(0, rx.isStereo());  // Turn On or Off the first LED of the TM1638 painel
  tm.setLED(1, bMute);  // Turn On or Off the secound LED of the TM1638 painel
  tm.setLED(2, bBass); // 
  showRSSI();
  showVolume();
}


void showVolume() {
  int8_t vol = rx.getVolume();
  tm.setLED(5, 0);
  tm.setLED(6, 0);
  tm.setLED(7, 0);
  if ( vol < 6 ) {
    tm.setLED(5, 1);   
  } else if ( vol < 10 ) {
    tm.setLED(5, 1);
    tm.setLED(6, 1);  
  }
  else {
    tm.setLED(5, 1);
    tm.setLED(6, 1);  
    tm.setLED(7, 1);
  }   

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

  // clearStatusDisplay();

  tm.displayASCII(0, 'S');
  tm.displayHex(1, rssiAux);
}


void loop() {
  // Check if the encoder has moved.
  uint8_t tm_button = tm.readButtons();
  // Checks if a button was pressed
  if (tm_button) {
    switch (tm_button) {
      case S1_FREQ_DOWN:
        rx.setFrequencyDown();
        seekDirection = 0;
        showFrequency();
        break;
      case S2_FREQ_UP:
        rx.setFrequencyUp();
        seekDirection = 1;
        currentFrequency = rx.getFrequency();
        showFrequency();
        break;
      case S3_SEEK_STATION:
        rx.seek(RDA_SEEK_WRAP, seekDirection, showFrequencySeek);  // showFrequencySeek will be called by the seek function during the process.
        break;
      case S4_VOL_DOWN:
        rx.setVolumeDown();
        break;
      case S5_VOL_UP:
        rx.setVolumeUp();
        break;
      case S6_AUDIO_MUTE:
        rx.setMute(bMute = !bMute);
        break;
      case S7_AUDIO_STEREO:
        rx.setMono(bSt = !bSt);
        break;
      case S8_BASS:
        // toggles bass on off
        rx.setBass(bBass = !bBass);
        break;
    }
    // Something was changed
      delay(PUSH_MIN_DELAY);
      storeTime = millis();
      previousFrequency = 0;
      showStatus();
  }

  // Show RSSI status only if this condition has changed
  if ((millis() - elapsedRSSI) > MIN_ELAPSED_RSSI_TIME)
  {
    int aux = rx.getRssi();
    if (rssi != aux)
    {
      rssi = aux;
      showRSSI();
    }
    elapsedRSSI = millis();
  }


  // Check if some information was changed. If so, it will be eligible to be saved into eeprom
  if ((currentFrequency = rx.getFrequency()) != previousFrequency) {
    if ((millis() - storeTime) > STORE_TIME) {
      saveAllReceiverInformation();
      storeTime = millis();
      previousFrequency = currentFrequency;
    }
  }

  delay(10);
}
