/*

    Test RDS functions  using Serial Monitor.
    This sketch is useful to check how the RDS work and how the local stations provide this service.
    For example: I have noticed in my location that some stations send wrong time information. So, for these stations, 
    the RDS time functions implemented by this library seem do not work fine

    To use this aplication, please select the Arduino IDE Serial Monitor. 
    Type ? to see the instructions. 


    Arduino Pro Mini and RDA5807 wire up

    | Device  RDA5807 |  Arduino Pin  |
    | --------------- | ------------  |
    | SDIO            |     A4        |
    | SCLK            |     A5        |


  ATTENTION:
   Please, avoid using the computer connected to the mains during testing. Used just the battery of your computer.
   This sketch was tested on ATmega328 based board. If you are not using a ATmega328, please check the pins of your board.

   By Ricardo Lima Caratti, 2023.
*/

#include <RDA5807.h>

#define MAX_DELAY_RDS 80  //  polling method
#define MAX_DELAY_STATUS 5000
#define MAX_DELAY_SHOW_RDS 80

#define STATION_WITH_RDS_SERVICE 8990  // Local station with good RDS service (Example: 89,90Mhz)

long rds_elapsed = millis();
long status_elapsed = millis();

uint8_t showrRdsInfo = 3;  // Default: show RDS time.

RDA5807 rx;

void setup() {

  Serial.begin(9600);
  while (!Serial)
    ;
  Serial.println(F("\nPU2CLR RDA5807 Arduino Library."));

  rx.setup();

  rx.setRDS(true);  // Turns RDS on

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
char *localTime;

void showRdsInfo(char *infoType, char *infoText) {
  char aux[120];
  if (infoText != NULL) {
    sprintf(aux, "%s => %s", infoType, infoText);
    Serial.println(aux);
  }
}

void showRDS() {
  // Checks RDS information. Gets the points of char array containing RDS information.
  if (rx.getRdsReady()) {
    if (rx.hasRdsInfo()) {
      programInfo = rx.getRdsProgramInformation();
      showRdsInfo((char *)"Progrgam Information: ", programInfo);
      stationInfo = rx.getRdsStationInformation();
      showRdsInfo((char *)"Station Information:  ", stationInfo);
      stationName = rx.getRdsStationName();
      showRdsInfo((char *)"Station Name........: ", stationName);
      utcTime = rx.getRdsTime();
      showRdsInfo((char *)"UTC Time............: ", utcTime);
      delay(MAX_DELAY_SHOW_RDS);
      localTime = rx.getRdsLocalTime();
      showRdsInfo((char *)"Local Time............: ", localTime);
    }
  }
}

void loop() {
  if ((millis() - rds_elapsed) > MAX_DELAY_RDS) {
    showRDS();
    rds_elapsed = millis();
  }

  if ((millis() - status_elapsed) > MAX_DELAY_STATUS) {
    showStatus();
    status_elapsed = millis();
  }

  if (Serial.available() > 0) {
    char key = Serial.read();
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
}
