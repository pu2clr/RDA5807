/**
 * @mainpage RDA5807 Arduino Library implementation
 * @details RDA5807 Arduino Library implementation. This is an Arduino library for the RDA5807, BROADCAST RECEIVER.
 * @details It works with I2C protocol and can provide an easier interface to control the RDA5807 device.<br>
 * @details This library was built based on "RDA5807M - SINGLE-CHIP BROADCAST FM RADIO TUNER - Rev.1.0–May.2011"
 * @details This library can be freely distributed using the MIT Free Software model.
 * @copyright Copyright (c) 2020 Ricardo Lima Caratti.
 * @author Ricardo LIma Caratti (pu2clr@gmail.com)
 */

#include <RDA5807.h>

/**
 * @defgroup GA03 Basic Functions
 * @section GA03 Basic
 */

/**
 * @ingroup GA03
 * @brief Gets all current device status and RDS information registers (From 0x0A to 0x0F)
 * @see RDA5807M - SINGLE-CHIP BROADCAST FMRADIO TUNER; pages 5, 9, 12 and 13. 
 * @see rda_reg0a, rda_reg0b, rda_reg0c, rda_reg0d, rda_reg0e, rda_reg0f
 * @see shadowStatusRegisters;
 */
void RDA5807::getStatusRegisters()
{
    word16_to_bytes aux;
    int i;

    Wire.requestFrom(this->deviceAddressDirectAccess, 12); // This call starts reading from 0x0A register
    delayMicroseconds(250);
    for (i = 0; i <= 6; i++) {
        aux.refined.highByte = Wire.read();
        aux.refined.lowByte = Wire.read();
        shadowStatusRegisters[i] = aux.raw;
    }
    Wire.endTransmission();
}

/**
 * @ingroup GA03
 * @brief Gets the register content of a given status register (from 0x0A to 0x0F) 
 * @details Useful when you need just a specific status register content.
 * @details This methos update the first element of the shadowStatusRegisters linked to the register
 * @return rdax_reg0a the reference to current value of the 0x0A register. 
 */
void *RDA5807::getStatus(uint8_t reg)
{
    word16_to_bytes aux;
    if ( reg < 0x0A || reg > 0x0F ) return NULL;  // Maybe not necessary.
    Wire.requestFrom(this->deviceAddressDirectAccess, 2); // reading 0x0A register
    delayMicroseconds(250);
    aux.refined.highByte = Wire.read();
    aux.refined.lowByte = Wire.read();
    shadowStatusRegisters[reg - 0x0A] = aux.raw;
    return &shadowStatusRegisters[reg - 0x0A];
}

/**
 * @ingroup GA03
 * @brief   Sets values to the device registers from 0x02 to 0x07
 * @details An internal address counter automatically increments to allow continuous data byte writes, starting with the upper byte of register 02h, followed by the lower byte of register 02h, and onward until the lower byte of the last register is reached. 
 * @details The registers from 0x2 to 0x07 are used to setup the device. This method writes the array  shadowStatusRegisters. See Device registers map  in RDA5807.h file.
 * @details To implement this, a register maping was created to deal with each register structure. For each type of register, there is a reference to the array element.
 * @see shadowStatusRegisters;
 */
void RDA5807::setAllRegisters()
{
    word16_to_bytes aux;
    Wire.beginTransmission(this->deviceAddressFullAccess);
    for (int i = 0x02; i <= 0x7; i++)
    {
        aux.raw = shadowStatusRegisters[i];
        Wire.write(aux.refined.highByte);
        Wire.write(aux.refined.lowByte);
    }
    Wire.endTransmission();
}

/**
 * @ingroup GA03
 * @brief Sets a given value to a specific device register 
 *
 * @see RDA5807M - SINGLE-CHIP BROADCAST FMRADIO TUNER; pages 5, 9, 10 and 11. 
 * @see rda_reg02, rda_reg03, rda_reg04, rda_reg05, rda_reg06, rda_reg07
 *
 * @param reg    register number (valid values is between 0x02 and 0x07)   
 * @param value  the unsigned 16 bits word value (see rda_rec0x data types)   
 */
void RDA5807::setRegister(uint8_t reg, uint16_t value)
{
    word16_to_bytes aux;
    if (reg > 8) return; // Maybe not necessary.
    Wire.beginTransmission(this->deviceAddressDirectAccess);
    Wire.write(reg);
    aux.raw = value;
    Wire.write(aux.refined.highByte);
    Wire.write(aux.refined.lowByte);
    Wire.endTransmission();
    shadowRegisters[reg] = aux.raw;  // Updates the shadowRegisters element
    delayMicroseconds(250);
}

