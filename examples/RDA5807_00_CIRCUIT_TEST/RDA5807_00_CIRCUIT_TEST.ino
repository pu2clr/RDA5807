/*
  This sketch intends to check the circuit and functions implemented in the library
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

  if (i2cStatus == -1)
    showMsgHalt(F("\nError while try to access the device.\n")); 
  else if (i2cStatus == 0) 
    showMsgHalt(F("\nNo device was detected. Please, check your circuit!"));
  else { 
    for (int i = 0; i < i2cStatus ; i++) {
      sprintf(bufferAux,"Found I2C adress: %X (HEX)\n", i2cAdd[i]);
      Serial.print(bufferAux);
    }
  }     

  showSeparator();

  rx.setup();  // 32.768kHz passive crystal
  // rx.setup(CLOCK_32K, OSCILLATOR_TYPE_ACTIVE); // 32.768kHz Active Crystal
  // rx.setup(CLOCK_12M, OSCILLATOR_TYPE_PASSIVE);
  // rx.setup(CLOCK_38_4M, OSCILLATOR_TYPE_PASSIVE);


  delay(200);
  Serial.println(F("Some information just after the receiver starts - rx.setup();"));
  showReceiverInfo();
  delay(5000);


  showSeparator();
  Serial.println(F("Setting band to 76–108 MHz (world wide)"));
  rx.setBand(2); // Sets band: 76–108 MHz (world wide)  
  delay(3000);

  showSeparator();
  Serial.print(F("\nTrying to tune a strong station\n"));
  rx.setVolume(8); 
  rx.setFrequency(STRONG_STATION); 
  delay(200);
  sprintf(bufferAux,"\nCurrent Channel: %d, Real Frequency: %d ( / 100), RSSI: %d\n", rx.getRealChannel(), rx.getRealFrequency(), rx.getRssi());
  Serial.print(bufferAux);
  delay(5000);
  showSeparator();
  
  // Mute test
  Serial.print(F("\nChecking mute function. After 3s, the receiver will mute during 3s"));
  delay(3000);
  rx.setMute(true);
  delay(3000);
  rx.setMute(false);
  Serial.print(F("\nMute test has finished."));
  showSeparator();

  Serial.print(F("\nTrying to tune a weak station\n"));
  rx.setFrequency(WAEK_STATION); // Please, change it to another local FM station
  delay(200);
  sprintf(bufferAux,"\nCurrent Channel: %d, Real Frequency: %d ( / 100), RSSI: %d\n", rx.getRealChannel(), rx.getRealFrequency(), rx.getRssi());
  Serial.print(bufferAux);
  delay(5000);
  showSeparator();
  Serial.print(F("\n\nChecking LNA setup.\n"));
  for (uint8_t i = 0; i < 4; i++) {
    sprintf(bufferAux,"\nSetting LNA PORT SETUP to %d\n",i); 
    Serial.print(bufferAux);
    rx.setLnaPortSel(i);
    showReceiverInfo();
    delay(5000);
  }
  Serial.print(F("\nBacking  LNA PORT SETUP to 2 (default value)\n"));
  rx.setLnaPortSel(2);
  showReceiverInfo();
  delay(5000);

  for (uint8_t i = 0; i < 4; i++) {
    sprintf(bufferAux,"\nSetting LNA IC Sel to %d\n",i); 
    Serial.print(bufferAux);
    rx.setLnaIcSel(i);
    showReceiverInfo();
    delay(5000);
  }
  Serial.print(F("\nSetting LNA IC Sel to default value (0)\n"));
  rx.setLnaIcSel(0); // Setting to default (0)
  delay(3000);

  showSeparator();
  showReceiverInfo();
  Serial.print(F("\n\nSetting New Demodulate Method. It can improve the receive sensitivity about 1dB."));
  rx.setNewDemodulateMethod(true);
  showReceiverInfo();
  delay(5000);
  showSeparator();
  Serial.print(F("\n\nDisabling New Demodulate and change to teh STRONG station\n"));
  rx.setNewDemodulateMethod(false);
  rx.setFrequency(STRONG_STATION);
  delay(200);
  showReceiverInfo();
  Serial.print(F("\n\nSetting New Demodulate Method again."));
  rx.setNewDemodulateMethod(true);
  showReceiverInfo();
  delay(5000);
  showSeparator();

  Serial.print(F("\n\nChecking audio output setup.\n"));

  Serial.print(F("Setting volume to 0\n"));
  rx.setVolume(0); 
  showReceiverInfo();
  delay(3000);
  Serial.print(F("Setting volume to 6, mute audio and setting Softmute to true\n"));
  rx.setVolume(6);
  rx.setMute(true);
  rx.setSoftmute(true);
  showReceiverInfo();
  delay(3000);
  Serial.print(F("Setting mute false\n"));
  rx.setMute(false);
  delay(3000);
  Serial.print(F("Setting audio output impedance to high\n"));
  rx.setAudioOutputHighImpedance(true);
  showReceiverInfo();
  delay(3000);

  showSeparator();
  Serial.print(F("\n\nResetting the system via setup command in 5s\n"));
  delay(5000);
  rx.setup();
  Serial.print(F("\nSystema started again!"));
  showReceiverInfo();
  delay(3000);
  
  showSeparator();
  Serial.print(F("\n\nResetting the system via softReset and starting with setup in 5s\n"));
  delay(5000);
  rx.softReset();
  Serial.print(F("\nReseted!\n"));
  showReceiverInfo();
  rx.setup();
  Serial.print("Started");
  showReceiverInfo();
  Serial.print(F("\nSystema reseted!"));
  showReceiverInfo();
  delay(3000);

  showSeparator();
  Serial.print(F("Going to the Lowest frequency of the band"));
  rx.setFrequencyToBeginBand();
  delay(200);
  showReceiverInfo();
  delay(3000);
  Serial.print(F("Going to the Hihgest frequency of the band"));
  rx.setFrequencyToEndBand();
  delay(200);
  showReceiverInfo();
  delay(3000);

  showSeparator();

  Serial.print(F("\nTrying to seek stations with LNA PORT 3 and New Demodulate Enebled\n"));
  rx.setLnaPortSel(3);
  rx.setNewDemodulateMethod(true);
  rx.setFrequencyToBeginBand();
  rx.setVolume(8);
  delay(3000);
  showSeparator();

  Serial.println(F("Setting band to 87–108 MHz (US/Europe)"));
  rx.setBand(0); 
  delay(3000);

  // Seek test
  Serial.print(F("\n\nSeeking stations"));
  for (int i = 0; i < 15; i++ ) { 
    rx.seek(RDA_SEEK_STOP,RDA_SEEK_UP, showFrequency);
    delay(200);
    showReceiverInfo();
    delay(4000);
  }
  
  showSeparator();

  Serial.print(F("\nTrying to seek stations with LNA PORT 2 and New Demodulate Disabled\n"));
  rx.setLnaPortSel(2);
  rx.setNewDemodulateMethod(false);
  rx.setFrequencyToBeginBand();
  rx.setVolume(8);
  delay(3000);
  showSeparator();
  // Seek test
  Serial.print(F("\n\nSeeking stations"));
  for (int i = 0; i < 15; i++ ) { 
    rx.seek(RDA_SEEK_STOP,RDA_SEEK_UP, showFrequency);
    delay(200);
    showReceiverInfo();
    delay(4000);
  }

  Serial.println(F("\nTest finished!\n"));
}

void showFrequency() {
  char strFreq[10];
  Serial.print(rx.formatFrequency(rx.getFrequency(), strFreq, ','));
  Serial.print(" ");
  Serial.flush();
}

void showReceiverInfo() {
  char strFreq[10];
  int rssi = rx.getRssi();
  sprintf(bufferAux,"\nID: %x(HEX) | Freq: %sMHz | RSSI: %d | Band Space: %d | Volume: %d | Muted: %d | HighZ: %d | Soft Mute: %d | Lna Port: %d | Lna IC Sel: %d \n", rx.getDeviceId(), rx.formatFrequency(rx.getRealFrequency(), strFreq, ','), rssi, rx.getSpace(), rx.getVolume(), rx.isMuted(), rx.isAudioOutputHighImpedance(), rx.isSoftmuted(), rx.getLnaPortSel(), rx.getLnaIcSel() );
  Serial.print(bufferAux);
}

void showSeparator() {
  Serial.print("\n------------------------------------\n\n");
}

void showMsgHalt(String msg) {
  Serial.print(F("\n********************\n"));
  Serial.print(msg);
  Serial.print(F("\n********************\n")); 
  while(1);
}


void loop() {

}

