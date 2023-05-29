/*
   It is a RDA5807 receiver controlled by an ATtiny84 device  using I2C OLED 96" (128x64).
   It is FM receiver with RDS support. You can monitor Station Name, Station Info, Program info and UTC (time).
   It also uses the internal EEPROM to save the latest frequency (Condition: After 10s of inativity and frequency changed) 
   You can also explore other RDS information by addming more functions.   

   This sketch uses polling method to control the encoder instead interrupt. 
  
   ATtiny84 and RDA5807 wireup  

    | RDA5807 Function | Attiny84 REF pin | Physical pin  | 
    | ---------------- | -----------------| ------------- | 
    | SEEK_UP          |     3            |    10         | 
    | SEEK_DOWN        |     5            |     8         |
    | ENCODER_PIN_A    |     0            |    13         |
    | ENCODER_PIN_B    |     1            |    12         |  
    | SDIO / SDA       |     SDA          |     7         |
    | SCLK / CLK       |     SCL          |     9         |

   By Ricardo Lima Caratti, 2023.
*/

#include <RDA5807.h>
#include <Tiny4kOLED.h>
#include <EEPROM.h>


// Please, check the ATtiny84 physical pins
#define SEEK_UP 3
#define SEEK_DOWN 5
#define ENCODER_PIN_A 0
#define ENCODER_PIN_B 1

#define VALID_DATA 85


unsigned char encoder_pin_a;
unsigned char encoder_prev = 0;
unsigned char encoder_pin_b;
long elapsedTimeEncoder = millis();
long timeRdsShow = millis();

char *stationName;
char *stationInfo;
char *programInfo;
char *utcTime;

uint8_t idxProgInfo = 0;
uint8_t idxStationInfo = 0;

long delayStationName = millis();
long delayStationInfo = millis();
long delayProgramInfo = millis();
long delayUtcTime = millis();
long delayInativity = millis();

uint16_t currentFrequency;
uint16_t oldFrequency;

RDA5807 rx;

void setup() {
  pinMode(SEEK_UP, INPUT_PULLUP);
  pinMode(SEEK_DOWN, INPUT_PULLUP);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  oled.begin();
  oled.on();
  oled.clear();
  oled.setFont(FONT6X8);
  oled.setCursor(0, 0);
  oled.print(F("RDA5807-Attiny84"));
  oled.setCursor(0, 2);
  oled.print(F("   By PU2CLR   "));
  delay(2000);
  oled.clear();

  rx.setup();

  // Restores the latest frequency and audio mute statis saved into the EEPROM
  if (EEPROM.read(0) == VALID_DATA) {
    currentFrequency = EEPROM.read(1) << 8;
    currentFrequency |= EEPROM.read(2);
    rx.setMute(EEPROM.read(3));
  } else {
    currentFrequency = 10390;  // default value
  }

  // Starts RDS setup
  rx.setRDS(true);
  rx.setRdsFifo(true);
  rx.setVolume(7);
  rx.setFrequency(oldFrequency = currentFrequency);  // It is the frequency you want to select in MHz multiplied by 100.
  showStatus();
}


void saveEEPROM() {
  // Saves the current frequency if it has changed.
  // currentFrequency = rx.getFrequency();
  EEPROM.update(0, VALID_DATA);               // Says that a valid frequency will be saved
  EEPROM.update(1, currentFrequency >> 8);    // stores the current Frequency HIGH byte
  EEPROM.update(2, currentFrequency & 0xFF);  // stores the current Frequency LOW byte
  EEPROM.update(3, rx.isMuted());             // Stores the current audio mute status
}

void showStatus() {
  oled.clear();
  oled.setCursor(0, 0);
  oled.print(rx.formatCurrentFrequency());
  rx.clearRdsBuffer();
}

void showRdsText(uint8_t col, uint8_t lin, char *rdsInfo) {
  oled.setCursor(col, lin);
  oled.clearToEOL();
  oled.setCursor(col, lin);
  oled.print(rdsInfo);
}

void processRdsInfo() {

  long currentmillis = millis();
  char aux[22];

  // Shows station name on Display each three seconds.
  if (stationName != NULL && (currentmillis - delayStationName) > 3000) {
    showRdsText(60, 0, stationName);
    delayStationName = currentmillis;
  }

  // Shows, with scrolling, station info on display each five seconds.
  if (stationInfo != NULL && strlen(stationInfo) > 1 && (currentmillis - delayStationInfo) > 1000) {
    strncpy(aux, &stationInfo[idxStationInfo], 20);
    aux[20] = 0;
    showRdsText(0, 1, aux);
    idxStationInfo += 2;  // shift left two character
    if (idxStationInfo > 31) idxStationInfo = 0;
    delayStationInfo = currentmillis;
  }

  // Shows, with scrolling, the  program information each a half seconds.
  if (programInfo != NULL && strlen(programInfo) > 1 && (currentmillis - delayProgramInfo) > 500) {
    // Process scrolling
    strncpy(aux, &programInfo[idxProgInfo], 20);
    aux[20] = 0;
    idxProgInfo += 2;  // shift left two character
    showRdsText(0, 2, aux);
    if (idxProgInfo > 60) idxProgInfo = 0;
    delayProgramInfo = currentmillis;
  }
  // Some stations broadcast spurious information. In this case, the displayed time may not make sense.
  if (utcTime != NULL && strlen(utcTime) > 8 && (currentmillis - delayUtcTime) > 60000) {
    showRdsText(0, 3, utcTime);
    delayUtcTime = currentmillis;
  }
}

void loop() {

  uint8_t bkey;
  bkey = (digitalRead(SEEK_UP) << 1) | digitalRead(SEEK_DOWN);  // If 3 (0b11) means no button is pressed - It may help you to save some bytes of memory
  if (bkey != 0b11) {
    if (bkey == 0b10)  // 2 (0b10) -  Seek up pressed
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_UP, showStatus);
    else  // 1 (0b01) - Seek down pressed
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_DOWN, showStatus);
    showStatus();  // call once again to show the latest status.
    delay(200);    // avoids repeated reading of the button
    currentFrequency = rx.getFrequency();
    delayInativity = millis();
  }

  // Instead of using interrupts to deal with encoder control, this sketch utilizes the polling approach.
  // While it may not be the optimal method, it proved effective memory optimization in this particular case.
  if ((millis() - elapsedTimeEncoder) > 5) {
    encoder_pin_a = digitalRead(ENCODER_PIN_A);
    encoder_pin_b = digitalRead(ENCODER_PIN_B);
    if ((!encoder_pin_a) && (encoder_prev))  // has ENCODER_PIN_A gone from high to low?
    {                                        // if so,  check ENCODER_PIN_B. It is high then clockwise (1) else counter-clockwise (-1)
      if (encoder_pin_b)
        rx.setFrequencyUp();
      else
        rx.setFrequencyDown();
      showStatus();
      currentFrequency = rx.getFrequency();
      delayInativity = millis();
    }
    encoder_prev = encoder_pin_a;
    elapsedTimeEncoder = millis();  // keep elapsedTimeEncoder updated
  }

  // This RDA5807 Arduino Library implements almost all needed function to get all RDS information. 
  // Please, check the API documentation or the source-code and check it out.
  if (rx.getRdsAllData(&stationName, &stationInfo, &programInfo, &utcTime))
    processRdsInfo();

  // Save only if the frequency changes and the time of inativity is grater than 10s.
  if (oldFrequency != currentFrequency && (millis() - delayInativity) > 10000L) {
    oldFrequency = currentFrequency;
    delayInativity = millis();
    saveEEPROM();
  }

  delay(1);
}
