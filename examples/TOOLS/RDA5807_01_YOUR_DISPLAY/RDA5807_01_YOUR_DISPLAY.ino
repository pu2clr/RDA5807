/*

  This project is about a library to control the RDA5807 device and the focus of this project is the library and its functionalities. 
  It is not part of this project to assist you in your: displays, encoders, LED, buttons or something else out the library scope. 

  This sketch can be useful to mitigate your effort to develop a RDA5807 application with your favourit display.
  
  ABOUT THE EEPROM:
  ATMEL says the lifetime of an EEPROM memory position is about 100,000 writes.
  For this reason, this sketch tries to avoid unnecessary writes into the eeprom.
  So, the condition to store any status of the receiver is changing the frequency or volume and 10 seconds of inactivity.
  For example, if you switch the band and turn the receiver off immediately, no new information will be written into the eeprom.
  But you wait 10 seconds after changing anything, all new information will be written.

  TO RESET the EEPROM: Turn your receiver on with the encoder push button pressed.


  Read more on https://pu2clr.github.io/RDA5807/

  Wire up on Arduino UNO, Nano or Pro mini

  | Device name               | Device Pin / Description  |  Free Arduino Pins |
  | --------------------------| ------------------------- | -----------------  |
  | YOur Display Setup        |                           |  8, 9, 10, 11,     |
  |                           |                           | 13, A1 (15),       |
  |                           |                           | A2 (16).           |
  | --------------------------| ------------------------- | -------------------|
  | RDA5807                   |                           |                    | 
  |                           | +Vcc 3.3V [1]             |   3.3V             |    
  |                           | SDIO (pin 8)              |     A4             |
  |                           | SCLK (pin 7)              |     A5             |
  | --------------------------| --------------------------| -------------------|
  | Buttons                   |                           |                    |
  |                           | Volume Up                 |      4             |
  |                           | Volume Down               |      5             |
  |                           | Stereo/Mono               |      6             |
  |                           | RDS ON/off                |      7             |
  |                           | SEEK (encoder button)     |     A0/14          |
  | --------------------------| --------------------------|--------------------| 
  | Encoder                   |                           |                    |
  |                           | A                         |       2            |
  |                           | B                         |       3            |

  1. Do not use more than 3.3V to power the RDA5807.

  Please, see user_manual.txt

  Prototype documentation: https://pu2clr.github.io/RDA5807/
  PU2CLR RDA5807 API documentation: https://pu2clr.github.io/RDA5807/extras/apidoc/html/

  By PU2CLR, Ricardo,  Feb  2023.
*/

#include <RDA5807.h>
#include <EEPROM.h>

#include "Rotary.h"

// Your display and Arduino pins setup
// #define ... 
// #define ...

#define COLOR_BLACK 0x0000
#define COLOR_WHITE 0xFFFF

// Enconder PINs
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

// Buttons controllers
#define VOLUME_UP 4       // Volume Up
#define VOLUME_DOWN 5     // Volume Down
#define SWITCH_STEREO 6   // Select Mono or Stereo
#define SWITCH_RDS 7      // SDR ON or OFF
#define SEEK_FUNCTION 14  // Pin A0 / Digital 14

#define POLLING_TIME 3000
#define POLLING_RDS 20

#define STORE_TIME 10000    // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
#define PUSH_MIN_DELAY 300  // Minimum waiting time after an action

const uint8_t app_id = 43;  // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();


bool bSt = true;
bool bRds = false;
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

// Declare  your display driver here
// YOUR_DISPLAY display;


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

  // Start your display here 
  // display.begin(); or display.setup() etc
  showSplash(); // You can remove it
  showTemplate();


  // If you want to reset the eeprom, keep the ENCODER PUSH BUTTON  pressed during statup
  if (digitalRead(SEEK_FUNCTION) == LOW) {
    EEPROM.write(eeprom_address, 0);
    // display.print("RESET");
    delay(1500);
    showSplash();
  }

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup();  // 32.768 kHz passive crystal
  // rx.setup(CLOCK_32K,OSCILLATOR_TYPE_ACTIVE, RLCK_NO_CALIBRATE_MODE_OFF); // 32.768 kHz active crystal / signal generator
  // rx.setLnaPortSel(3);

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
  rx.setLnaPortSel(3);                // LNA setup

  showStatus();
}


void saveAllReceiverInformation() {
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
  // Your Splash here
}

/*
   Shows the static content on  display
*/
void showTemplate() {

  // Your template here

}


/*
   Shows frequency information on Display
*/
void showFrequency() {
  currentFrequency = rx.getFrequency();
  // Show frequency in your display here

}

void showFrequencySeek() {
  // Show frequency during seek in your display here
}

/*
    Show some basic information on display
*/
void showStatus() {
  showFrequency();
  showStereoMono();
  showRSSI();
  showRds();
  if (bRds) {
    showRDSMsg();
    showRDSStation();
    showRDSTime();
  }
}

/* *******************************
   Shows RSSI status
*/
void showRSSI() {
  char rssi[12];

  rx.convertToChar(rx.getRssi(), rssi, 3, 0, '.');
  strcat(rssi, "dB");
  // show the rssi here
}

void showStereoMono() {
  // Show Mono/Stereo status 
}

/*********************************************************
   RDS
 *********************************************************/
char *programInfo;
char *stationName;
char *rdsTime;
long stationNameElapsed = millis();
long polling_rds = millis();


void showRDSMsg() {
  if (programInfo == NULL) return;
  // Show programInfo content here
  // Example:  display.print(programInfo);

}

/**
   TODO: process RDS Dynamic PS or Scrolling PS
*/
void showRDSStation() {
  if (stationName == NULL) return;
  // Show the stationName here 

}

void showRDSTime() {
  if (rdsTime == NULL) return;
  // Show rdsTime here 
}


void clearRds() {
  bShow = false;
  programInfo = NULL;
  stationName = NULL;
  rdsTime = NULL;
}

void checkRDS() {
  // You must call getRdsReady before calling any RDS query function. 
  if (rx.getRdsReady()) {
    if (rx.hasRdsInfo() ) {
      programInfo = rx.getRdsProgramInformation();
      stationName = rx.getRdsStationName();
      rdsTime = rx.getRdsTime();  // Gets the UTC Time. Check the getRdsTime documentation for more details. Some stations do not broadcast the right time.
    }
  }
}


void showRds() {
  // Show bRds (Buntton RDS setup)
  // Example
  // if (bRds) {
  //   display.print("RDS");
  // } else {
  //   display.print("   ");
  // }
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
  showRds();
  resetEepromDelay();
}

/**
   Process seek command.
   The seek direction is based on the last encoder direction rotation.
*/
void doSeek() {
  rx.seek(RDA_SEEK_WRAP, seekDirection, showFrequencySeek);  // showFrequency will be called by the seek function during the process.
  delay(PUSH_MIN_DELAY);
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
    if (bShow) clearRds();
    pollin_elapsed = millis();
  }

  if ((millis() - polling_rds) > POLLING_RDS) {
    if (bRds) {
      checkRDS();
    }
    polling_rds = millis();
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
