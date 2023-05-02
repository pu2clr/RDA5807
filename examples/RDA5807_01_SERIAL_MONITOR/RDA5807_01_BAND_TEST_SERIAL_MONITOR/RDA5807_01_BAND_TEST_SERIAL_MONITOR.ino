/*
    This sketch is useful to check the band setup of the RDA5807 in your location

    | Value | Description                 | 
    | ----- | --------------------------- | 
    | 00    | 87–108 MHz (US/Europe)      |
    | 01    | 76–91 MHz (Japan)           | 
    | 10    | 76–108 MHz (world wide)     | 
    | 11    | 65 –76 MHz (East Europe) or 50-65MHz (see bit 9 of gegister 0x06) |

    To use this aplication, please select the Arduino IDE Serial Monitor. 
    Type ? to see the instructions. 

    Arduino Pro Mini and RDA5807 wire up

    | Device  RDA5807 |  Arduino Pin  |
    | --------------- | ------------  |
    | RESET           |     14/A0     |
    | SDIO            |     A4        |
    | SCLK            |     A5        |


  ATTENTION:
   Please, avoid using the computer connected to the mains during testing. Used just the battery of your computer.
   This sketch was tested on ATmega328 based board. If you are not using a ATmega328, please check the pins of your board.

   By Ricardo Lima Caratti, 2023.
*/

#include <RDA5807.h>

#define RESET_PIN 14 // On Arduino Atmega328 based board, this pin is labeled as A0 (14 means digital pin instead analog)

#define MAX_DELAY_STATUS 5000

#define YOUR_LOCAL_STATION 8990  // Local station with good RDS service (Example: 89,90Mhz) 

long rds_elapsed = millis();
long status_elapsed = millis();

uint8_t showrRdsInfo = 3; // Default: show RDS time.

char *bandTable[] = { (char *) "0 - 87–108 MHz (US/Europe)", 
                      (char *) "1 - 76–91 MHz (Japan)", 
                      (char *) "2 - 76–108 MHz (world wide)",
                      (char *) "3 - 65 –76 MHz (East Europe)",
                      (char *) "3 - 50 - 65 MHz"};

RDA5807 rx;

void setup()
{

  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println(F("\nPU2CLR RDA5807 Arduino Library."));

  rx.setup();

  rx.setVolume(6);

  delay(500);

  // Select a station with RDS service in your place
  Serial.print(F("\nTuning at the FM local station. See YOUR_LOCAL_STATION constant and chenge it if necessary."));
  rx.setFrequency(YOUR_LOCAL_STATION); 

  rx.setLnaPortSel(3); // Trying improve sensitivity.
  rx.setAFC(true);    // Sets Automatic Frequency Control

  showHelp();
}

void showHelp()
{
  Serial.println(F("Type U to increase and D to decrease the frequency"));
  Serial.println(F("     S or s to seek station Up or Down"));
  Serial.println(F("     + or - to volume Up or Down"));
  Serial.println(F("     0 to use 87–108 MHz (US/Europe)"));
  Serial.println(F("     1 to use 76–91 MHz (Japan)"));
  Serial.println(F("     2 to use 76–108 MHz (world wide)"));
  Serial.println(F("     3 to use 65–76 MHz (East Europe) or 50-65MHz (see bit 9 of gegister 0x06)"));
  Serial.println(F("     4 to use 50–65 MHz"));
  Serial.println(F("     ? to this help."));
  Serial.println(F("=================================================="));
  delay(5000);
}

// Show current frequency
void showStatus()
{
  char aux[120];
  sprintf(aux,"\nCurrent band is:  %s", bandTable[rx.getBand()] );
  Serial.print(aux);
  sprintf(aux, "\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | Stereo: %s | Band Status: %d\n", rx.getFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No", rx.getBand3Status());
  Serial.print(aux);
  status_elapsed = millis();
}

void loop()
{
  if ((millis() - status_elapsed) > MAX_DELAY_STATUS)
  {
    showStatus();
    status_elapsed = millis();
  }

  if (Serial.available() > 0)
  {
    char key = Serial.read();
    switch (key)
    {
    case '+':
      rx.setVolumeUp();
      break;
    case '-':
      rx.setVolumeDown();
      break;
    case 'U':
    case 'u':
      rx.setFrequencyUp();
      break;
    case 'D':
    case 'd':
      rx.setFrequencyDown();
      break;
    case 'S':
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_UP, showStatus);
      break;
    case 's':
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_DOWN, showStatus);
      break;
    case '0':  
    case '1':
    case '2':
    case '3':
      rx.setBand( key - 48 );
      rx.setBand3_50_65_Mode(1); // Band 3 will work from  65 to 76 MHz;
      rx.setFrequencyToBeginBand();
      Serial.print(F("\n**** Switching to band: "));
      Serial.print(rx.getBand());
      break;
    case '4': 
      // ATTENTION: The functions setFrequencyToBeginBand and setFrequencyToEnBand do not work for 50-65MHz setup. You have to control it by yourself.
      //            Also, you must control the band limits from 50 to 65 MHz. The setFrequencyUp and setFrequencyDown do not work properly. 
      rx.setBand(3);
      rx.setBand3_50_65_Mode(0); // Band 3 will work from  50 to 65 MHz;
      rx.setFrequency(5500); // 55 Mhz; rx.setFrequencyDown and rx.setFrequencyUP, rx.setFrequencyToBeginBand() and rx.setFrequencyToEndBand() do not work properly for this setup;
      Serial.print(F("\n**** Switching to band: 3 from 50 to 65 MHz) "));
      break;      
    case '?':
      showHelp();
      break;
    default:
      break;
    }
    showStatus();
  }
}
