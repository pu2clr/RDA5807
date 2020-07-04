#include <RDA5807.h>

RDA5807 rx;

void setup() {
  
    Serial.begin(9600);

    
    rx.setup(); // Stats the receiver with default valuses. Normal operation
    // rx.setMute(false);
    // rx.setMono(false);
    rx.setVolume(12);
    rx.setFrequency(9550);
    delay(2000);
    rx.setVolume(2);    
    delay(20000);
    rx.setFrequency(10650);
    delay(20000);
    rx.powerDown();


    /*
    rx.powerUp();
    // rx.setRegister(0x2, 0x1);    // 0000000000000001
    rx.setRegister(0x5, 0x9084); // 1001000010000100
    rx.setRegister(0x2, 0x1);    // 0000000000000001
    rx.setRegister(0x2, 0x4001); // 100000000000001
    // setFrequency(8990)
    rx.setRegister(0x2, 0xC009); // 1100000000001001
    rx.setRegister(0x3, 0x750);  // 0000011101010000 
    rx.setRegister(0x5, 0x9084); // 1001000010000100
    */
}

void loop() {

    
}
