#include <RDA5807.h>

RDA5807 rx;

void setup() {
  
    Serial.begin(9600);
   
    rx.setup(); // Stats the receiver with default valuses. Normal operation
    rx.setVolume(4);
    rx.setFrequency(9550);  // Tune on 95.5 MHz
    delay(5000);
    // Valume Test
    Serial.println("\nSet volume to 10");
    rx.setVolume(10);
    delay(3000);
    Serial.println("\nSet volume to 2");
    rx.setVolume(2);
    delay(3000);
    Serial.println("\nSeek Test");
    // SEEK function teste
    for (int i = 0; i < 6; i++ ) {
      rx.seek(RDA_SEEK_WRAP, RDA_SEEK_UP);
      Serial.println("\nReal Freq.....: ");
      Serial.println(rx.getRealFrequency());
      delay(1000);
      Serial.println("\nRSSI.....: ");
      Serial.println(rx.getRssi());
      delay(5000);
    }  
}



void loop() {

    
}
