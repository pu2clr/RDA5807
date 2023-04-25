/*
  UNDER CONSTRUCTION...  
  
  This sketch uses the mcufriend TFT touch Display Shield for Arduino UNO, Mega or DUE.

  Driver LCD: ST7781
  Controller: ILI9341 (ili9481 ili9468, ili9488 hx8357, or r61581)
  Resolution: 240 x 320


  The  purpose  of  this  example  is  to  demonstrate a prototype  receiver based  on  the  RDA5807  and  the
  "PU2CLR RDA5807 Arduino Library". It is not the purpose of this prototype  to provide you a beautiful interface. 
  You can do it better.


  Features:

  RDA5807 and Arduino wireup 

  | Function               | Arduino UNO  (Pin)   |  MEGA2560 or DUE  Pin   |
  | ---------------------- | -------------------- | ----------------------- |
  | SDA                    | 4                    | 20                      |
  | SCL                    | 5                    | 21                      |
  | ENCODER_A              | 2                    | 18                      |
  | ENCODER_B              | 3                    | 19                      |
  | ENCODER PUSH BUTTON    | A0 / 14              | 23                      |
 
  ATTENTION: Your toutch screen needs to be calibrated to work properly.
             To do that, use the TouchScreen_Calibr_native.ino that comes with MCUFRIEND_kbv library.
             Read the TouchScreen_Calibr_native.ino and check the XP, XM , YP and YM pins configuration.
             You might need to change the XP, XM , YP and YM values in the TouchScreen_Calibr_native.ino
             depending on the display you are using.

  Libraries used: SI4735; Adafruit_GFX; MCUFRIEND_kbv; FreeDefaultFonts; TouchScreen;

  Prototype documentation: https://pu2clr.github.io/SI4735/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/

  By Ricardo Lima Caratti, Apr 2023
*/

#include <RDA5807.h>
#include <Adafruit_GFX.h>
#include <MCUFRIEND_kbv.h>
#include <TouchScreen.h>

#include "Rotary.h"


#define BLACK 0x0000
#define BLUE 0x001F
#define RED 0xF800
#define GREEN 0x07E0
#define CYAN 0x07FF
#define MAGENTA 0xF81F
#define YELLOW 0xFFE0
#define WHITE 0xFFFF

#define MINPRESSURE 200
#define MAXPRESSURE 1000

#define ENCODER_PUSH_BUTTON 14  //

// Enconder PINs (interrupt pins used on DUE. All Digital DUE Pins can be used as interrupt)
#define ENCODER_PIN_A 2
#define ENCODER_PIN_B 3

#define MIN_ELAPSED_TIME 100
#define MIN_ELAPSED_RSSI_TIME 150
#define DEFAULT_VOLUME 6

bool audioMute = false;
bool fmStereo = true;
bool touch = false;
uint8_t rssi = 0;
// Encoder control variables
volatile int encoderCount = 0;


uint16_t currentFrequency;
uint16_t previousFrequency;

char buffer[100];
char bufferFreq[10];
char bufferStereo[10];

Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);
MCUFRIEND_kbv tft;
RDA5807 rx;

// ALL Touch panels and wiring is DIFFERENT
// copy-paste results from TouchScreen_Calibr_native.ino
// ATTENTION: if you do not get success at first, check and change the XP, XM , YP and YM pins.
// Read TouchScreen_Calibr_native.ino  (MCUFRIEND shield shares pins with the TFT).

// TFT Touch shield  (my old and original MFUFIEND toutch screen)
// const int XP = 6, XM = A2, YP = A1, YM = 7; //240x320 ID=0x9328
// const int TS_LEFT = 170, TS_RT = 827, TS_TOP = 130, TS_BOT = 868;

// TFT Touch shield 2 (my new kind of mcufriend toutch screen)
const int XP = 7, XM = A1, YP = A2, YM = 6;  //240x320 ID=0x2053
const int TS_LEFT = 155, TS_RT = 831, TS_TOP = 158, TS_BOT = 892;

TouchScreen ts = TouchScreen(XP, YP, XM, YM, 320);
Adafruit_GFX_Button bNextBand, bPreviousBand, bVolumeUp, bVolumeDown, bSeekUp, bSeekDown, bStep, bAudioMute, bAM, bLSB, bUSB, bFM, bMW, bSW, bFilter, bAGC;

