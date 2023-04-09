/*
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
  |    Buttons                |                               |               |
  |                           | Volume Up                     |   GPIO01      |
  |                           | Volume Down                   |   GPIO02      |
  |                           | Stereo/Mono                   |   GPIO15      |
  |                           | RDS ON/off                    |   GPIO16      |
  |                           | SEEK (encoder button)         |   GPIO12      |
  |    Encoder                |                               |               |
  |                           | A                             |  GPIO 13      |
  |                           | B                             |  GPIO 14      |
  |                           | PUSH BUTTON (encoder)         |  GPIO 12      |

 
  ATTENTION: Read the file user_manual.txt
  Prototype documentation: https://pu2clr.github.io/SI4735/
  PU2CLR Si47XX API documentation: https://pu2clr.github.io/SI4735/extras/apidoc/html/

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
#define ENCODER_PIN_A 13           // GPIO13 
#define ENCODER_PIN_B 14           // GPIO14 

// Buttons controllers

// Buttons controllers
#define VOLUME_UP 1       // Volume Up
#define VOLUME_DOWN 2     // Volume Down
#define SWITCH_STEREO 15  // Select Mono or Stereo
#define SWITCH_RDS 16     // SDR ON or OFF
#define SEEK_FUNCTION 12  // SEEK Station

#define POLLING_TIME 2000
#define POLLING_RDS 20

#define STORE_TIME 10000  // Time of inactivity to make the current receiver status writable (10s / 10000 milliseconds).
#define PUSH_MIN_DELAY 300

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

// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

RDA5807 rx;

void setup()
{
  // Encoder pins
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);
  pinMode(SEEK_FUNCTION, INPUT_PULLUP);  

  // Push button pin
  pinMode(VOLUME_UP, INPUT_PULLUP);
  pinMode(VOLUME_DOWN, INPUT_PULLUP);
  pinMode(SWITCH_STEREO, INPUT_PULLUP);
  pinMode(SWITCH_RDS, INPUT_PULLUP);

  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // Address 0x3C for 128x32

  display.display();
  display.setTextColor(SSD1306_WHITE);

  // Splash - Remove or change it for your introduction text.
  display.clearDisplay();
  print(0, 0, NULL, 2, "PU2CLR");
  print(0, 15, NULL, 2, "ESP8266");
  display.display();
  delay(2000);
  display.clearDisplay();
  print(0, 0, NULL, 2, "RDA5807");
  print(0, 15, NULL, 2, "Arduino");
  display.display();
  // End Splash

  delay(2000);
  display.clearDisplay();

  EEPROM.begin(EEPROM_SIZE);

  // If you want to reset the eeprom, keep the VOLUME_UP button pressed during statup
  if (digitalRead(SEEK_FUNCTION) == LOW)
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
  showStatus();

}


/**
 * Prints a given content on display 
 */
void print(uint8_t col, uint8_t lin, const GFXfont *font, uint8_t textSize, const char *msg) {
  display.setFont(font);
  display.setTextSize(textSize);
  display.setCursor(col,lin);
  display.print(msg);
}

void printParam(const char *msg) {
 display.fillRect(0, 10, 128, 10, SSD1306_BLACK); 
 print(0,10,NULL,1, msg);
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
  EEPROM.write(eeprom_address + 4, (uint8_t) bRds);
  EEPROM.write(eeprom_address + 5, (uint8_t) bSt);

  EEPROM.end();

}

/**
 * reads and set the last receiver status. 
 */
void readAllReceiverInformation()
{
  rx.setVolume(EEPROM.read(eeprom_address + 1));
  currentFrequency = EEPROM.read(eeprom_address + 2) << 8;
  currentFrequency |= EEPROM.read(eeprom_address +3);
  previousFrequency = currentFrequency;

  bRds = (bool) EEPROM.read(eeprom_address + 4);
  rx.setRDS(bRds);
  rx.setRdsFifo(bRds);

  bSt = (bool) EEPROM.read(eeprom_address + 5);
  rx.setMono(bSt);
}


/*
   To store any change into the EEPROM, it is needed at least STORE_TIME  milliseconds of inactivity.
*/
void resetEepromDelay()
{
  delay(PUSH_MIN_DELAY);
  storeTime = millis();
  previousFrequency = 0;
}




/**
 * Reads encoder via interrupt
 * Use Rotary.h and  Rotary.cpp implementation to process encoder via interrupt
 * if you do not add ICACHE_RAM_ATTR declaration, the system will reboot during attachInterrupt call. 
 * With ICACHE_RAM_ATTR macro you put the function on the RAM.
 */
