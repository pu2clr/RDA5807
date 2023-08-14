/*
  
  This sketch works on Atmega328 and LGT8FX based board.

  Features:
      RDS - Program information, Station Name, Program Type and Time
      Bass, Volume and Mute control
      Seek Station

  SEEK COMMAND - On FM or AM modes press the encoder push button.
                 The direction of the seek up or seek down will  depend on the last rotating movement of the encoder, 
                 clockwise and counterclockwise respectively.


  Nano/LGT8FX and RDA5807 wireup

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
  |                           | VCC                       |    3.3V       |
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
  |                           | B                         |       3       


   By Ricardo Lima Caratti, 2020.
*/

#include <RDA5807.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <EEPROM.h>
#include "DSEG7_Classic_Regular_16.h"
#include "Rotary.h"

// Please, check the ATtiny84 physical pins

#define VOLUME_UP 8
#define VOLUME_DOWN 9
#define AUDIO_MUTE  10
#define AUDIO_BASS  11
#define SEEK_STATION 14 // A0 = D14

// Enconder PINs
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3


#define STORE_TIME 10000    // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
#define PUSH_MIN_DELAY 300  // Minimum waiting time after an action

const uint8_t app_id = 43;  // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();

volatile int encoderCount = 0;
uint8_t seekDirection = 1; // 0 = Down; 1 = Up. This value is set by the last encoder direction.
long elapsedTimeEncoder = millis();

uint16_t currentFrequency;
uint16_t previousFrequency;

// Encoder control
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
Adafruit_SSD1306 oled = Adafruit_SSD1306(128, 32, &Wire);

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

  oled.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32
  oled.display();
  oled.setTextColor(SSD1306_WHITE);

  // Splash - Remove or change it for your introduction text.
  oled.clearDisplay();
  print(35, 0, NULL, 1, "RDA5807");
  print(15, 10, NULL, 1, "Arduino Library");
  oled.display();
  delay(1500);
  print(38, 20, NULL, 1, "By PU2CLR");
  oled.display();
  delay(3000);
  oled.clearDisplay();
  oled.display();
  // End Splash


  // If you want to reset the eeprom, keep the ENCODER PUSH BUTTON  pressed during statup
  if (digitalRead(SEEK_STATION) == LOW) {
    EEPROM.write(eeprom_address, 0);
    oled.setCursor(0, 0);
    oled.print("RESET");
    oled.display();
    delay(1500);
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
    currentFrequency = previousFrequency = 9390;
    // currentFrequency = previousFrequency = 10650;
  }

  rx.setFrequency(currentFrequency);  // It is the frequency you want to select in MHz multiplied by 100.
  rx.setSeekThreshold(50);            // Sets RSSI Seek Threshold (0 to 127)
  rx.setLnaPortSel(3);                // LNA setup

  rx.setRDS(true);
  rx.setRdsFifo(true);

  showStatus();
}


void saveAllReceiverInformation() {
  EEPROM.update(eeprom_address, app_id);
  EEPROM.update(eeprom_address + 1, rx.getVolume());           // stores the current Volume
  EEPROM.update(eeprom_address + 2, currentFrequency >> 8);    // stores the current Frequency HIGH byte for the band
  EEPROM.update(eeprom_address + 3, currentFrequency & 0xFF);  // stores the current Frequency LOW byte for the band
}