int pixel_x, pixel_y;  //Touch_getXY() updates global vars
bool Touch_getXY(void) {
  TSPoint p = ts.getPoint();
  pinMode(YP, OUTPUT);  //restore shared pins
  pinMode(XM, OUTPUT);
  digitalWrite(YP, HIGH);  //because TFT control pins
  digitalWrite(XM, HIGH);
  bool pressed = (p.z > MINPRESSURE && p.z < MAXPRESSURE);
  if (pressed) {
    pixel_x = map(p.x, TS_LEFT, TS_RT, 0, tft.width());  //.kbv makes sense to me
    pixel_y = map(p.y, TS_TOP, TS_BOT, 0, tft.height());
  }
  return pressed;
}

void setup(void) {

  // Encoder pins
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP);


  uint16_t ID = tft.readID();

  if (ID == 0xD3D3)
    ID = 0x9486;  // write-only shield
  tft.begin(ID);
  tft.setRotation(0);  //PORTRAIT
  tft.fillScreen(BLACK);

  // tft.setFont(&FreeSans12pt7b);
  showText(55, 30, 2, NULL, GREEN, "RDA5807");
  showText(55, 90, 2, NULL, YELLOW, "Arduino");
  showText(55, 160, 2, NULL, YELLOW, "Library");
  showText(55, 240, 2, NULL, WHITE, "By PU2CLR");

  delay(3000);

  tft.fillScreen(BLACK);

  showTemplate();

  // Atach Encoder pins interrupt
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup(); // Problem here. The A5 pin is used to reset the display shield. So, when the I2C starts the display stop working. 
  rx.setFrequency(10650) ; 

  // Set up the radio for the current band (see index table variable bandIdx )
  currentFrequency = previousFrequency = rx.getFrequency();
  rx.setVolume(DEFAULT_VOLUME);
  tft.setFont(NULL);  // default font

}

/*
   Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt

*/
void rotaryEncoder() {  // rotary encoder events
  uint8_t encoderStatus = encoder.process();
  if (encoderStatus) {
    if (encoderStatus == DIR_CW) {
      encoderCount = 1;
    } else {
      encoderCount = -1;
    }
  }
}


/*
   Shows a text on a given position; with a given size and font, and with a given color

   @param int x column
   @param int y line
   @param int sz font size
   @param const GFXfont *f font type
   @param uint16_t color
   @param char * msg message
*/
void showText(int x, int y, int sz, const GFXfont *f, uint16_t color, const char *msg) {
  tft.setFont(f);
  tft.setCursor(x, y);
  tft.setTextColor(color);
  tft.setTextSize(sz);
  tft.print(msg);
}

/*
    Prevents blinking during the frequency display.
    Erases the old char/digit value if it has changed and print the new one.
*/
void printText(int col, int line, int sizeText, char *oldValue, char *newValue, uint16_t color, uint8_t space) {
  int c = col;
  char *pOld;
  char *pNew;

  pOld = oldValue;
  pNew = newValue;

  tft.setTextSize(sizeText);

  // prints just changed digits
  while (*pOld && *pNew) {
    if (*pOld != *pNew) {
      tft.setTextColor(BLACK);
      tft.setCursor(c, line);
      tft.print(*pOld);
      tft.setTextColor(color);
      tft.setCursor(c, line);
      tft.print(*pNew);
    }
    pOld++;
    pNew++;
    c += space;
  }

  // Is there anything else to erase?
  tft.setTextColor(BLACK);
  while (*pOld) {
    tft.setCursor(c, line);
    tft.print(*pOld);
    pOld++;
    c += space;
  }

  // Is there anything else to print?
  tft.setTextColor(color);
  while (*pNew) {
    tft.setCursor(c, line);
    tft.print(*pNew);
    pNew++;
    c += space;
  }

  // Save the current content to be tested next time
  strcpy(oldValue, newValue);
}

