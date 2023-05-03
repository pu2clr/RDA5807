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
class MyRDA5807 : public RDA5807 {  // extending the original PU2CLR RDA5807 class

private: 
  // Implements some specifics members and methods to the new class if necessary
  uint16_t up_limit, down_limit;
  void getBandLimits();

public:
  // Implements some new members functions to the new class
  int getSoftBlendEnable();  // A RDA5807 command that PU2CLR RDA5807 Arduino Library does not implement
  uint16_t getDeviceInfo();  // another RDA5807 command that PU2CLR RDA5807 Arduino Library does not implement

  // Overwriting parent method setFrequencyUp - Chenging the behavior of the setFrequencyUp function
  void setFrequencyUp(); 
  // Overwriting parent method setFrequencyDown - Chenging the behavior of the setFrequencyDown function
  void setFrequencyDown();
};