void readAllReceiverInformation() {
  rx.setVolume(EEPROM.read(eeprom_address + 1));
  currentFrequency = EEPROM.read(eeprom_address + 2) << 8;
  currentFrequency |= EEPROM.read(eeprom_address + 3);
  previousFrequency = currentFrequency;

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


/**
 * Prints a given content on display
 */
void print(uint8_t col, uint8_t lin, const GFXfont *font, uint8_t textSize, const char *msg)
{
  oled.setFont(font);
  oled.setTextSize(textSize);
  oled.setCursor(col, lin);
  oled.print(msg);
  oled.display();
}

void showStatus()
{
  oled.clearDisplay();
  showFrequency();

  oled.setCursor(0, 0);
  if (rx.isStereo())
    oled.print("ST");
  else
    oled.print("MO");

  oled.setCursor(100, 0);
  oled.print("MHz");  

  oled.display();
}

void showFrequency()
{
  oled.setFont(&DSEG7_Classic_Regular_16);
  oled.setCursor(20, 20);
  oled.print(rx.formatCurrentFrequency());
  oled.setFont(NULL);
  oled.setTextSize(1);
  oled.display();
}


/*********************************************************
   RDS
 *********************************************************/
char *programInfo;
char *stationName;
char *rdsTime;

uint8_t controlInfo = 0;
long delayContronInfo = millis();

long delayStationName = millis();
long delayProgramInfo = millis();
long delayTime = millis();
uint8_t idxProgramInfo = 0;



void showLineRDS( char *info) {
  oled.fillRect(0, 25, 120, 20, BLACK);
  oled.setCursor(0, 25);
  oled.print(info);
  oled.display();
}

void showProgramInfo() {

  char aux[20];

  if (programInfo == NULL || (strlen(programInfo) < 2) || (millis() - delayProgramInfo) < 1000) return;

  strncpy(aux, &programInfo[idxProgramInfo], 18);
  aux[18] = '\0';
  showLineRDS(aux);
  idxProgramInfo += 3;
  if (idxProgramInfo > 60) idxProgramInfo = 0;
  delayProgramInfo = millis();
}

/**
   TODO: show RDS information 
*/
void showRDSStation() {

  if (stationName == NULL || strlen(stationName) < 2 || (millis() - delayStationName) < 6000) return;

  stationName[8] = 0;
  showLineRDS(stationName);
  delayStationName = millis();

}

void showRDSTime() {

  char *p;
  char aux[30];
  if (rdsTime == NULL || (millis() - delayTime) < 60000) return;

  switch (rx.getRdsProgramType()) {
    case 1: p = (char *) "News"; break;
    case 3:
    case 4: p = (char *) "Info/Sport"; break;
    case 7: p = (char *) "Culture"; break;
    case 10:
    case 11:
    case 12:
    case 13:
    case 14:
    case 15: p = (char *) "Music"; break;
    default: p = (char *) "Other";
  }

  strcpy(aux,p);
  strcat(aux," - ");
  strcat(aux,rdsTime);
  
  showLineRDS(aux);
  delayTime = millis();
}


void clearRds() {
  programInfo = NULL;
  stationName = NULL;
  rdsTime = NULL;
  rx.clearRdsBuffer();
}

void checkRDS() {
  // You must call getRdsReady before calling any RDS query function.
  if (rx.getRdsReady()) {
    if (rx.hasRdsInfo()) {
      if ( controlInfo == 0 ) {
        programInfo = rx.getRdsProgramInformation();
        showProgramInfo();
      }
      else if ( controlInfo == 1 ) {
        stationName = rx.getRdsStationName();
        showRDSStation();
      }
      else if ( controlInfo == 2 ) { 
        rdsTime = rx.getRdsLocalTime();  // Gets the Local Time. Check the getRdsTime documentation for more details. Some stations do not broadcast the right time.
        showRDSTime();
      }

      if ( (millis() - delayContronInfo) > 25000 ) {
        controlInfo++;
        if (controlInfo > 2) controlInfo = 0;
        delayContronInfo = millis();
      }

    }
  }
}


void loop()
{

  bool bVolu,bVold, bMute, bBass;
  bVolu = bVold =  bMute = bBass = false;

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
  else if ( (bVolu = digitalRead(VOLUME_UP)) == LOW) 
    rx.setVolumeUp();
  else if ( (bVold = digitalRead(VOLUME_DOWN) ) == LOW)
    rx.setVolumeDown();
  else if ( (bMute = digitalRead(AUDIO_MUTE)) == LOW)
    rx.setMute(!rx.getMute());  
  else if (digitalRead(SEEK_STATION) == LOW) {
    rx.seek(RDA_SEEK_WRAP, seekDirection, showStatus); // showFrequency will be called by the seek function during the process.
    delay(200);
  }
  else if ( (bBass = digitalRead(AUDIO_BASS)) == LOW)
    rx.setBass(!rx.getBass());

  // Delay a litle more if some button was pressed
  if (bVolu || bVold || bMute || bBass ) delay(PUSH_MIN_DELAY);

  checkRDS();

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
