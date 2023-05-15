/*
  This sketch uses an Arduino Pro Mini, 3.3V (8MZ) with a SPI TFT ST7735 1.8"
  It is also a FM receiver capable to tune your local FM stations.

  Read more on https://pu2clr.github.io/RDA5807/

  Wire up on Arduino UNO, Pro mini

  | Device name               | Device Pin / Description  |  Arduino Pin  |
  | ----------------          | --------------------      | ------------  |
  | Display TFT               |                           |               |
  |                           | RST (RESET)               |      8        |
  |                           | RS  or DC                 |      9        |
  |                           | CS  or SS                 |     10        |
  |                           | SDI                       |     11        |
  |                           | CLK                       |     13        |
  |     RDA5807               |                           |               |
  |                           | VCC                       |   3.3V        |
  |                           | SDIO (pin 8)              |     A4        |
  |                           | SCLK (pin 7)              |     A5        |
  |     Buttons               |                           |               |
  |                           | Volume Up                 |      4        |
  |                           | Volume Down               |      5        |
  |                           | Stereo/Mono               |      6        |
  |                           | RDS ON/off                |      7        |
  |                           | SEEK (encoder button)     |     A0/14     |
  |    Encoder                |                           |               |
  |                           | A                         |       2       |
  |                           | B                         |       3       |

  About 77% of the space occupied by this sketch is due to the library for the TFT Display.


  Prototype documentation: https://pu2clr.github.io/RDA5807/
  PU2CLR RDA5807 API documentation: https://pu2clr.github.io/RDA5807/extras/apidoc/html/

  By PU2CLR, Ricardo,  Feb  2020.
*/

#include <RDA5807.h>

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library for ST7735
#include "Serif_plain_7.h"
#include "Serif_plain_14.h"
#include "DSEG7_Classic_Mini_Regular_30.h"
#include <SPI.h>

#include "Rotary.h"

// TFT MICROYUM or ILI9225 based device pin setup
#define TFT_RST 8
#define TFT_DC 9
#define TFT_CS 10  // SS
#define TFT_SDI 11 // MOSI
#define TFT_CLK 13 // SCK
#define TFT_LED 0  // 0 if wired to +3.3V directly
#define TFT_BRIGHTNESS 200

#define COLOR_BLACK 0x0000
#define COLOR_YELLOW 0xFFE0
#define COLOR_WHITE 0xFFFF
#define COLOR_RED 0xF800
#define COLOR_BLUE 0x001F

#define RESET_PIN 14
#define SDA_PIN A4 // SDA pin used by your Arduino Board

// Enconder PINs
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

// Buttons controllers
#define VOLUME_UP 4       // Volume Up
#define VOLUME_DOWN 5     // Volume Down
#define SWITCH_STEREO 6   // Select Mono or Stereo
#define SWITCH_RDS 7      // SDR ON or OFF
#define SEEK_FUNCTION 14  // Pin A0 / D14

#define POLLING_TIME  2000
#define POLLING_RDS     80

char oldFreq[10];
char oldStereo[10];
char oldRssi[10];
char oldRdsStatus[10];
char oldRdsMsg[65];

bool bSt = true;
bool bRds = true;
bool bShow = false;
uint8_t seekDirection = 1; // 0 = Down; 1 = Up. This value is set by the last encoder direction.

long pollin_elapsed = millis();


// Encoder control variables
volatile int encoderCount = 0;

uint16_t currentFrequency;

// Encoder control
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

// TFT control
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

RDA5807 rx;

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

/*
   Shows the static content on  display
*/
void showTemplate()
{

  int maxX1 = tft.width() - 2;
  int maxY1 = tft.height() - 5;

  tft.fillScreen(COLOR_BLACK);

  tft.drawRect(2, 2, maxX1, maxY1, COLOR_YELLOW);
  tft.drawLine(2, 40, maxX1, 40, COLOR_YELLOW);
  tft.drawLine(2, 60, maxX1, 60, COLOR_YELLOW);
}

