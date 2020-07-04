/*
   Test and validation of RDA5807 on ESP32 board.
    
   ATTENTION:  
   Please, avoid using the computer connected to the mains during testing. Used just the battery of your computer. 
   This sketch was tested on ATmega328 based board. If you are not using a ATmega328, please check the pins of your board. 

  | RDA5807    | Function              |ESP LOLIN32 WEMOS (GPIO) |
  |-----------| ----------------------|-------------------------|
  | SDA/SDIO  |   SDIO                |   21 (SDA / GPIO21)     |
  | CLK/CLOCK |   SCLK                |   22 (SCL / GPIO22)     |


   By Ricardo Lima Caratti, 2020.
*/

#include <RDA5807.h>

// I2C bus pin on ESP32
#define ESP32_I2C_SDA 21
#define ESP32_I2C_SCL 22

#define MAX_DELAY_RDS 40   // 40ms - polling method

long rds_elapsed = millis();

RDA5807 rx;

void showHelp()
{
  Serial.println("Type U to increase and D to decrease the frequency");
  Serial.println("Type S or s to seek station Up or Down");
  Serial.println("Type + or - to volume Up or Down");
  Serial.println("Type 0 to show current status");
  Serial.println("Type ? to this help.");
  Serial.println("==================================================");
  delay(1000);
}

// Show current frequency
void showStatus()
{
  char aux[80];
  sprintf(aux,"\nYou are tuned on %u MHz | RSSI: %3.3u dbUv | Vol: %2.2u | %s ",rx.getFrequency(), rx.getRssi(), rx.getVolume(), (rx.isStereo()) ? "Yes" : "No" );
  Serial.print(aux);
}

void setup()
{
    Serial.begin(9600);
    while (!Serial) ;

    // The line below may be necessary to setup I2C pins on ESP32
    Wire.begin(ESP32_I2C_SDA, ESP32_I2C_SCL);
    
    rx.setup();

    rx.setVolume(6);

    delay(500);

    // Select a station with RDS service in your place
    Serial.print("\nEstacao 106.5MHz");
    rx.setFrequency(10650); // It is the frequency you want to select in MHz multiplied by 100.

    // Enables SDR
    rx.setRDS(true);

    showHelp();
    showStatus();
 
}

void loop()
{
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
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_UP);
      break;
    case 's':
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_DOWN);
      break;
    case '0':
      showStatus();
      break;
    case '?':
      showHelp();
      break;
    default:
      break;
    }
    delay(200);
    showStatus();
  } 
  delay(5);
}
