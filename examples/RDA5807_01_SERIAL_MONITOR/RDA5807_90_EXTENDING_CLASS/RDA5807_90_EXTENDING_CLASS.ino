/*
The best way to customize the PU2CLR RDA5807 Arduino Library for your needs is extending the 
current version of the library by using C++ OOP approach. 

If you use that approach, all you have to do is download the current version of PU2CLR RDA5807 Arduino Library. 
Instead of using the PU2CLR RDA5807 Arduino Library class directly, you can use your own class that extends the original class. 
This way, you always have the current version of the library customized for your needs. 
So, no extra work will be needed when you update the PU2CLR RDA5807 Arduino Library. 
In other words, your custom code will always be synchronized with the PU2CLR RDA5807 Arduino Library code.

When should you extend the RDA5807 class?  

You have at least two great reasons to extend the RDA5807 class: 

1. you need a function that has not yet been implemented in RDA5807;
2. you want to change the behavior of an already implemented function.



By Ricardo Lima Caratti, 2023.

*/
#include <RDA5807.h>
class MyCustomRDA5807 : public RDA5807 {  // extending the original class RDA5807

private: 
  // Implements some specifics members and methods to the new class if necessary
  uint16_t up_limit, down_limit;

  void getBandLimits() {
    if (this->getBand3Status() == 0) {
      up_limit = 6500;
      down_limit = 5000;
    } else {
      up_limit = this->endBand[this->currentFMBand];
      down_limit = this->startBand[this->currentFMBand];
    }
  }

public:
  // Implements some new members functions to the new class
  int getSoftBlendEnable() {  // A RDA5807 command that PU2CLR RDA5807 Arduino Library does not implement
    rda_reg07 tmp;
    tmp.raw = this->getDirectRegister(0x07).raw;
    return tmp.refined.SOFTBLEND_EN;
  }

  uint16_t getDeviceInfo() {  // another RDA5807 command that PU2CLR RDA5807 Arduino Library does not implement
      rda_reg00 tmp;
      tmp.raw = this->getDirectRegister(0x00).raw;
      return tmp.refined.HIGH_CHIP_ID;
  }

  // Overwriting parent method setFrequencyUp - Chenging the behavior of the setFrequencyUp function
  void setFrequencyUp() {
    getBandLimits();
    if (this->currentFrequency < up_limit)
      this->currentFrequency += (this->fmSpace[currentFMSpace]);
    else
      this->currentFrequency = down_limit;

    setFrequency(this->currentFrequency);
  }

  // Overwriting parent method setFrequencyDown - Chenging the behavior of the setFrequencyDown function
  void setFrequencyDown() {
    getBandLimits();
    if (this->currentFrequency > down_limit)
      this->currentFrequency -= (this->fmSpace[currentFMSpace]);
    else
      this->currentFrequency = up_limit;

    setFrequency(this->currentFrequency);
  }
};

MyCustomRDA5807 radio;  // the instance of your custom class based on RDA5807 class

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