/*
  Prevents blinking during the frequency display.
  Erases the old digits if it has changed and print the new digit values.
*/
void printValue(int col, int line, char *oldValue, char *newValue, uint8_t space, uint16_t color)
{
  int c = col;
  char *pOld;
  char *pNew;

  pOld = oldValue;
  pNew = newValue;

  // prints just changed digits
  while (*pOld && *pNew)
  {
    if (*pOld != *pNew)
    {
      // Erases olde value
      tft.setTextColor(COLOR_BLACK);
      tft.setCursor(c, line);
      tft.print(*pOld);
      // Writes new value
      tft.setTextColor(color);
      tft.setCursor(c, line);
      tft.print(*pNew);
    }
    pOld++;
    pNew++;
    c += space;
  }

  // Is there anything else to erase?
  tft.setTextColor(COLOR_BLACK);
  while (*pOld)
  {
    tft.setCursor(c, line);
    tft.print(*pOld);
    pOld++;
    c += space;
  }

  // Is there anything else to print?
  tft.setTextColor(color);
  while (*pNew)
  {
    tft.setCursor(c, line);
    tft.print(*pNew);
    pNew++;
    c += space;
  }

  // Save the current content to be tested next time
  strcpy(oldValue, newValue);
}

/*
   Shows frequency information on Display
*/
void showFrequency()
{
  char freq[10];
  char tmp[10];

  currentFrequency = rx.getFrequency();
  sprintf(tmp, "%5.5u", currentFrequency);

  freq[0] = (tmp[0] == '0') ? ' ' : tmp[0];
  freq[1] = tmp[1];
  freq[2] = tmp[2];
  freq[3] = '\0';
  freq[4] = tmp[3];
  freq[5] = tmp[4];
  freq[6] = '\0';

  tft.setFont(&DSEG7_Classic_Mini_Regular_30);
  tft.setTextSize(1);
  printValue(0, 35, &oldFreq[0], &freq[0], 23, COLOR_RED);
  printValue(80, 35, &oldFreq[4], &freq[4], 23, COLOR_RED);
  tft.setCursor(78, 35);
  tft.print('.');
}

/*
    Show some basic information on display
*/
void showStatus()
{
  oldFreq[0] = oldStereo[0] = oldRdsStatus[0] = oldRdsMsg[0] =  0;

  showFrequency();
  showStereoMono();
  showRSSI();
}

/* *******************************
   Shows RSSI status
*/
void showRSSI()
{
  char rssi[10];
  sprintf(rssi, "%i dBuV", rx.getRssi());
  tft.setFont(&Serif_plain_14);
  tft.setTextSize(1);
  printValue(5, 55, oldRssi, rssi, 11, COLOR_WHITE);
}

void showStereoMono() {
  char stereo[10];
  sprintf(stereo, "%s", (rx.isStereo()) ? "St" : "Mo");
  tft.setFont(&Serif_plain_14);
  tft.setTextSize(1);
  printValue(125, 55, oldStereo, stereo, 15, COLOR_WHITE);
}

/*********************************************************
   RDS
 *********************************************************/
char *rdsMsg;
char *stationName;
char *rdsTime;
char bufferStatioName[16];
char bufferRdsMsg[40];
char bufferRdsTime[20];
long stationNameElapsed = millis();
long polling_rds = millis();
long clear_fifo = millis();

void showRDSMsg()
{
  tft.setFont(&Serif_plain_7);
  rdsMsg[22] = bufferRdsMsg[22] = '\0';   // Truncate the message to fit on display.  You can try scrolling
  if (strcmp(bufferRdsMsg, rdsMsg) == 0)
    return;
  printValue(5, 90, bufferRdsMsg, rdsMsg, 7, COLOR_YELLOW);
  delay(250);
}

/**
   TODO: process RDS Dynamic PS or Scrolling PS
*/
void showRDSStation()
{
  tft.setFont(&Serif_plain_7);
  if (strncmp(bufferStatioName, stationName, 3) == 0)
    return;
  printValue(5, 110, bufferStatioName, stationName, 7, COLOR_YELLOW);
}

void showRDSTime()
{
  tft.setFont(&Serif_plain_7);
  if (strcmp(bufferRdsTime, rdsTime) == 0)
    return;
  printValue(80, 110, bufferRdsTime, rdsTime, 6, COLOR_RED);
  delay(100);
}


void clearRds() {
  tft.fillRect(4, 79, 150, 40, COLOR_BLACK);
  bShow = false;
}

