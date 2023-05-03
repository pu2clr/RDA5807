/*
The best way to customize the PU2CLR RDA5807 Arduino Library for your needs is extending the 
current version of the library by using C++ OOP approach. 

If you use that approach, all you have to do is download the current version of PU2CLR RDA5807 Arduino Library. 
Instead of using the PU2CLR RDA5807 Arduino Library class directly, you can use your own class that extends the original class. 
This way, you always have the current version of the library customized for your needs. 
So, no extra work will be needed when you update the PU2CLR RDA5807 Arduino Library. 
In other words, your custom code will always be synchronized with the PU2CLR RDA5807 Arduino Library code.

By Ricardo Lima Caratti, 2023.

*/
#include <RDA5807.h>
class MyCustomRDA5807 : public RDA5807 {  // extending the original class RDA5807
public:
  // New functions / methods
  int methodA() {  // some RDA5807 command that PU2CLR RDA5807 Arduino Library does not implement
    return 0;
  }

  int methodB() {  // another RDA5807 command that PU2CLR RDA5807 Arduino Library does not implement
    return 1;
  }

  // Overwriting parent method setFrequencyUp
  void setFrequencyUP() {
    uint16_t up_limit, down_limit;
    uint8_t b3 = this->getBand3Status();
    if (b3 == 0) {
      up_limit = 6500;
      down_limit = 5000;
    } else {
      up_limit = this->endBand[this->currentFMBand];
      down_limit = this->startBand[this->currentFMBand];
    }
    if (this->currentFrequency < up_limit)
      this->currentFrequency += (this->fmSpace[currentFMSpace]);
    else
      this->currentFrequency = down_limit;
    ;

    setFrequency(this->currentFrequency);
  }

  // Overwriting parent method setFrequencyDown
  void setFrequencyDown() {
    uint16_t up_limit, down_limit;
    uint8_t b3 = this->getBand3Status();
    if (b3 == 0) {
      up_limit = 6500;
      down_limit = 5000;
    } else {
      up_limit = this->endBand[this->currentFMBand];
      down_limit = this->startBand[this->currentFMBand];
    }
    if (this->currentFrequency > down_limit)
      this->currentFrequency -= (this->fmSpace[currentFMSpace]);
    else
      this->currentFrequency = up_limit;

    setFrequency(this->currentFrequency);
  }
};

MyCustomRDA5807 radio;  // the instance of your custom class based on SI4735 class

void setup() {
  Serial.begin(9600);
  while (!Serial);
  Serial.println("Customizing RDA5807 class example.");
  radio.setup();
  radio.setFrequency(10390);
  Serial.println(radio.methodA());
  Serial.println(radio.methodB());
  radio.setBand3_50_65_Mode(0);
}

void loop() {
  radio.setFrequency(5000);
  radio.setFrequencyDown();
  delay(2000);
  Serial.print(radio.getFrequency());
  radio.setFrequencyUp();
  delay(2000);
  radio.setFrequency(6500);
  radio.setFrequencyUp();
  Serial.print(radio.getFrequency());
  delay(2000);
  radio.setFrequencyDown();
  delay(2000);
}