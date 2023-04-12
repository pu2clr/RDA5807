/*

  UNDER CONSTRUCTION...

  This sketch runs on ESP8266 device.

  ESP8266/ESP12F and components wire up.

  | Device name               | Device Pin / Description      |  ESP8266      |
  | ----------------          | ----------------------------- | ------------  |
  |    OLED                   |                               |               |
  |                           | SDA/SDIO                      |  GPIO4        |
  |                           | SCL/SCLK                      |  GPIO5        |
  |    RDA5807                |                               |               |
  |                           | +Vcc 3.3V [1]                 |   3.3V        |
  |                           | SDIO (pin 8)                  |   GPIO4       |
  |                           | SCLK (pin 7)                  |   GPIO5       |
  |    Encoder                |                               |               |
  |                           | A                             |  GPIO 13      |
  |                           | B                             |  GPIO 14      |
  |                           | PUSH BUTTON (encoder)         |  GPIO 12      |


  ATTENTION: Read the file user_manual.txt


  References:
  Prototype documentation: https://pu2clr.github.io/SI4735/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/
  ESP8266 pinout: https://randomnerdtutorials.com/esp8266-pinout-reference-gpios/
                  https://s3.amazonaws.com/randomnerdtutorials/jhdfsASDFJEWJjsdfajdsafJDAFSJafd/ESP8266_Pinout_Diagrams.pdf

  ESP8266 Arduino Core’s documentation - https://arduino-esp8266.readthedocs.io/en/2.5.2/reference.html
  Digital pins 0—15 can be INPUT, OUTPUT, or INPUT_PULLUP.
  Pin 16 can be INPUT, OUTPUT or INPUT_PULLDOWN_16.
  At startup, pins are configured as INPUT."

  By PU2CLR, Ricardo, May  2021.
*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include "EEPROM.h"
#include <RDA5807.h>
#include "DSEG7_Classic_Regular_16.h"
#include "Rotary.h"

// Enconder PINs
#define ENCODER_PIN_A 13 // GPIO13
#define ENCODER_PIN_B 14 // GPIO14

#define ENCODER_PUSH_BUTTON 12 // SEEK Station - Working well

#define POLLING_TIME 2000
#define RDS_MSG_TYPE_TIME 20000
#define POLLING_RDS 20

#define STORE_TIME 10000 // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
#define PUSH_MIN_DELAY 300
#define EEPROM_SIZE 512
#define MIN_ELAPSED_TIME 300
#define ELAPSED_COMMAND 5000 // time to turn off the last command controlled by encoder. Time to goes back to the FVO control
#define ELAPSED_CLICK 1000   // time to check the double click commands

// MENU CONTROL

const char *menu[] = {"Volume", "RDS", "Stereo", "Step"};
int8_t menuIdx = 0;
const int lastMenu = 3;
int8_t currentMenuCmd = -1;

bool cmdVolume = false;
bool cmdStep = false;
bool cmdRds = false;
bool cmdStereo = false;
bool cmdMenu = false;

long elapsedClick = millis();
long elapsedCommand = millis();

uint8_t nivelMenu = 0;

// END MENU CONTROL

const uint8_t app_id = 43; // Useful to check the EEPROM content before processing useful data
const int eeprom_address = 0;
long storeTime = millis();

bool bSt = true;
bool bRds = false;
bool bShow = false;
uint8_t seekDirection = 1; // 0 = Down; 1 = Up. This value is set by the last encoder direction.

long pollin_elapsed = millis();

// Encoder control variables
volatile int encoderCount = 0;
uint16_t currentFrequency;
uint16_t previousFrequency;

int8_t step = 0;
uint8_t tabStep[] = {100, 200, 50, 25};

// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

RDA5807 rx;

void setup()
{
  // Encoder pins
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  display.display();
  display.setTextColor(SSD1306_WHITE);

  // Splash - Remove or change it for your introduction text.
  display.clearDisplay();
  print(0, 0, NULL, 2, "PU2CLR");
  print(0, 15, NULL, 2, "ESP8266");
  display.display();
  delay(1500);
  display.clearDisplay();
  print(0, 0, NULL, 2, "RDA5807");
  print(0, 15, NULL, 2, "Arduino");
  display.display();
  // End Splash

  delay(1500);
  display.clearDisplay();

  EEPROM.begin(EEPROM_SIZE);

  // If you want to reset the eeprom, keep the ENCODER PUSH BUTTON  pressed during statup
  if (digitalRead(ENCODER_PUSH_BUTTON) == LOW)
  {
    EEPROM.write(eeprom_address, 0);
    EEPROM.commit();
    print(0, 0, NULL, 2, "EEPROM RESETED");
    delay(3000);
    display.clearDisplay();
  }

  // ICACHE_RAM_ATTR void rotaryEncoder(); see rotaryEncoder implementation below.
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_A), rotaryEncoder, CHANGE);
  attachInterrupt(digitalPinToInterrupt(ENCODER_PIN_B), rotaryEncoder, CHANGE);

  rx.setup();

  // Checking the EEPROM content
  if (EEPROM.read(eeprom_address) == app_id)
  {
    readAllReceiverInformation();
  }
  else
  {
    // Default values
    rx.setVolume(6);
    rx.setMono(false); // Force stereo
    // rx.setRBDS(true);  //  set RDS and RBDS. See setRDS.
    rx.setRDS(true);
    rx.setRdsFifo(true);
    currentFrequency = previousFrequency = 10390;
  }

  rx.setFrequency(currentFrequency); // It is the frequency you want to select in MHz multiplied by 100.
  rx.setSeekThreshold(50);           // Sets RSSI Seek Threshold (0 to 127)
  showStatus();
}

// MENU CONTROL IMPLEMENTATION
/**
    Set all command flags to false
    When all flags are disabled (false), the encoder controls the frequency
*/
void disableCommands()
{
  cmdVolume = false;
  cmdStep = false;
  cmdRds = false;
  cmdStereo = false;
  cmdMenu = false;
  nivelMenu = 0;
}