void checkRDS()
{
  // check if RDS currently synchronized; the information are A, B, C and D blocks; and no errors
  if ( rx.hasRdsInfo() ) {
    rdsMsg = rx.getRdsProgramInformation();
    stationName = rx.getRdsStationName();
    rdsTime = rx.getRdsTime();
    if (rdsMsg != NULL)
      showRDSMsg();

    if ((millis() - stationNameElapsed) > 1000)
    {
      if (stationName != NULL)
        showRDSStation();
      stationNameElapsed = millis();
    }

    if (rdsTime != NULL)
      showRDSTime();
  }

  if ( (millis() - clear_fifo) > 60000 ) {
    rx.clearRdsFifo();
    rx.clearRdsBuffer();
    clear_fifo = millis();
    
  }
}

void showRds() {
  char rdsStatus[10];

  tft.setTextSize(1);
  tft.setFont(&Serif_plain_7);
  sprintf(rdsStatus, "RDS %s", (bRds) ? "ON" : "OFF");
  printValue(5, 75, oldRdsStatus, rdsStatus, 9, COLOR_WHITE);
  checkRDS();
}

/*********************************************************

 *********************************************************/

void showSplash()
{
  // Splash
  tft.setFont(&Serif_plain_14);
  tft.setTextSize(1);
  tft.setTextColor(COLOR_YELLOW);
  tft.setCursor(45, 23);
  tft.print("RDA5807");
  tft.setCursor(15, 50);
  tft.print("Arduino Library");
  tft.setCursor(25, 80);
  tft.print("By PU2CLR");
  tft.setFont(&Serif_plain_14);
  tft.setTextSize(0);
  tft.setCursor(12, 110);
  tft.print("Ricardo L. Caratti");
  delay(4000);
}

void setup()
{
  Serial.begin(9600);
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

  // Push button pin
  pinMode(VOLUME_UP, INPUT_PULLUP);
  pinMode(VOLUME_DOWN, INPUT_PULLUP);
  pinMode(SWITCH_STEREO, INPUT_PULLUP);
  pinMode(SWITCH_RDS, INPUT_PULLUP);
  pinMode(SEEK_FUNCTION, INPUT_PULLUP);

  tft.initR(INITR_BLACKTAB);
  tft.fillScreen(COLOR_BLACK);
  tft.setTextColor(COLOR_BLUE);
  tft.setRotation(1);

  showSplash();
  showTemplate();

  // Encoder interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup();
  rx.setVolume(6);
  rx.setMono(false); // Force stereo
  // rx.setRBDS(true);  //  set RDS and RBDS. See setRDS.
  rx.setRDS(true);
  rx.setRdsFifo(true);

  rx.setFrequency(10650); // It is the frequency you want to select in MHz multiplied by 100.
  rx.setSeekThreshold(50); // Sets RSSI Seek Threshold (0 to 127)
  showStatus();
}


void doStereo() {
  rx.setMono((bSt = !bSt));
  bShow =  true;
  showStereoMono();
  delay(100);
}

void doRds() {
  rx.setRDS((bRds = !bRds));
  showRds();
}

/**
   Process seek command.
   The seek direction is based on the last encoder direction rotation.
*/
void doSeek() {
  rx.seek(RDA_SEEK_WRAP, seekDirection, showFrequency);  // showFrequency will be called by the seek function during the process.
  delay(200);
  bShow =  true;
  showFrequency();
}

void loop()
{

  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if (encoderCount == 1) {
      rx.setFrequencyUp();
      seekDirection = RDA_SEEK_UP;
    }
    else {
      rx.setFrequencyDown();
      seekDirection = RDA_SEEK_DOWN;
    }
    showFrequency();
    bShow = true;
    encoderCount = 0;
  }

  if (digitalRead(VOLUME_UP) == LOW)
    rx.setVolumeUp();
  else if (digitalRead(VOLUME_DOWN) == LOW)
    rx.setVolumeDown();
  else if (digitalRead(SWITCH_STEREO) == LOW)
    doStereo();
  else if (digitalRead(SWITCH_RDS) == LOW)
    doRds();
  else if (digitalRead(SEEK_FUNCTION) == LOW)
    doSeek();

  if ( (millis() - pollin_elapsed) > POLLING_TIME ) {
    showRSSI();
    showStereoMono();
    if ( bShow ) clearRds();
    pollin_elapsed = millis();
  }

  if ( (millis() - polling_rds) > POLLING_RDS) {
    if ( bRds ) {
      showRds();
    }
    polling_rds = millis();
  }

  delay(5);
}