void showTemplate() {

  // Área reservada à frequência
  tft.drawRect(0, 0, tft.width(), 50, WHITE);

  tft.drawRect(0, 100, tft.width(), 160, CYAN);
  tft.setFont(NULL);
  bPreviousBand.initButton(&tft, 30, 120, 40, 30, WHITE, CYAN, BLACK, (char *)"Band-", 1);
  bNextBand.initButton(&tft, 90, 120, 40, 30, WHITE, CYAN, BLACK, (char *)"Band+", 1);


  bVolumeDown.initButton(&tft, 150, 120, 40, 30, WHITE, CYAN, BLACK, (char *)"Vol-", 1);
  bVolumeUp.initButton(&tft, 210, 120, 40, 30, WHITE, CYAN, BLACK, (char *)"Vol+", 1);
  bSeekDown.initButton(&tft, 30, 160, 40, 30, WHITE, CYAN, BLACK, (char *)"Seek-", 1);
  bSeekUp.initButton(&tft, 90, 160, 40, 30, WHITE, CYAN, BLACK, (char *)"Seek+", 1);
  bAudioMute.initButton(&tft, 150, 160, 40, 30, WHITE, CYAN, BLACK, (char *)"Mute", 1);

  bStep.initButton(&tft, 210, 160, 40, 30, WHITE, CYAN, BLACK, (char *)"Step", 1);

  // bFM.initButton(&tft, 30, 200, 40, 30, WHITE, CYAN, BLACK, (char *)"FM", 1);
  // bMW.initButton(&tft, 90, 200, 40, 30, WHITE, CYAN, BLACK, (char *)"MW", 1);
  // bSW.initButton(&tft, 150, 200, 40, 30, WHITE, CYAN, BLACK, (char *)"SW", 1);
  // bAGC.initButton(&tft, 210, 200, 40, 30, WHITE, CYAN, BLACK, (char *)"ATT", 1);
  // bAM.initButton(&tft, 30, 240, 40, 30, WHITE, CYAN, BLACK, (char *)"AM", 1);
  // bLSB.initButton(&tft, 90, 240, 40, 30, WHITE, CYAN, BLACK, (char *)"LSB", 1);
  // bUSB.initButton(&tft, 150, 240, 40, 30, WHITE, CYAN, BLACK, (char *)"USB", 1);
  // bFilter.initButton(&tft, 210, 240, 40, 30, WHITE, CYAN, BLACK, (char *)"|Y|", 1);

  // Exibe os botões (teclado touch)
  // bNextBand.drawButton(true);
  // bPreviousBand.drawButton(true);

  bVolumeUp.drawButton(true);
  bVolumeDown.drawButton(true);
  bSeekUp.drawButton(true);
  bSeekDown.drawButton(true);
  bStep.drawButton(true);
  bAudioMute.drawButton(true);

  // bFM.drawButton(true);
  // bMW.drawButton(true);
  // bSW.drawButton(true);
  // bAM.drawButton(true);
  // bLSB.drawButton(true);
  // bUSB.drawButton(true);
  // bFilter.drawButton(true);
  // bAGC.drawButton(true);

  showText(0, 270, 1, NULL, YELLOW, "PU2CLR-RDA5807 Arduino Library-Example");
  showText(0, 285, 1, NULL, YELLOW, "DIY - You can make it better.");

  showText(0, 302, 1, NULL, GREEN, "RSSI");
  tft.drawRect(30, 298, 210, 12, CYAN);

  tft.setFont(NULL);
}

/*
    Prevents blinking during the frequency display.
    Erases the old digits if it has changed and print the new digit values.
*/
void showFrequencyValue(int col, int line, char *oldValue, char *newValue, uint16_t color) {

  int c = col;

  // prints just changed digits
  while (*oldValue && *newValue) {
    if (*oldValue != *newValue) {
      tft.drawChar(c, line, *oldValue, BLACK, BLACK, 4);
      tft.drawChar(c, line, *newValue, color, BLACK, 4);
    }
    oldValue++;
    newValue++;
    c += 25;
  }

  // Is there anything else to erase?
  while (*oldValue) {
    tft.drawChar(c, line, *oldValue, BLACK, BLACK, 4);
    oldValue++;
    c += 25;
  }

  // Is there anything else to print?
  while (*newValue) {
    tft.drawChar(c, line, *newValue, color, BLACK, 4);
    newValue++;
    c += 25;
  }
}

void showFrequency() {

  // TO DO: Format current frequency (currentFrequency)


  // showText(10, 10, 4, NULL, color, buffer);
  // showFrequencyValue(10, 10, bufferFreq, buffer, color);
  // tft.setFont(NULL);  // default font
  // strcpy(bufferFreq, buffer);
}

// Will be used by seekStationProgress
void showFrequencySeek(uint16_t freq) {
  previousFrequency = currentFrequency = freq;
  showFrequency();
}


void showStatus() {

  tft.setFont(NULL);
}

