/*
  This sketch uses an Arduino Nano with LCD16X02 DISPLAY
  It is also a FM receiver capable to tune your local FM stations.
  This sketch saves the latest status of the receiver into the Atmega328 eeprom (frequency, RDS and Stereo setup).


  ABOUT THE ATMEGA328 EEPROM and saving the receiver current information
  ATMEL says the lifetime of an EEPROM memory position is about 100,000 writes.
  For this reason, this sketch tries to avoid unnecessary writes into the eeprom.
  This firmware saves the latest frequency or volume data 10 secounds after one of these information is changed.
  For example, if you switch the band and turn the receiver off immediately, no new information will be written into the eeprom.
  But you wait 10 seconds after changing anything, all new information will be written.

  TO RESET the EEPROM: Turn your receiver on with the encoder push button pressed.


  Read more on https://pu2clr.github.io/RDA5807/

  Wire up on Arduino UNO, Nano or Pro mini

  | Device name               | Device Pin / Description  |  Arduino Pin  |
  | --------------------------| --------------------      | ------------  |
  |    LCD 16x2 or 20x4       |                           |               |
  |                           | D4                        |     D7        |
  |                           | D5                        |     D6        |
  |                           | D6                        |     D5        |
  |                           | D7                        |     D4        |
  |                           | RS                        |     D12       |
  |                           | E/ENA                     |     D13       |
  |                           | RW & VSS & K (16)         |    GND        |
  |                           | A (15) & VDD              |    +Vcc       |
  | --------------------------| ------------------------- | --------------|
  | RDA5807                   |                           |               |
  |                           | ------------------------- | --------------|
  |                           | SDIO (pin 8)              |     A4        |
  |                           | SCLK (pin 7)              |     A5        |
  | --------------------------| --------------------------| --------------|
  | Buttons                   |                           |               |
  |                           | Volume Up                 |      8        |
  |                           | Volume Down               |      9        |
  |                           | Stereo/Mono               |     10        |
  |                           | RDS ON/off                |     11        |
  |                           | SEEK (encoder button)     |     D14/A0    |
  | --------------------------| --------------------------|---------------|
  | Encoder                   |                           |               |
  |                           | --------------------------|---------------|
  |                           | A                         |       2       |
  |                           | B                         |       3       |

  See user_manual.txt

  Prototype documentation: https://pu2clr.github.io/RDA5807/
  PU2CLR RDA5807 API documentation: https://pu2clr.github.io/RDA5807/extras/apidoc/html/

  By PU2CLR, Ricardo,  Feb  2023.
*/

#include <RDA5807.h>
#include <EEPROM.h>
#include <LiquidCrystal.h>

#include "Rotary.h"

// LCD 16x02 or LCD20x4 PINs
#define LCD_D7 4
#define LCD_D6 5
#define LCD_D5 6
#define LCD_D4 7
#define LCD_RS 12
#define LCD_E 13

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF

// Enconder PINs
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

// Buttons controllers
#define VOLUME_UP 8       // Volume Up
#define VOLUME_DOWN 9     // Volume Down
#define SWITCH_STEREO 10  // Select Mono or Stereo
#define SWITCH_RDS 11     // SDR ON or OFF
#define SEEK_FUNCTION 14

#define POLLING_TIME 2000
#define RDS_MSG_TYPE_TIME 25000
#define POLLING_RDS 80

#define STORE_TIME 10000  // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
#define PUSH_MIN_DELAY 300

const uint8_t app_id = 43;  // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();

bool bSt = true;
bool bRds = true;
bool bShow = false;
uint8_t seekDirection = 1;  // 0 = Down; 1 = Up. This value is set by the last encoder direction.

long pollin_elapsed = millis();

int maxX1;
int maxY1;

// Encoder control variables
volatile int encoderCount = 0;
uint16_t currentFrequency;
uint16_t previousFrequency;

// Encoder control
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

// LCD display
LiquidCrystal lcd(LCD_RS, LCD_E, LCD_D4, LCD_D5, LCD_D6, LCD_D7);

RDA5807 rx;

