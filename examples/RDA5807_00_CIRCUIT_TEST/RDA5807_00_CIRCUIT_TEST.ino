/*
  This sketch intend to check the circuit and functions implemented in the library
  1) check the I2C communication bus 
  2) try to tune some stations
  3) check some functions. Exemples: audio volume, mute, audio output inpedance, tune functions, seek, LNA etc. (you can add more test to check more functions)
 
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

RDA5807 rx; 
char buffer[255];

void setup() {

  Serial.begin(9600);
  while(!Serial);

  delay(500);
  
  uint8_t i2cAdd[5];
  int i2cStatus;

  i2cStatus = rx.checkI2C(i2cAdd); 

  if (i2cStatus == -1)
    showMsgHalt("\nError while try to access the device.\n"); 
  else if (i2cStatus == 0) 
    showMsgHalt("\nNo device was detected. Please, check your circuit!");
  else { 
    for (int i = 0; i < i2cStatus ; i++) {
      sprintf(buffer,"Found I2C adress: %X (HEX)\n", i2cAdd[i]);
      Serial.print(buffer);
    }
  }     

  showSeparator();

  rx.setup();

  delay(200);
  Serial.println("Some information just after the receiver starts - rx.setup();");
  showReceiverInfo();
  delay(5000);

  showSeparator();
  Serial.print("\nTrying station at 106.5MHz\n");
  rx.setVolume(8); 
  rx.setFrequency(10650); // Please, change it to your local FM station
  sprintf(buffer,"\nCurrent Channel: %d, Real Frequency: %d, RSSI: %d\n", rx.getRealChannel(), rx.getRealFrequency(), rx.getRssi());
  Serial.print(buffer);
  delay(5000);
  showSeparator();
  
  // Mute test
  Serial.print("\nChecking mute function. After 3s, the receiver will mute during 3s");
  delay(3000);
  rx.setMute(true);
  delay(3000);
  rx.setMute(false);
  Serial.print("\nMute test has finished.");
  showSeparator();

  Serial.print("\nTrying station at 9250MHz\n");
  rx.setFrequency(9250); // Please, change it to another local FM station
  sprintf(buffer,"\nCurrent Channel: %d, Real Frequency: %d, RSSI: %d\n", rx.getRealChannel(), rx.getRealFrequency(), rx.getRssi());
  Serial.print(buffer);
  delay(5000);
  showSeparator();
  Serial.print("\n\nChecking LNA setup.\n");
  for (uint8_t i = 0; i < 4; i++) {
    sprintf(buffer,"\nSetting LNA PORT SETUP to %d\n",i); 
    Serial.print(buffer);
    rx.setLnaPortSel(i);
    showReceiverInfo();
    delay(5000);
  }
  Serial.print("\nBacking  LNA PORT SETUP to 2 (default value)\n");
  rx.setLnaPortSel(2);
  showReceiverInfo();
  delay(5000);

  for (uint8_t i = 0; i < 4; i++) {
    sprintf(buffer,"\nSetting LNA IC Sel to %d\n",i); 
    Serial.print(buffer);
    rx.setLnaIcSel(i);
    showReceiverInfo();
    delay(5000);
  }
  Serial.print("\nSetting LNA IC Sel to default value (0)\n");
  rx.setLnaIcSel(0); // Setting to default (0)
  showSeparator();

  Serial.print("\n\nChecking audio output setup.\n");

  Serial.print("Setting volume to 0\n");
  rx.setVolume(0); 
  showReceiverInfo();
  delay(3000);
  Serial.print("Setting volume to 6, mute audio and setting Softmute to true\n");
  rx.setVolume(6);
  rx.setMute(true);
  rx.setSoftmute(true);
  showReceiverInfo();
  delay(3000);
  Serial.print("Setting mute false\n");
  rx.setMute(false);
  delay(3000);
  Serial.print("Setting audio output impedance to high\n");
  rx.setAudioOutputHighImpedance(true);
  showReceiverInfo();
  delay(3000);

  showSeparator();
  Serial.print("\n\nResetting the system in 5s\n");
  delay(5000);
  rx.setup();
  Serial.print("\nSystema started again!");
  showReceiverInfo();
  delay(3000);
  Serial.print("\nTrying to seek stations\n");
  rx.setFrequency(8700);
  rx.setVolume(8);
  showSeparator();
  // Seek test
  Serial.print("\n\nSeeking stations");
  for (int i = 0; i < 15; i++ ) { 
    rx.seek(1,1);
    Serial.print("\nReal Frequency.: ");
    Serial.println(rx.getRealFrequency());
    showReceiverInfo();
    delay(5000);
  }
  
  Serial.println("\nTest finished!\n");

}

void showReceiverInfo() {
  sprintf(buffer,"\nID: %x, Freq: %u, RSSI: %d, Band Space: %d, Volume: %d, Muted: %d, HighZ: %d, Soft Mute: %d, Lna Port: %d, Lna IC Sel: %d \n", rx.getDeviceId(), rx.getRealFrequency(), rx.getRssi(), rx.getSpace(), rx.getVolume(), rx.isMuted(), rx.isAudioOutputHighImpedance(), rx.isSoftmuted(), rx.getLnaPortSel(), rx.getLnaIcSel() );
  Serial.print(buffer);
}

void showSeparator() {
  Serial.print("\n------------------------------------\n\n");
}

void showMsgHalt(String msg) {
  Serial.print("\n********************\n");
  Serial.print(msg);
  Serial.print("\n********************\n"); 
  while(1);
}


void loop() {

}

