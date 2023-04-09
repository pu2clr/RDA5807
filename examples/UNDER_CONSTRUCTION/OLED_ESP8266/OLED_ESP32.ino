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
#define ENCODER_PUSH_BUTTON 12     // GPIO12



// Devices class declarations
Rotary encoder = Rotary(ENCODER_PIN_A, ENCODER_PIN_B);

Adafruit_SSD1306 display = Adafruit_SSD1306(128, 32, &Wire);

RDA5807 rx;


void setup()
{
  // Encoder pins
  pinMode(ENCODER_PUSH_BUTTON, INPUT_PULLUP);
  
  pinMode(ENCODER_PIN_A, INPUT_PULLUP);
  pinMode(ENCODER_PIN_B, INPUT_PULLUP);

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

  
  delay(300);

  // Checking the EEPROM content
  if (EEPROM.read(eeprom_address) == app_id)
  {
    readAllReceiverInformation();
  } else 
    rx.setVolume(volume);
  

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
   The EEPROM.update avoid write the same data in the same memory position. It will save unnecessary recording.
*/
void saveAllReceiverInformation()
{


}

/**
 * reads the last receiver status from eeprom. 
 */
void readAllReceiverInformation()
{

}

/*
 * To store any change into the EEPROM, it is needed at least STORE_TIME  milliseconds of inactivity.
 */
void resetEepromDelay()
{

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

  // strcat(bufferDisplay, unit);
  // display.setTextSize(2);
  /*
  display.setFont(&DSEG7_Classic_Regular_16);
  display.clearDisplay();
  display.setCursor(20, 24);
  display.print(bufferDisplay);
  display.setCursor(90,15);
  display.setFont(NULL);
  display.setTextSize(1);
  display.print(unit);
  display.display();
  */

}


/**
 *   Shows the current RSSI and SNR status
 */
void showRSSI()
{
  /*
  char sMeter[10];
  sprintf(sMeter, "S:%d ", rssi);

  display.fillRect(0, 25, 128, 10, SSD1306_BLACK);  
  display.setTextSize(1);
  display.setCursor(80, 25);
  display.print(sMeter);
  if (currentMode == FM)
  {
    display.setCursor(0, 25);
    display.print((rx.getCurrentPilot()) ? "ST" : "MO");
  }

  display.display();
  */

}

void loop()
{

}