void setup() {

  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  // Push button pin
  pinMode(VOLUME_UP, INPUT_PULLUP);
  pinMode(VOLUME_DOWN, INPUT_PULLUP);
  pinMode(SWITCH_STEREO, INPUT_PULLUP);
  pinMode(SWITCH_RDS, INPUT_PULLUP);
  pinMode(SEEK_FUNCTION, INPUT_PULLUP);

  // Start LCD display device
  lcd.begin(16, 2);
  showSplash();

  // If you want to reset the eeprom, keep the ENCODER PUSH BUTTON  pressed during statup
  if (digitalRead(SEEK_FUNCTION) == LOW) {
    EEPROM.write(eeprom_address, 0);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("RESET");
    delay(1500);
    showSplash();
  }

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup();

  // Checking the EEPROM content
  if (EEPROM.read(eeprom_address) == app_id) {
    readAllReceiverInformation();
  } else {
    // Default values
    rx.setVolume(6);
    rx.setMono(false);  // Force stereo
    // rx.setRBDS(true);  //  set RDS and RBDS. See setRDS.
    rx.setRDS(true);
    rx.setRdsFifo(true);
    currentFrequency = previousFrequency = 10390;
  }

  rx.setFrequency(currentFrequency);  // It is the frequency you want to select in MHz multiplied by 100.
  rx.setSeekThreshold(50);            // Sets RSSI Seek Threshold (0 to 127)
  lcd.clear();
  showStatus();
}

void saveAllReceiverInformation() {
  // The update function/method writes data only if the current data is not equal to the stored data.
  EEPROM.update(eeprom_address, app_id);
  EEPROM.update(eeprom_address + 1, rx.getVolume());           // stores the current Volume
  EEPROM.update(eeprom_address + 2, currentFrequency >> 8);    // stores the current Frequency HIGH byte for the band
  EEPROM.update(eeprom_address + 3, currentFrequency & 0xFF);  // stores the current Frequency LOW byte for the band
  EEPROM.update(eeprom_address + 4, (uint8_t)bRds);
  EEPROM.update(eeprom_address + 5, (uint8_t)bSt);
}

void readAllReceiverInformation() {
  rx.setVolume(EEPROM.read(eeprom_address + 1));
  currentFrequency = EEPROM.read(eeprom_address + 2) << 8;
  currentFrequency |= EEPROM.read(eeprom_address + 3);
  previousFrequency = currentFrequency;

  bRds = (bool)EEPROM.read(eeprom_address + 4);
  rx.setRDS(bRds);
  rx.setRdsFifo(bRds);

  bSt = (bool)EEPROM.read(eeprom_address + 5);
  rx.setMono(bSt);
}

/*
   To store any change into the EEPROM, it is needed at least STORE_TIME  milliseconds of inactivity.
*/
void resetEepromDelay() {
  delay(PUSH_MIN_DELAY);
  storeTime = millis();
  previousFrequency = 0;
}

/*
    Reads encoder via interrupt
    Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
*/
void rotaryEncoder() {  // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus)
    encoderCount = (encoderStatus == DIR_CW) ? 1 : -1;
}

void showSplash() {
  lcd.setCursor(0, 0);
  lcd.print("PU2CLR-RDA5807");
  lcd.setCursor(0, 1);
  lcd.print("Arduino Library");
  lcd.display();
  delay(1000);
}

/*
   Shows the static content on  display
*/
void showTemplate() {
}

/*
   Shows frequency information on Display
*/
void showFrequency() {
  currentFrequency = rx.getFrequency();
  lcd.setCursor(4, 1);
  lcd.print(rx.formatCurrentFrequency());
  lcd.display();
}

void showFrequencySeek() {
  clearLcdLine(1);
  showFrequency();
}

/*
    Show some basic information on display
*/
void showStatus() {
  clearLcdLine(1);
  showFrequency();
  showStereoMono();
  showRSSI();

  if (bRds) {
    showRds();
  }

  lcd.display();
}

/* *******************************
   Shows RSSI status
*/
void showRSSI() {
  char rssi[12];
  rx.convertToChar(rx.getRssi(), rssi, 3, 0, '.');
  strcat(rssi, "dB");
  lcd.setCursor(13, 1);
  lcd.print(rssi);
}

void showStereoMono() {
  lcd.setCursor(0, 1);
  if (bSt) {
    lcd.print("ST");
  } else {
    lcd.print("MO");
  }
}

/*********************************************************
   RDS
 *********************************************************/
char *programInfo;
char *stationName;
char *rdsTime;
char *stationInfo;
int currentMsgType = 0;
long polling_rds = millis();
long timeTextType = millis();  // controls the type of each text will be shown (Message, Station Name or time)

int programInfoIndex = 0;  // controls the part of the rdsMsg text will be shown on LCD 16x2 Display

long delayProgramInfo = millis();
long delayStationName = millis();
long delayRdsTime = millis();

