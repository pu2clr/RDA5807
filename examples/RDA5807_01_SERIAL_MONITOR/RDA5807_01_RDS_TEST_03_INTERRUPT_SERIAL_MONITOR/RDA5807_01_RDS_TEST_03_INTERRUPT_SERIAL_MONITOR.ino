/*

    Test RDS functions  using Serial Monitor.
    This sketch is useful to check how the RDS and interrupt setup. 

    To use this aplication, please select the Arduino IDE Serial Monitor. 
    Type ? to see the instructions. 


    Arduino Pro Mini and RDA5807 wire up

    | Device  RDA5807 |  Arduino Pin  | Description       |
    | --------------- | ------------  | ----------------- | 
    | GPIO2           |      2        | Will tell to the system when a RDS action occurs | 
    | SDIO            |     A4        | I2C Data communication  |
    | SCLK            |     A5        | I2C Clock communication |


  ATTENTION:
   Please, avoid using the computer connected to the mains during testing. Used just the battery of your computer.
   This sketch was tested on ATmega328 based board. If you are not using a ATmega328, please check the pins of your board.

   By Ricardo Lima Caratti, 2023.
*/

#include <RDA5807.h>

#define  MAX_DELAY_STATUS 30000        // Defined time to update the receiver status information 
#define  RDS_ACTION_INTERRUPT 2
#define  STATION_WITH_RDS_SERVICE 9390  // Local station with good RDS service (89,90Mhz)
#define  RDS_DELAY 1000


long rds_elapsed = millis();
long status_elapsed = millis();
uint8_t showrRdsInfo = 3;  // Default: show RDS time.

volatile int rdsCount = 0; // 


RDA5807 rx;

void setup() {
  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println(F("\nPU2CLR RDA5807 Arduino Library."));
  Serial.println(F("\nRDA5807FP Device and RDS with Interrupt control via GPIO2"));

  attachInterrupt(digitalPinToInterrupt(RDS_ACTION_INTERRUPT), rdsAction, RISING);

  rx.setup();

  rx.setLedStereoIndicator(true); // Out of the topic: Same: rx.setGpio(3, 1); // Just checking the GPIO3 - LED Stereo indicator setup 
  rx.setInterruptMode(1); // Sets interrupt on GPIO2 to deal with RDS.
  
  rx.setRDS(true);  // Turns RDS on
  rx.setRdsFifo(true);

  rx.setVolume(6);
  delay(500);
  // Select a station with RDS service in your place
  Serial.print(F("\nTuning at the FM local station with good RDS service (see: STATION_WITH_RDS_SERVICE"));
  rx.setFrequency(STATION_WITH_RDS_SERVICE);
  // RDS setup
  rx.setRDS(true);
  rx.setRdsFifo(true);
  rx.setLnaPortSel(3);  // Trying improve sensitivity.
  rx.setAFC(true);      // Sets Automatic Frequency Control
  showHelp();
}


// Tells to the system that RDA5807FP has RDS information 
void rdsAction() {
  rdsCount++;
} 


void showHelp() {
  Serial.println(F("Type U to increase and D to decrease the frequency"));
  Serial.println(F("     S or s to seek station Up or Down"));
  Serial.println(F("     + or - to volume Up or Down"));
  Serial.println(F("     ? to this help."));
  Serial.println(F("=================================================="));
  delay(5000);
}

// Show current frequency
void showStatus() {
  char aux[80];
  sprintf(aux, "\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | Stereo: %s\n", rx.getFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No");
  Serial.print(aux);
  status_elapsed = millis();
}

/*********************************************************
   RDS
 *********************************************************/
char *programInfo;
char *stationName;
char *stationInfo;
char *utcTime;

void showRdsInfo(char *infoType, char *infoText) {
  char aux[120];
  if ( infoText != NULL ) {
    sprintf(aux,"%s => %s", infoType, infoText);
    Serial.println(aux);
  }
}

void showRDS() {
  if (rx.getRdsAllData(&stationName, &stationInfo, &programInfo, &utcTime)) {
    programInfo = rx.getRdsProgramInformation();
    showRdsInfo((char *) "Progrgam Information: ", programInfo);
    stationInfo = rx.getRdsStationInformation();
    showRdsInfo((char *) "Station Information:  ", stationInfo);
    stationName = rx.getRdsStationName();
    showRdsInfo((char *) "Station Name........: ", stationName);
    utcTime = rx.getRdsTime();
    showRdsInfo((char *) "UTC Time............: ", utcTime);
  }
}

void loop() {

  // checks for any RDS action

  if ( rdsCount > 1 && (millis() - rds_elapsed) > RDS_DELAY ) {
      showRDS();      // Consumes all RDS information
      rds_elapsed = millis();
      rdsCount = 0;
  }

  if ((millis() - status_elapsed) > MAX_DELAY_STATUS) {
    showStatus();
    status_elapsed = millis();
  }

  if (Serial.available() > 0) {
    char key = Serial.read();
    rx.clearRdsBuffer();
    switch (key) {
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
  delay(5);
}
