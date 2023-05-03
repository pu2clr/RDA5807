/*
  MyRDA5807 implementation
*/

#include "MyRDA5807.h"

void MyRDA5807::getBandLimits() {
  if (this->getBand3Status() == 0) {
    up_limit = 6500;
    down_limit = 5000;
  } else {
    up_limit = this->endBand[this->currentFMBand];
    down_limit = this->startBand[this->currentFMBand];
  }
}

// Implements some new members functions to the new class
int MyRDA5807::getSoftBlendEnable() {  // A RDA5807 command that PU2CLR RDA5807 Arduino Library does not implement
  rda_reg07 tmp;
  tmp.raw = this->getDirectRegister(0x07).raw;
  return tmp.refined.SOFTBLEND_EN;
}

uint16_t MyRDA5807::getDeviceInfo() {  // another RDA5807 command that PU2CLR RDA5807 Arduino Library does not implement
  rda_reg00 tmp;
  tmp.raw = this->getDirectRegister(0x00).raw;
  return tmp.refined.HIGH_CHIP_ID;
}

// Overwriting parent method setFrequencyUp - Chenging the behavior of the setFrequencyUp function
void MyRDA5807::setFrequencyUp() {
  getBandLimits();
  if (this->currentFrequency < up_limit)
    this->currentFrequency += (this->fmSpace[currentFMSpace]);
  else
    this->currentFrequency = down_limit;

  setFrequency(this->currentFrequency);
}

// Overwriting parent method setFrequencyDown - Chenging the behavior of the setFrequencyDown function
void MyRDA5807::setFrequencyDown() {
  getBandLimits();
  if (this->currentFrequency > down_limit)
    this->currentFrequency -= (this->fmSpace[currentFMSpace]);
  else
    this->currentFrequency = up_limit;

  setFrequency(this->currentFrequency);
}