/**
 * Starts the MENU action process
 * {"Volume", "RDS", "Stereo", "Step"};
 */
void doCurrentMenuCmd()
{
  disableCommands();
  switch (currentMenuCmd)
  {
  case 0: // VOLUME
    cmdVolume = true;
    showVolume();
    break;
  case 1: // RDS
    cmdRds = true;
    showRds();
    break;
  case 2: // STEREO
    cmdStereo = true;
    showStereoStatus();
    break;
  case 3: // STEP
    cmdStep = true;
    showStepStatus();
    break;
  default:
    showStatus();
    break;
  }
  currentMenuCmd = -1;
  elapsedCommand = millis();
}

/**
 *  Menu options selection
 */
void doMenu(int8_t v)
{
  menuIdx = (v == 1) ? menuIdx + 1 : menuIdx - 1;
  if (menuIdx > lastMenu)
    menuIdx = 0;
  else if (menuIdx < 0)
    menuIdx = lastMenu;

  showMenu();
  delay(MIN_ELAPSED_TIME); // waits a little more for releasing the button.
  elapsedCommand = millis();
}

/**
 * Returns true if the current status is Menu command
 */
bool isMenuMode()
{
  return (cmdMenu | cmdStep | cmdVolume | cmdRds | cmdStereo);
}

/**
 * Show cmd on display. It means you are setting up something.
 */
void showCommandStatus(char *currentCmd)
{
  display.fillRect(40, 0, 50, 8, SSD1306_BLACK);
  display.setCursor(40, 0);
  display.print(currentCmd);
  display.display();
}

/**
 * Show menu options
 */
void showMenu()
{
  display.clearDisplay();
  display.setCursor(0, 10);
  display.print(menu[menuIdx]);
  display.display();
  showCommandStatus((char *)"Menu");
}

// END MENU CONTROL IMPLEMENTATION

/**
 * Prints a given content on display
 */
void print(uint8_t col, uint8_t lin, const GFXfont *font, uint8_t textSize, const char *msg)
{
  display.setFont(font);
  display.setTextSize(textSize);
  display.setCursor(col, lin);
  display.print(msg);
  display.display();
}

void printParam(const char *msg)
{
  display.fillRect(0, 10, 128, 10, SSD1306_BLACK);
  print(0, 10, NULL, 1, msg);
  display.display();
}

/*
   writes the conrrent receiver information into the eeprom.
   The EEPROM.write avoid write the same data in the same memory position. It will save unnecessary recording.
*/
void saveAllReceiverInformation()
{
  EEPROM.begin(EEPROM_SIZE);

  // The write function/method writes data only if the current data is not equal to the stored data.
  EEPROM.write(eeprom_address, app_id);
  EEPROM.write(eeprom_address + 1, rx.getVolume());          // stores the current Volume
  EEPROM.write(eeprom_address + 2, currentFrequency >> 8);   // stores the current Frequency HIGH byte for the band
  EEPROM.write(eeprom_address + 3, currentFrequency & 0xFF); // stores the current Frequency LOW byte for the band
  EEPROM.write(eeprom_address + 4, (uint8_t)bRds);
  EEPROM.write(eeprom_address + 5, (uint8_t)bSt);
  EEPROM.write(eeprom_address + 6, step);
  EEPROM.commit();
  EEPROM.end();
}

/**
 * reads and set the last receiver status.
 */