void showRSSI() {

  int signalLevel;

  signalLevel = map(rssi, 0, 63, 0, 208);
  tft.fillRect(30, 300, 209, 8, BLACK);
  tft.fillRect(30, 300, signalLevel, 8, (signalLevel > 25) ? CYAN : RED);
}


// RDS 

char *rdsMsg;
char *stationName;
char *rdsTime;
char bufferStatioName[255];
char bufferRdsMsg[255];
char bufferRdsTime[32];

void showRDSMsg() {
  if (strcmp(bufferRdsMsg, rdsMsg) == 0)
    return;
  printText(55, 85, 1, bufferRdsMsg, rdsMsg, GREEN, 6);
  delay(250);
}

void showRDSStation() {
  if (strcmp(bufferStatioName, stationName) == 0)
    return;
  printText(55, 60, 1, bufferStatioName, stationName, GREEN, 6);
  delay(250);
}

void showRDSTime() {

  if (strcmp(bufferRdsTime, rdsTime) == 0)
    return;
  printText(150, 60, 1, bufferRdsTime, rdsTime, GREEN, 6);
  delay(250);
}

void checkRDS() {
}

void showVolume() {
}


/* two buttons are quite simple
*/
void loop() {

  bool down = Touch_getXY();
  bNextBand.press(down && bNextBand.contains(pixel_x, pixel_y));
  bPreviousBand.press(down && bPreviousBand.contains(pixel_x, pixel_y));
  bVolumeUp.press(down && bVolumeUp.contains(pixel_x, pixel_y));
  bVolumeDown.press(down && bVolumeDown.contains(pixel_x, pixel_y));
  bSeekUp.press(down && bSeekUp.contains(pixel_x, pixel_y));
  bSeekDown.press(down && bSeekDown.contains(pixel_x, pixel_y));
  bStep.press(down && bStep.contains(pixel_x, pixel_y));
  bAudioMute.press(down && bAudioMute.contains(pixel_x, pixel_y));
  bFM.press(down && bFM.contains(pixel_x, pixel_y));
  bMW.press(down && bMW.contains(pixel_x, pixel_y));
  bSW.press(down && bSW.contains(pixel_x, pixel_y));
  bAM.press(down && bAM.contains(pixel_x, pixel_y));
  bLSB.press(down && bLSB.contains(pixel_x, pixel_y));
  bUSB.press(down && bUSB.contains(pixel_x, pixel_y));
  bFilter.press(down && bFilter.contains(pixel_x, pixel_y));
  bAGC.press(down && bAGC.contains(pixel_x, pixel_y));

  // Check if the encoder has moved.
  if (encoderCount != 0) {
    if (encoderCount == 1)
      rx.setFrequencyUp();
    else
      rx.setFrequencyDown();

    // Show the current frequency only if it has changed
    currentFrequency = rx.getFrequency();

    encoderCount = 0;
  }

  // Volume
  if (bVolumeUp.justPressed()) {
    // bVolumeUp.drawButton(true);
    rx.setVolumeUp();
    delay(MIN_ELAPSED_TIME);
  }

  if (bVolumeDown.justPressed()) {
    // bVolumeDown.drawButton(true);
    rx.setVolumeDown();
    delay(MIN_ELAPSED_TIME);
  }

  // SEEK Test
  if (bSeekUp.justPressed()) {
    rx.seek(RDA_SEEK_WRAP, 1, showFrequencySeek);  // showFrequency will be called by the seek function during the process.
    showStatus();
  }

  if (bSeekDown.justPressed()) {
    rx.seek(RDA_SEEK_WRAP, 0, showFrequencySeek);  // showFrequency will be called by the seek function during the process.
    showStatus();
  }

  // Mute
  if (bAudioMute.justPressed()) {
    audioMute = !audioMute;
    rx.setMute(audioMute);
    delay(MIN_ELAPSED_TIME);  // waits a little more for releasing the button.
  }

  if (bStep.justPressed()) {
    delay(1);
  }

  if (digitalRead(ENCODER_PUSH_BUTTON) == LOW) {

    delay(250);
  }

  if (currentFrequency != previousFrequency) {
    tft.fillRect(54, 59, 250, 36, BLACK);
    bufferStatioName[0] = bufferRdsMsg[0] = rdsTime[0] = bufferRdsTime[0] = rdsMsg[0] = stationName[0] = '\0';
    showRDSMsg();
    showRDSStation();
    previousFrequency = currentFrequency;
  }

  delay(20);
}
