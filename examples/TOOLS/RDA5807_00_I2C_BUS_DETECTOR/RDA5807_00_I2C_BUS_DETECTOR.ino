/*
  This sketch intends to check the circuit and functions implemented in the library
  1) check the I2C communication bus 
  2) try to tune some stations
 
  ATTENTION:  
  Please, avoid using the computer connected to the mains during testing. Used just the battery of your computer. 
  This sketch was tested on ATmega328 based board. If you are not using a ATmega328, please check the pins of your board.

  The main advantages of using this sketch are: 
    1) It is a easy way to check if your circuit is working;
    2) You do not need to connect any display device to make your radio work;
    3) You do not need connect any push buttons or encoders to change volume and frequency;
    4) The Arduino IDE is all you need to check your circuit.  
   
  By Ricardo Lima Caratti, 2020-2023
*/

#include <RDA5807.h>


#define WAEK_STATION    10270   // Please, change to your weakest station in your location 
#define STRONG_STATION  10650   // Please, change to your strongest station in your location

RDA5807 rx; 
char bufferAux[160];

void setup() {

  Serial.begin(9600);
  while(!Serial);

  delay(500);
  
  uint8_t i2cAdd[5];
  int i2cStatus;

  i2cStatus = rx.checkI2C(i2cAdd); 

  if (i2cStatus == -1) {
    Serial.println(F("\nError while try to access the device.\n")); 
    while(1);
  }
  else if (i2cStatus == 0) {
    Serial.println(F("\nNo device was detected. Please, check your circuit!"));
    while(1);
  }
  else { 
    Serial.println(F("\nDevice found: \n"));
    for (int i = 0; i < i2cStatus ; i++) {
      sprintf(bufferAux,"Found I2C adress: %X (HEX)\n", i2cAdd[i]);
      Serial.print(bufferAux);
    }
  }     
  rx.setup();  // 32.768kHz passive crystal
  rx.setFrequency(10390); // Tunes at 103,9 MHz - Change it to your local FM Station. 
  Serial.print(F("\nHave a nice project with the device ID: "));
  Serial.println(rx.getDeviceId());
}


void loop() {

}