void readAllReceiverInformation()
{
  rx.setVolume(EEPROM.read(eeprom_address + 1));
  currentFrequency = EEPROM.read(eeprom_address + 2) << 8;
  currentFrequency |= EEPROM.read(eeprom_address + 3);
  previousFrequency = currentFrequency;

  bRds = (bool)EEPROM.read(eeprom_address + 4); // Rescue RDS status (ON / OFF)
  rx.setRDS(bRds);
  rx.setRdsFifo(bRds);

  bSt = (bool)EEPROM.read(eeprom_address + 5); // Rescue Stereo status (ON / OFF)
  rx.setMono(bSt);

  step = EEPROM.read(eeprom_address + 6); // Rescue step: 0 = 100; 1 = 200; 2 = 50; 3 = 25 (in kHz)
  rx.setSpace(step);                      // See tabStep
}

/*
   To store any change into the EEPROM, it is needed at least STORE_TIME  milliseconds of inactivity.
*/
void resetEepromDelay()
{
  delay(PUSH_MIN_DELAY);
  storeTime = millis();
  elapsedCommand = millis();
  previousFrequency = 0;
}

/**
 * Reads encoder via interrupt
 * Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
 * if you do not add ICACHE_RAM_ATTR declaration, the system will reboot during attachInterrupt call.
 * With ICACHE_RAM_ATTR macro you put the function on the RAM.
 */
ICACHE_RAM_ATTR void rotaryEncoder()
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

  char freq[10];
  currentFrequency = rx.getFrequency();
  rx.convertToChar(currentFrequency, freq, 5, 3, ',', true);

  display.setFont(&DSEG7_Classic_Regular_16);
  // display.clearDisplay();
  display.setCursor(20, 27);
  display.print(freq);
  display.setFont(NULL);
  display.setTextSize(1);
  display.display();
}

void showFrequencySeek()
{
  display.clearDisplay();
  showFrequency();
}

/*
    Show some basic information on display
*/
void showStatus()
{
  display.clearDisplay();
  showFrequency();
  showStereoStatusMono();
  showRSSI();

  if (bRds)
  {
    showRds();
  }

  display.display();
}

/*
 *  Shows the volume level on LCD
 */
void showVolume()
{
  char volAux[12];
  sprintf(volAux, "VOLUME: %2u", rx.getVolume());
  printParam(volAux);
}

/**
 *   Shows the current step
 */
void showStepStatus()
{
  char aux[12];
  sprintf(aux, "STEP: %3u", tabStep[step]);
  printParam(aux);
}

void showStereoStatus()
{
  char aux[12];
  sprintf(aux, "STEREO: %s", (bSt) ? "ON" : "OFF");
  printParam(aux);
}

void showRdsStatus()
{
  char aux[12];
  sprintf(aux, "RDS: %s", (bRds) ? "ON" : "OFF");
  printParam(aux);
}

/* *******************************
   Shows RSSI status
*/
void showRSSI()
{
  char rssi[12];
  rx.convertToChar(rx.getRssi(), rssi, 3, 0, '.');
  strcat(rssi, "dB");
  display.setCursor(90, 0);
  display.print(rssi);
}

void showStereoStatusMono()
{
  display.setCursor(0, 0);
  if (bSt)
  {
    display.print("ST");
  }
  else
  {
    display.print("MO");
  }
}

/*********************************************************
   RDS
 *********************************************************/
char *rdsMsg;
char *stationName;
char *rdsTime;
int currentMsgType = 0;
long polling_rds = millis();
long timeTextType = millis(); // controls the type of each text will be shown (Message, Station Name or time)

int rdsMsgIndex = 0; // controls the part of the rdsMsg text will be shown on LCD 16x2 Display

/**
  showRDSMsg - Shows the Program Information
*/
void showRDSMsg()
{
  char txtAux[17];

  if (rdsMsg == NULL)
    return;

  rdsMsg[41] = '\0'; // Truncate the message to fit on display line
  strncpy(txtAux, &rdsMsg[rdsMsgIndex], 16);
  txtAux[16] = '\0';
  rdsMsgIndex += 4;
  if (rdsMsgIndex > 40)
    rdsMsgIndex = 0;
  display.setCursor(0, 0);
  display.print(txtAux);
}

/**
   showRDSStation - Shows the
*/
void showRDSStation()
{
  char txtAux[17];

  if (stationName == NULL)
    return;

  stationName[16] = '\0';
  strncpy(txtAux, stationName, 16);
  txtAux[16] = '\0';
  display.setCursor(0, 0);
  display.print(txtAux);
}

void showRDSTime()
{
  char txtAux[17];

  if (rdsTime == NULL)
    return;

  rdsTime[16] = '\0';
  strncpy(txtAux, rdsTime, 16);
  txtAux[16] = '\0';
  display.setCursor(0, 0);
  display.print(txtAux);
}

