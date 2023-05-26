/*
   Test and validation of RDA5807 on ATtiny85 device  using I2C OLED 91" (128x32).
   It is a FM receiver with mini OLED and and three push buttons (Seek Up, Seek Down and Audio Mute).
   This sketch implements FM RDS and eeprom function to store the latest frequency and audio status .  
   
   ATtiny85 and RDA5807 wireup  

    | RDA5807 pin     | Attiny85 REF pin | Physical pin  | 
    | ----------------| -----------------| ------------- | 
    | SEEK_UP         |     PB1          |     6         | 
    | SEEK_DOWN       |     PB4          |     3         |
    | AUDIO_MUTE      |     PB3          |     2         | 
    | SDIO / SDA      |     SDA          |     5         |
    | SCLK / CLK      |     SCL          |     7         |
   
    Compiling and uploading: 
    1) Install the ATtiny Core in Arduino IDE - Insert the URL http://drazzy.com/package_drazzy.com_index.json on board manager. 
                                                To do that, go to Preferences, enter the above URL in "Additional Boards Manager URLs. 
                                                To setup ATtiny85 on Arduino IDE, go to Tools Menu, Board, Board Manager and install 
                                                "ATTinyCore by Spence Konde". 
    2) Setup: Chip: ATtiny85;  Clock Source: 4MHz (Internal); LTO Enabled; millis() / macros() Enabled; 

    ATTENTION: if you select Clock source 8 MHz, for some reason, the system will work very slow. Maybe a bug. 

   By Ricardo Lima Caratti, 2023.
*/

#include <RDA5807.h>
#include <EEPROM.h>  // The ATtiny85 contains 512 bytes of data EEPROM memory. The EEPROM has an endurance of at least 100,000 write/erase cycles.
#include <Tiny4kOLED.h>
#include <5x5_font.h>

#define SEEK_UP PB1
#define SEEK_DOWN PB4
#define AUDIO_MUTE PB3
#define TIME_RDS_ON_DISPLAY 2300

#define VALID_DATA 85

char *stationName;
char *programInfo;
uint16_t currentFrequency;

long timeRdsShow = millis();
bool hasRds = false;

RDA5807 rx;

void setup() {
  pinMode(SEEK_UP, INPUT_PULLUP);
  pinMode(SEEK_DOWN, INPUT_PULLUP);
  pinMode(AUDIO_MUTE, INPUT_PULLUP);

  oled.begin();
  oled.clear();
  oled.on();
  oled.setFont(FONT8X16);
  // Remove the Splash if you want.
  // Begin Splash
  // oled.setCursor(0, 0);
  // oled.print(F("RDA5807-ATtiny85"));
  // oled.setCursor(0, 2);
  // oled.print(F("   By PU2CLR   "));
  // delay(2000);
  oled.clear();
  // End Splash
  rx.setup();
  rx.setVolume(8);

  // Restores the latest frequency and audio mute statis saved into the EEPROM
  if (EEPROM.read(0) == VALID_DATA) {
    currentFrequency = EEPROM.read(1) << 8;
    currentFrequency |= EEPROM.read(2);
    rx.setMute(EEPROM.read(3));
  } else {
    currentFrequency = 10390;  // default value
  }
  rx.setFrequency(currentFrequency);
  rx.setRDS(true);
  rx.setRdsFifo(true);
  showStatus();
}

void showStatus() {
  oled.setFont(FONT8X16);
  oled.setCursor(0, 0);
  oled.print(F("FM"));
  oled.setCursor(38, 0);
  oled.clearToEOL();
  oled.setCursor(38, 0);
  oled.print(rx.formatCurrentFrequency());
  oled.setCursor(95, 0);
  oled.print(F("MHz"));
  oled.setCursor(0, 2);
  oled.clearToEOL();
  rx.clearRdsBuffer();
  hasRds = false;
}

void showRdsMsg(uint8_t lin, char *msg) {
  oled.setFont(FONT5X5);
  oled.setCursor(0, lin);
  oled.clearToEOL();
  oled.setCursor(0, lin);
  oled.print(msg);
  hasRds = true;
}

void loop() {
  uint8_t bkey;
  bkey = ((digitalRead(SEEK_UP) << 2) | (digitalRead(SEEK_DOWN) << 1)) | digitalRead(AUDIO_MUTE);  // 3, 5 or 6 (Pressed = 0 - considering just one button pressed)
  if (bkey != 0b111) {                                                                             // if none of them is pressed (not igual to 0b011, 0b101 or 0b110) then do nothing.
    if (bkey == 0b011)                                                                             // 3
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_UP, showStatus);
    else if (bkey == 0b101)  // 5
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_DOWN, showStatus);
    else                          // 6
      rx.setMute(!rx.isMuted());  // inverts the audio mute status
    showStatus();
    delay(200);
    // Saves the current frequency if it has changed.
    currentFrequency = rx.getFrequency();
    EEPROM.update(0, VALID_DATA);               // Says that a valid frequency will be saved
    EEPROM.update(1, currentFrequency >> 8);    // stores the current Frequency HIGH byte
    EEPROM.update(2, currentFrequency & 0xFF);  // stores the current Frequency LOW byte
    EEPROM.update(3, rx.isMuted());             // Stores the current audio mute status
  }

  // You must call getRdsReady before calling any RDS query function/method
  if (!hasRds) {
    if (rx.getRdsReady()) {
      if (rx.hasRdsInfoAB()  && !rx.isNewRdsFlagAB() ) {
        stationName = rx.getRdsStationName();
        programInfo = rx.getRdsProgramInformation();
        if (stationName != NULL)
          showRdsMsg(2, stationName);
        if (programInfo != NULL)
          showRdsMsg(3, programInfo);

        delay(70);
      }
    }
  }

  if ( ( millis() - timeRdsShow ) >  (long) TIME_RDS_ON_DISPLAY ) { 
    hasRds = false; 
    timeRdsShow = millis();
  }  

  delay(5);
}