/**
 * @ingroup GA03
 */
void RDA5807::waitAndFinishTune()
{

}

/**
 * @ingroup GA03
 * @brief Resets the device
 * @details The RDA5807M is RESET itself When VIO is Power up. 
 * @details Also, it support soft reset by triggering the 0x02 register (rda_reg02) bit 1 from 0 to 1. 
 */
void RDA5807::softReset()
{
    reg02->refined.SOFT_RESET = 1;
    setRegister(REG02,reg02->raw);
}

/**
 * @ingroup GA03
 * @brief Powers the receiver on
 */
void RDA5807::powerUp()
{
    reg02->raw = 0;
    reg02->refined.NEW_METHOD = 0;
    reg02->refined.RDS_EN = 0;  // RDS disable
    reg02->refined.CLK_MODE = this->clockType;
    reg02->refined.RCLK_DIRECT_IN = this->oscillatorType;
    reg02->refined.MONO = 1;    // Force mono
    reg02->refined.DMUTE = 1;   // Normal operation
    reg02->refined.DHIZ = 1;    // Normal operation
    reg02->refined.ENABLE = 1;

    // reg02->refined.SOFT_RESET = 1;
    // setRegister(REG02,reg02->raw);

    reg05->raw = 0x9081;

    reg06->raw = 0;
    reg07->raw = 0; 
    reg08->raw = 0;

    setAllRegisters();

    // reg02->refined.ENABLE = 1;
    // reg02->raw = 1;
    setRegister(REG02, reg02->raw);

    /*
    reg05->raw = 0;
    reg05->refined.VOLUME = this->currentVolume;
    reg05->refined.INT_MODE = 1;
    reg05->refined.SEEKTH = 0b1000;
    reg05->refined.LNA_PORT_SEL = 2;
    reg05->refined.LNA_ICSEL_BIT = 1;
    setRegister(REG05, reg05->raw);
    */
}

/**
 * @ingroup GA03
 * @brief
 */
void RDA5807::powerDown()
{
    reg02->refined.ENABLE = 0;
    setRegister(REG02, reg02->raw);
}

/**
 * @ingroup GA03
 * @brief Starts the device
 * @param clock_type       Clock used.
 * @param oscillator_type  optional. Sets the Oscillator type used (Default Crystal or Ref. Clock).
 */
void RDA5807::setup(uint8_t clock_type, uint8_t oscillator_type)
{
    this->oscillatorType = oscillator_type;
    this->clockType = clock_type;

    Serial.println("Cheguei aqui");

    Wire.begin();
    delay(1);
    powerUp();
    Serial.println("Passei");
}


/**
 * @ingroup GA03
 * @brief Sets the channel
 * @param channel
 */
void RDA5807::setChannel(uint16_t channel)
{
    /*
    reg02->refined.ENABLE = 1;
    reg02->refined.RDS_EN = 0;
    reg02->refined.DHIZ = 1;
    reg02->refined.DMUTE = 1;
    setRegister(REG02, reg02->raw);
    */

    reg03->refined.CHAN = channel;
    reg03->refined.TUNE = 1;
    reg03->refined.BAND = this->currentFMBand;
    reg03->refined.SPACE = this->currentFMSpace;
    reg03->refined.DIRECT_MODE = 0;
    setRegister(REG03, reg03->raw);

    /* 
      reg05->refined.LNA_PORT_SEL = 2;
      setRegister(REG05, reg05->raw);
    */
}

/**
 * @ingroup GA03
 * @brief Sets the frequency
 * @param frequency
 */
void RDA5807::setFrequency(uint16_t frequency)
{
    uint16_t channel = (frequency - this->startBand[currentFMBand] ) / (this->fmSpace[this->currentFMSpace] / 10.0);
    setChannel(channel);
    this->currentFrequency = frequency;
}

/**
 * @ingroup GA03
 * @brief Gets the current frequency.
 * @return uint16_t
 */
uint16_t RDA5807::getFrequency()
{
    return this->currentFrequency;
}

/**
 * @ingroup GA03
 * @brief
 * @return uint16_t
 */
uint16_t RDA5807::getRealChannel()
{

}

/**
 * @ingroup GA03
 * @brief
 *
 * @return uint16_t
 */
uint16_t RDA5807::getRealFrequency() {

}