void clearRds()
{
  bShow = false;
  rdsMsg = NULL;
  stationName = NULL;
  rdsTime = NULL;
  rdsMsgIndex = currentMsgType = 0;
}

void checkRDS()
{
  // check if RDS currently synchronized; the information are A, B, C and D blocks; and no errors
  if (rx.getRdsReady() && rx.hasRdsInfo())
  {
    rdsMsg = rx.getRdsText2A();
    stationName = rx.getRdsText0A();
    rdsTime = rx.getRdsTime();
  }
}

void showRds()
{

  display.setCursor(50, 0);
  if (bRds)
    display.print("Rds");
  else
    display.print("   ");

  if (currentMsgType == 0)
    showRDSMsg();
  else if (currentMsgType == 1)
    showRDSStation();
  else if (currentMsgType == 2)
    showRDSTime();
}

/*********************************************************

 *********************************************************/

/**
 * Sets the audio volume
 */
void doVolume(int8_t v)
{
  if (v == 1)
    rx.setVolumeUp();
  else
    rx.setVolumeDown();

  showVolume();
  resetEepromDelay();
}

void doStep(int8_t v)
{

  if (v == 1)
    step++;
  else
    step--;

  if (step > 3)
    step = 0;
  else if (step < 0)
    step = 3;

  rx.setSpace((uint8_t)step); // See tabStep: 0 = 100; 1 = 200; 2 = 50; 3 = 25 (in kHz)
  showStepStatus();
  resetEepromDelay();
}

void doStereo(int8_t v)
{
  bSt = (bool)v;
  rx.setMono(bSt);
  showStereoStatusMono();
  resetEepromDelay();
}

void doRds(int8_t v)
{
  bRds = (bool)v;
  rx.setRDS(bRds);
  rdsMsgIndex = currentMsgType = 0;
  showRdsStatus();
  resetEepromDelay();
}

/**
   Process seek command.
   The seek direction is based on the last encoder direction rotation.
*/
void doSeek()
{
  rx.seek(RDA_SEEK_WRAP, seekDirection, showFrequencySeek); // showFrequency will be called by the seek function during the process.
  delay(200);
  bShow = true;
  showStatus();
}

void loop()
{

  // Check if the encoder has moved.
  if (encoderCount != 0)
  {
    if (cmdMenu)
      doMenu(encoderCount);
    else if (cmdVolume)
      doVolume(encoderCount);
    else if (cmdStep)
      doStep(encoderCount);
    else if (cmdStereo)
      doStereo(encoderCount);
    else if (cmdRds)
      doRds(encoderCount);
    else
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
      // Show the current frequency only if it has changed
      currentFrequency = rx.getFrequency();
      showStatus();
      bShow = true;
    }

    storeTime = millis();
    elapsedClick = millis();
    
    encoderCount = 0;
  }
  else
  {
    if (digitalRead(ENCODER_PUSH_BUTTON) == LOW)
    {
      nivelMenu++;
      if (nivelMenu == 1)
      {
        cmdMenu = true;
        currentMenuCmd = menuIdx;
        doCurrentMenuCmd();
      }
      else if (nivelMenu >= 2)
      {
        disableCommands();
        showStatus();
        cmdMenu = false;
        nivelMenu = 0;
      }
      delay(MIN_ELAPSED_TIME);
      elapsedCommand = millis();
      elapsedClick = millis();
    }
  }

  if ((millis() - elapsedClick) > ELAPSED_CLICK)
  {
    nivelMenu = 0;
    elapsedClick = millis();
  }

  if ((millis() - pollin_elapsed) > POLLING_TIME)
  {
    showStatus();
    if (bShow)
      clearRds();
    pollin_elapsed = millis();
  }

  if ((millis() - polling_rds) > POLLING_RDS)
  {
    if (bRds)
    {
      checkRDS();
    }
    polling_rds = millis();
  }

  if ((millis() - timeTextType) > RDS_MSG_TYPE_TIME)
  {
    // Toggles the type of message to be shown - See showRds function
    currentMsgType++;
    if (currentMsgType > 2)
      currentMsgType = 0;
    timeTextType = millis();
  }

  // Show the current frequency only if it has changed
  if ((currentFrequency = rx.getFrequency()) != previousFrequency)
  {
    clearRds();
    if ((millis() - storeTime) > STORE_TIME)
    {
      saveAllReceiverInformation();
      storeTime = millis();
      previousFrequency = currentFrequency;
    }
  }

  // Disable commands control
  if ((millis() - elapsedCommand) > ELAPSED_COMMAND)
  {
    if (isMenuMode())
      showStatus();
    disableCommands();
    elapsedCommand = millis();
  }

  delay(5);
}
