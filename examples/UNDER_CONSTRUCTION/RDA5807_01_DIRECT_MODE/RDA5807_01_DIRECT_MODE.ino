/*
    UNDER CONSTRUCTION...
    Direct mode test

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

#define MAX_DELAY_STATUS 5000
#define YOUR_LOCAL_STATION 10390  // Local station with good RDS service (Example: 89,90Mhz) 

long status_elapsed = millis();

RDA5807 rx;

void setup()
{
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println(F("\nPU2CLR RDA5807 Arduino Library."));

  rx.setup();

  rx.setFrequencyMode(1);


  rx.setVolume(6);

  delay(500);

  // Select a station with RDS service in your place
  Serial.print(F("\nTuning at the FM local station via Direct Frequency. See YOUR_LOCAL_STATION constant and chenge it if necessary."));
  rx.setDirectFrequency(YOUR_LOCAL_STATION); 
}


// Show current frequency
void showStatus()
{
  char aux[120];
  sprintf(aux, "\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | Stereo: %s | Band Status: %d\n", rx.getRealFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No", rx.getBand3Status());
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
}
