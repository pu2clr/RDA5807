/*
  
The best way to customize the PU2CLR RDA5807 Arduino Library for your needs is extending the 
current version of the library by using C++ OOP approach. 

This example shows how to create your own RDA5807 class inheriting the functions already implemented in  original PU2CLR RDA5807 class.

When should you extend the RDA5807 class?  

You have at least two great reasons to extend the RDA5807 class: 

1. you need a function that has not yet been implemented in RDA5807;
2. you want to change the behavior of an already implemented function.

If you use this approach, all you have to do is download the current version of PU2CLR RDA5807 Arduino Library. 
Instead of using the PU2CLR RDA5807 Arduino Library class directly, you can use your own class that extends the original class. 
This way, you always have the current version of the library customized for your needs. 
So, no extra work will be needed when you update the PU2CLR RDA5807 Arduino Library. 
In other words, your custom code will always be synchronized with the PU2CLR RDA5807 Arduino Library code.

By Ricardo Lima Caratti, 2023.

*/
#include "MyRDA5807.h"

MyRDA5807 radio;  // the instance of your custom class based on RDA5807 class

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Customizing RDA5807 class example.");
  radio.setup();
  radio.setFrequency(10390);
  Serial.println(radio.getSoftBlendEnable());
  Serial.println(radio.getDeviceInfo());
  radio.setBand(3);
  radio.setBand3_50_65_Mode(0);
}

void loop() {

  radio.setFrequency(6500);
  Serial.println(radio.getFrequency());
  delay(2000);
  radio.setFrequencyUp();  // Go to 50 MHz
  Serial.println(radio.getFrequency());
  delay(2000);
  radio.setFrequencyDown();  // Go to 65 MHz
  Serial.println(radio.getFrequency());
  delay(2000);
}