ICACHE_RAM_ATTR void  rotaryEncoder()
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
  rx.convertToChar(currentFrequency,freq,5,3,',', true);

  display.setFont(&DSEG7_Classic_Regular_16);
  display.clearDisplay();
  display.setCursor(20, 24);
  display.print(freq);
  display.setCursor(90,15);
  display.setFont(NULL);
  display.setTextSize(1);
  display.print("Mhz");
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
  lcd.clear();
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
void showRSSI()
{
  char rssi[12];
  rx.convertToChar(rx.getRssi(),rssi,3,0,'.');
  strcat(rssi,"dB");
  lcd.setCursor(13, 1);
  lcd.print(rssi);
}

void showStereoMono() {
  lcd.setCursor(0, 2);
  if ( bSt ) { 
    lcd.print("ST");
  } else {
    lcd.print("MO");
  }
}

/*********************************************************
   RDS
 *********************************************************/
char *rdsMsg;
char *stationName;
char *rdsTime;
int  currentMsgType = 0; 
long polling_rds = millis();
long timeTextType = millis();  // controls the type of each text will be shown (Message, Station Name or time)

int rdsMsgIndex = 0;  // controls the part of the rdsMsg text will be shown on LCD 16x2 Display


/**
  showRDSMsg - Shows the Program Information
*/
void showRDSMsg()
{
  char txtAux[17];

  if (rdsMsg == NULL) return;

  rdsMsg[41] = '\0';   // Truncate the message to fit on display line
  strncpy(txtAux,&rdsMsg[rdsMsgIndex],16);
  txtAux[16] = '\0';
  rdsMsgIndex += 4;
  if (rdsMsgIndex > 40) rdsMsgIndex = 0;
  lcd.setCursor(0,0);
  lcd.print(txtAux);
}

/**
   showRDSStation - Shows the 
*/
void showRDSStation()
{
  char txtAux[17];

  if (stationName == NULL) return;

  stationName[16] = '\0';
  strncpy(txtAux,stationName,16);
  txtAux[16] = '\0';
  lcd.setCursor(0,0);
  lcd.print(txtAux);
}

void showRDSTime()
{
  char txtAux[17];

  if (rdsTime == NULL) return;

  rdsTime[16] = '\0';
  strncpy(txtAux,rdsTime,16);
  txtAux[16] = '\0';
  lcd.setCursor(0,0);
  lcd.print(txtAux);
}


void clearRds() {
  bShow = false;
  rdsMsg = NULL;
  stationName = NULL;
  rdsTime = NULL;
  rdsMsgIndex = currentMsgType = 0;
}

void checkRDS()
{
  // check if RDS currently synchronized; the information are A, B, C and D blocks; and no errors
  if ( rx.getRdsReady() &&  rx.hasRdsInfo()) {
    rdsMsg = rx.getRdsText2A();
    stationName = rx.getRdsText0A();
    rdsTime = rx.getRdsTime();
  }
}

void showRds() {

    lcd.setCursor(2, 1);
    if (bRds)
       lcd.print(".");
    else
       lcd.print(" ");

    if ( currentMsgType == 0)
      showRDSMsg();
    else if ( currentMsgType == 1)   
      showRDSStation();
    else if ( currentMsgType == 2)  
      showRDSTime();
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
  rdsMsgIndex = currentMsgType = 0;
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
  bShow =  true;
  showStatus();
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
    showStatus();
    bShow = true;
    encoderCount = 0;
    storeTime = millis();
  }

  if (digitalRead(VOLUME_UP) == LOW) {
    rx.setVolumeUp();
    resetEepromDelay();
  }
  else if (digitalRead(VOLUME_DOWN) == LOW) {
    rx.setVolumeDown();
    resetEepromDelay();
  }
  else if (digitalRead(SWITCH_STEREO) == LOW)
    doStereo();
  else if (digitalRead(SWITCH_RDS) == LOW)
    doRds();
  else if (digitalRead(SEEK_FUNCTION) == LOW)
    doSeek();

  if ( (millis() - pollin_elapsed) > POLLING_TIME ) {
    showStatus();
    if ( bShow ) clearRds();
    pollin_elapsed = millis();
  }

  if ( (millis() - polling_rds) > POLLING_RDS) {
    if ( bRds ) {
      checkRDS();
    }
    polling_rds = millis();
  }

  if ( (millis() - timeTextType) > RDS_MSG_TYPE_TIME ) {
    // Toggles the type of message to be shown - See showRds function
    currentMsgType++; 
    if ( currentMsgType > 2) currentMsgType = 0;
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

  delay(5);
}