/**
 * @ingroup GA03
 * @brief Seek function
 *
 * @param seek_mode
 * @param direction
 */
void RDA5807::seek(uint8_t seek_mode, uint8_t direction)
{

}

/**
 * @ingroup GA03
 * @brief Sets RSSI Seek Threshold
 * @param  value
 */
void RDA5807::setSeekThreshold(uint8_t value)
{

}

/**
 * @ingroup GA03
 * @brief
 */
void RDA5807::setBand(uint8_t band)
{


}

/**
 * @ingroup GA03
 * @brief
 */
void RDA5807::setSpace(uint8_t space)
{

}

/**
 * @ingroup GA03
 * @brief Gets the Rssi
 *
 * @return int
 */
int RDA5807::getRssi()
{

}

/**
 * @ingroup GA03
 * @brief
 */
void RDA5807::setSoftmute(bool value)
{

}

/**
 * @ingroup GA03
 * @brief
 *
 * @param value
 */
void RDA5807::setSoftmuteAttack(uint8_t value){

}

/**
 * @ingroup GA03
 * @brief
 */
void RDA5807::setSoftmuteAttenuation(uint8_t value)
{

}

/**
 * @ingroup GA03
 * @brief
 */
void RDA5807::setAgc(bool value) {

}


/**
 * @ingroup GA03
 * @brief
 * @param value TRUE or FALSE
 */
void RDA5807::setMute(bool value)
{
    reg02->refined.DHIZ = !value;
    setRegister(REG02,reg02->raw); 
}

/**
 * @ingroup GA03
 * @brief
 *
 * @param value TRUE or FALSE
 */
void RDA5807::setMono(bool value)
{
    reg02->refined.MONO = value;
    setRegister(REG02, reg02->raw);
}


/**
 * @ingroup GA03
 * @brief Sets the RDS operation
 * @details Enable or Disable the RDS
 *
 * @param true = turns the RDS ON; false  = turns the RDS OFF
 */
void RDA5807::setRds(bool value)
{

}

/**
 * @ingroup GA03
 * @brief Sets the Rds Mode Standard or Verbose
 *
 * @param rds_mode  0 = Standard (default); 1 = Verbose
 */
void RDA5807::setRdsMode(uint8_t rds_mode)
{

}

/**
 * @ingroup GA03
 * @brief Sets the audio volume level
 *
 * @param value
 */
void RDA5807::setVolume(uint8_t value)
{
    if ( value > 15 ) value = 15;

    reg05->refined.VOLUME = this->currentVolume;
    setRegister(REG05, reg05->raw);
}

/**
 * @ingroup GA03
 * @brief Gets the current audio volume level
 *
 * @return uint8_t  0 to 15
 */
uint8_t RDA5807::getVolume()
{
    return this->currentVolume;
}

/**
 * @ingroup GA03
 * @brief Increments the audio volume
 *
 */
void RDA5807::setVolumeUp()
{
    if (this->currentVolume < 15)
    {
        this->currentVolume++;
        setVolume(this->currentVolume);
    }
}

/**
 * @ingroup GA03
 * @brief Decrements the audio volume
 *
 */
void RDA5807::setVolumeDown()
{
    if (this->currentVolume > 0)
    {
        this->currentVolume--;
        setVolume(this->currentVolume);
    }
}



/**
 * @ingroup GA03
 * @brief Gets the Part Number
 * @return the part number
 */
uint8_t RDA5807::getPartNumber()
{
  return 0;
}

/**
 * @ingroup GA03
 * @brief Gets the Manufacturer ID
 * @return number
 */
uint16_t RDA5807::getManufacturerId()
{
  return 0;
}

/**
 * @ingroup GA03
 * @brief Gets the Firmware Version
 * @details The return velue before powerup will be 0. After powerup should be 010011 (19)
 * @return number
 */
uint8_t RDA5807::getFirmwareVersion()
{
  return 0;
}

/**
 * @ingroup GA03
 * @brief Gets the Device identification
 * @return number
 */
uint8_t RDA5807::getDeviceId()
{
    return 0;
}

/**
 * @ingroup GA03
 * @brief Gets the Chip Version
 * @return number
 */
uint8_t RDA5807::getChipVersion()
{
    return 0;
}

/**
 * @brief Sets De-emphasis.
 * @details 75 μs. Used in USA (default); 50 μs. Used in Europe, Australia, Japan.
 *
 * @param de  0 = 75 μs; 1 = 50 μs
 */
void RDA5807::setFmDeemphasis(uint8_t de) {
    
}
