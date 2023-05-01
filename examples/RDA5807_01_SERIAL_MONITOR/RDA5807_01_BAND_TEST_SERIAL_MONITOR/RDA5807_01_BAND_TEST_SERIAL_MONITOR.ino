/*

    Test RDS functions  using Serial Monitor.
    This sketch is useful to check the band setup of the RDA5807 in your location

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

#define MAX_DELAY_RDS 80 //  polling method
#define MAX_DELAY_STATUS 5000
#define MAX_DELAY_SHOW_RDS 250

#define STATION_WITH_RDS_SERVICE 8990  // Local station with good RDS service (Example: 89,90Mhz) 

long rds_elapsed = millis();
long status_elapsed = millis();

uint8_t showrRdsInfo = 3; // Default: show RDS time.

RDA5807 rx;

void setup()
{

  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println("\nPU2CLR RDA5807 Arduino Library.");

  rx.setup();

  rx.setRDS(true); // Turns RDS on

  rx.setVolume(6);

  delay(500);

  // Select a station with RDS service in your place
  Serial.print("\nTuning at the FM local station with good RDS service (see: STATION_WITH_RDS_SERVICE");
  rx.setFrequency(STATION_WITH_RDS_SERVICE); 

  // RDS setup
  rx.setRDS(true);
  rx.setRdsFifo(true);
  rx.setLnaPortSel(3); // Trying improve sensitivity.
  rx.setAFC(true);    // Sets Automatic Frequency Control

  showHelp();
}

void showHelp()
{
  Serial.println("Type U to increase and D to decrease the frequency");
  Serial.println("     S or s to seek station Up or Down");
  Serial.println("     + or - to volume Up or Down");
  Serial.println("     0 to use 87–108 MHz (US/Europe)");
  Serial.println("     1 to use 76–91 MHz (Japan)");
  Serial.println("     2 to use 76–108 MHz (world wide)");
  Serial.println("     3 to use  65 –76 MHz (East Europe) or 50-65MHz (see bit 9 of gegister 0x06)");
  Serial.println("     ? to this help.");
  Serial.println("==================================================");
  delay(5000);
}

// Show current frequency
void showStatus()
{
  char aux[80];
  sprintf(aux,"\nCurrent band is:  %d", rx.getBand());
  Serial.print(aux);
  sprintf(aux, "\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | Stereo: %s\n", rx.getFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No");
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
      Serial.print("\n**** Switching to band: ");
      Serial.print(key - 48);
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
