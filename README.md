# SI470X Arduino Library

It is an Arduini Library for Si4702/03 FM tuner family. 


## Contents

1. [Preface](https://pu2clr.github.io/SI470X#preface)
2. [API Documentation](https://pu2clr.github.io/SI470X/extras/apidoc/html)



## Si4703 features implemented by this library

1. 76–108 MHz
2. Seek tuning
3. Automatic frequency control (AFC)
4. Automatic gain control (AGC)
5. Programmable de-emphasis (50/75 μs)
6. Adaptive noise suppression
7. Volume control
8. RDS/RBDS Processor



## Preface 

The SI470X is a FM DSP receiver with RDS/RBDS support.  This document you will give you Arduino source codes, schematics, examples and tips to help you to build a receiver based on Arduino board and the SI470X Shield. The following figure shows a very common kit sold on eBay and AliExpress.

![SI4703 Shield](https://github.com/pu2clr/SI470X/blob/master/extras/images/si4703_module0.png)


### See also

1. [PU2CLR Si4735 Library for Arduino](https://pu2clr.github.io/SI4735/). This library was built based on “Si47XX PROGRAMMING GUIDE; AN332” and it has support to FM, AM and SSB modes (LW, MW and SW). It also can be used on all members of the SI47XX family respecting, of course, the features available for each IC version; 
2. [PU2CLR SI4844 Arduino Library](https://github.com/pu2clr/SI4844). This is an Arduino library for the SI4844, BROADCAST ANALOG TUNING DIGITAL DISPLAY AM/FM/SW RADIO RECEIVER,  IC from Silicon Labs.  It is available on Arduino IDE. This library is intended to provide an easier interface for controlling the SI4844.
3. [PU2CLR AKC695X Arduino Library](https://pu2clr.github.io/AKC695X/). The AKC695X is a family of IC DSP receiver from AKC technology. The AKC6955 and AKC6959sx support AM and FM modes. On AM mode the AKC6955 and AKC6959sx work on LW, MW and SW. On FM mode they work from 64MHz to 222MHz.
4. [PU2CLR KT0915 Arduino Library](https://pu2clr.github.io/KT0915/).


## About the Si4703




## SI470X and Registers

The SI4701/02/03 can be controlled by deal with register via I2C interface.  The tebla below was extracted from Silicon Labs; document Si4702/03-C19 - "BROADCAST FM RADIO TUNER FOR PORTABLE APPLICATIONS"; Rev 1.1; page 22. This table shows the set of register that you can used to controle the device.


![SI4702/03 Register Summary](https://github.com/pu2clr/SI470X/blob/master/extras/images/SI470X_REGISTER_SUMMARY.png)

Source: Silicon Labs; document Si4702/03-C19 - "BROADCAST FM RADIO TUNER FOR PORTABLE APPLICATIONS"; Rev 1.1; Page 22.


__Except that you need something very specific, the PU2CLR SI470X Arduino Library offers all the functions necessary for you to build your own FM receiver using an Arduino Board__.

If you need something else, this library implemented two basic functions to deal direct with the device registers shown above. See setAllRegister and getAllRegister functions on [https://pu2clr.github.io/SI470X/extras/apidoc/html/](https://pu2clr.github.io/SI470X/extras/apidoc/html/).













# References 

* [RDA5807M - SINGLE-CHIP BROADCAST FMRADIO TUNER](https://www.electrodragon.com/w/images/5/5f/RDA5807M_datasheet_v1.pdf)
* [](https://github.com/csdexter/RDA5807M)