/**
  showRDSMsg - Shows the Program Information
*/
void showRDSMsg() {
  char txtAux[17];

  if (programInfo == NULL || strlen(programInfo) < 2 || (millis() - delayProgramInfo) < 1000)
    return;
  delayProgramInfo = millis();
  clearLcdLine(0);
  programInfo[61] = '\0';  // Truncate the message to fit on display line
  strncpy(txtAux, &programInfo[programInfoIndex], 16);
  txtAux[16] = '\0';
  programInfoIndex += 3;
  if (programInfoIndex > 60)
    programInfoIndex = 0;
  lcd.setCursor(0, 0);
  lcd.print(txtAux);
}

/**
   showRDSStation - Shows the
*/
void showRDSStation() {
  if (stationName == NULL || strlen(stationName) < 2 || (millis() - delayStationName) < 3000)
    return;
  delayStationName = millis();
  clearLcdLine(0);
  lcd.setCursor(0, 0);
  lcd.print(stationName);
}

void showRDSTime() {
  char txtAux[17];

  if (rdsTime == NULL || strlen(rdsTime) < 2 || (millis() - delayRdsTime) < 60000)
    return;
  delayRdsTime = millis();
  clearLcdLine(0);
  rdsTime[16] = '\0';
  strncpy(txtAux, rdsTime, 16);
  txtAux[16] = '\0';
  lcd.setCursor(0, 0);
  lcd.print(txtAux);
}

void clearLcdLine(uint8_t line) {
  for (int i = 0; i < 16; i++) {
    lcd.setCursor(i, line);
    lcd.print(' ');
  }
}

void clearRds() {
  bShow = false;
  programInfo = NULL;
  stationName = NULL;
  rdsTime = NULL;
  programInfoIndex = currentMsgType = 0;
  rx.clearRdsBuffer();
  clearLcdLine(0);
}

void checkRDS() {
  // Gets the points of array char containing RDS information
  if (rx.getRdsAllData(&stationName, &stationInfo , &programInfo, &rdsTime) ) {
    if (currentMsgType == 0)
      showRDSMsg();  // Time to show program information
    else if (currentMsgType == 1)
      showRDSStation(); // Time to show station name
    else if (currentMsgType == 2)
      showRDSTime(); // Time to show UTC time
  }
}

void showRds() {

  lcd.setCursor(2, 1);
  if (bRds)
    lcd.print(".");
  else
    lcd.print(" ");
}

/*********************************************************

 *********************************************************/

void doStereo() {
  rx.setMono((bSt = !bSt));
  bShow = true;
  showStereoMono();
  resetEepromDelay();
}

void doRds() {
  rx.setRDS((bRds = !bRds));
  programInfoIndex = currentMsgType = 0;
  showRds();
  resetEepromDelay();
}

/**
   Process seek command.
   The seek direction is based on the last encoder direction rotation.
*/
void doSeek() {
  rx.seek(RDA_SEEK_WRAP, seekDirection, showFrequencySeek);  // showFrequency will be called by the seek function during the process.
  delay(200);
  bShow = true;
  showStatus();
}

void loop() {

  // Check if the encoder has moved.
  if (encoderCount != 0) {
    if (encoderCount == 1) {
      rx.setFrequencyUp();
      seekDirection = RDA_SEEK_UP;
    } else {
      rx.setFrequencyDown();
      seekDirection = RDA_SEEK_DOWN;
    }
    showStatus();
    bShow = true;
    encoderCount = 0;
    storeTime = millis();
  }

  if (digitalRead(VOLUME_UP) == LOW) {
    rx.setVolumeUp();
    resetEepromDelay();
  } else if (digitalRead(VOLUME_DOWN) == LOW) {
    rx.setVolumeDown();
    resetEepromDelay();
  } else if (digitalRead(SWITCH_STEREO) == LOW)
    doStereo();
  else if (digitalRead(SWITCH_RDS) == LOW)
    doRds();
  else if (digitalRead(SEEK_FUNCTION) == LOW)
    doSeek();

  if ((millis() - pollin_elapsed) > POLLING_TIME) {
    showStatus();
    if (bShow)
      clearRds();
    pollin_elapsed = millis();
  }

  if ((millis() - polling_rds) > POLLING_RDS) {
    if (bRds) {
      checkRDS();
    }
    polling_rds = millis();
  }

  if ((millis() - timeTextType) > RDS_MSG_TYPE_TIME) {
    // Toggles the type of message to be shown - See showRds function
    currentMsgType++;
    programInfoIndex = 0;
    if (currentMsgType > 2)
      currentMsgType = 0;
    timeTextType = millis();
  }

  // Show the current frequency only if it has changed
  if ((currentFrequency = rx.getFrequency()) != previousFrequency) {
    clearRds();
    if ((millis() - storeTime) > STORE_TIME) {
      saveAllReceiverInformation();
      storeTime = millis();
      previousFrequency = currentFrequency;
    }
  }

  delay(5);
}
