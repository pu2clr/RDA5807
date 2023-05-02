/*
   
   Test of RDA5807 Tune, Volume, Seek and RDS features.

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

#define MAX_DELAY_RDS 80   // 40ms - polling method
#define MAX_DELAY_STATUS   2000 

long rds_elapsed = millis();
long status_elapsed = millis();


RDA5807 rx;

void setup()
{

  Serial.begin(9600);
  while (!Serial) ;
  Serial.println(F("\nPU2CLR RDA5807 Arduino Library."));


  rx.setup();

  rx.setRDS(true); // Turns RDS on

  rx.setVolume(6);

  delay(500);

  // Select a station with RDS service in your place
  Serial.print(F("\nTuning 106.5MHz"));
  rx.setFrequency(10650); // It is the frequency you want to select in MHz multiplied by 100.

  // RDS setup
  rx.setRDS(true);
  rx.setRdsFifo(true);

  rx.setGpio(3,1);  //  Mono/Stereo indicator. When Stereo, the GPIO03 (pin 15 of the RDA5807FP) becomes high 
  rx.setAFC(true);  // Sets Automatic Frequency Control
  
  showHelp();

}

void showHelp()
{
  Serial.println(F("Type U to increase and D to decrease the frequency"));
  Serial.println(F("Type S or s to seek station Up or Down"));
  Serial.println(F("Type + or - to volume Up or Down"));
  Serial.println(F("Type 0 to show current status"));
  Serial.println(F("Type ? to this help."));
  Serial.println(F("=================================================="));
  delay(5000);
}

// Show current frequency
void showStatus()
{
  char aux[80];
  sprintf(aux, "\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | Stereo: %s\n", rx.getFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No" );
  Serial.print(aux);
  status_elapsed = millis();
}




/*********************************************************
   RDS
 *********************************************************/
char *rdsMsg;
char *stationName;
char *rdsTime;

void checkRDS()
{
  if (rx.getRdsReady())
  {
     if ( (rdsMsg = rx.getRdsText2A()) != NULL) 
        Serial.println(rdsMsg);
     else if ( (stationName = rx.getRdsText0A()) != NULL)
        Serial.println(stationName);
     else if ( (rdsTime = rx.getRdsTime()) != NULL )
        Serial.println(rdsTime);
  }
}

void showRds()
{
  if ( rx.getRdsReady() )
    checkRDS();
}


void loop()
{
  if ((millis() - rds_elapsed) > MAX_DELAY_RDS ) {
    if ( rx.getRdsReady() )  checkRDS();
    rds_elapsed = millis();
  }

  if ((millis() - status_elapsed) > MAX_DELAY_STATUS ) {
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
      case '?':
        showHelp();
        break;
      default:
        break;
    }
    showStatus();
  }
}
