# RDA5807 Arduino Library

It is a cross-platform Arduini Library for RDA5807. You can use it with oficial Arduino boards, ATtiny, STM32 , ESP32 and more. It works with I2C protocol and can provide you an easier interface to control the RDA5807 device. This library was built based on "RDA5807M - SINGLE-CHIP BROADCAST FM RADIO TUNER - Rev.1.1–Aug.2015"

This library can be freely distributed using the MIT Free Software model. 

[Copyright (c) 2019 Ricardo Lima Caratti](https://pu2clr.github.io/RDA5807/#mit-license). 

Contact: __pu2clr@gmail.com__.


## Contents

1. [Preface](https://pu2clr.github.io/RDA5807#preface)
2. [Library Installation](https://pu2clr.github.io/RDA5807#library-installation)
3. [API Documentation](https://pu2clr.github.io/RDA5807/extras/apidoc/html)
4. [Schematic](https://pu2clr.github.io/RDA5807#schematic)
5. [Sketch examples](https://github.com/pu2clr/RDA5807/tree/master/examples)


## Preface 

The RDA5807 is a FM DSP integrated circuit receiver (50 to 115MHz) with low noise amplifier support. This device requires very few external components if compared with other similar devices. It also supports RDS/RBDS functionalities, direct auto gain control (AGC) and real time adaptive noise cancellation function. The PU2CLR RDA5807 Arduino Library was developed to take the most functionalities of this device. Plese, check the [API Documentation](https://pu2clr.github.io/RDA5807/extras/apidoc/html/) for more details. 


{% include video01.html %}

[See the presentation video on youtube](https://youtu.be/eLWEWEjxM8U) 

The photo below shows a Breakout that uses the RDA5807. You can find it on eBay, AliExpress and Amazon. 

![RDA5807 Breakout board](extras/images/breakout_01A.png)


## RDA5807 features implemented by this library

1. 76–108 MHz
2. Seek tuning
3. Automatic frequency control (AFC)
4. Automatic gain control (AGC)
5. Programmable de-emphasis (50/75 μs)
6. Adaptive noise suppression
7. Volume control
8. Mute control
9. Mono/Stereo control
10. RDS/RBDS Processor (under construction)


### See also

1. [PU2CLR Si4735 Library for Arduino](https://pu2clr.github.io/SI4735/). This library was built based on “Si47XX PROGRAMMING GUIDE; AN332” and it has support to FM, AM and SSB modes (LW, MW and SW). It also can be used on all members of the SI47XX family respecting, of course, the features available for each IC version; 
2. [PU2CLR SI4844 Arduino Library](https://github.com/pu2clr/SI4844). This is an Arduino library for the SI4844, BROADCAST ANALOG TUNING DIGITAL DISPLAY AM/FM/SW RADIO RECEIVER,  IC from Silicon Labs.  It is available on Arduino IDE. This library is intended to provide an easier interface for controlling the SI4844.
3. [PU2CLR AKC695X Arduino Library](https://pu2clr.github.io/AKC695X/). The AKC695X is a family of IC DSP receiver from AKC technology. The AKC6955 and AKC6959sx support AM and FM modes. On AM mode the AKC6955 and AKC6959sx work on LW, MW and SW. On FM mode they work from 64MHz to 222MHz.
4. [PU2CLR KT0915 Arduino Library](https://pu2clr.github.io/KT0915/).
5. [PU2CLR SI470X Arduino Library](https://pu2clr.github.io/SI470X/).


## MIT License 

Copyright (c) 2019 Ricardo Lima Caratti

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE ARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


## Library Installation

 The easiest method to install this library is via your Arduino IDE. All you have to do is:
 
 1. Select Tools menu;
 2. Select Manage Libraries option;
 3. In the text box (top windows), type PU2CLR or RDA5807;
 4. Select the PU2CLR RDA5807. 


The images below show how you can install this library via your Arduino IDE from Manage Libraries tool.

<BR>

![Arduino IDE - Manage Libraries menu option](extras/images/RDA5807_install_01.png)

<BR>

![Arduino IDE - Looking for the library RDA5807](extras/images/RDA5807_install_02.png)


### Installing via the repository 

With this approach, you will have the __most current version__ of the library. However, __it may not be the most stable version__. This is because the current version is always in development. [Prefer releases](https://github.com/pu2clr/RDA5807/releases). Do you need some old version (release) of this library?  If yes, [check here](https://github.com/pu2clr/RDA5807/releases).


First, you have to [download](https://github.com/pu2clr/RDA5807/archive/master.zip) this library in zip format.  
After, unzip the RDA5807-master.zip file in your Arduino Library folder. 

* On __Windows__: "My Documents\Arduino\libraries"
* On __MAC OS__: ˜/Documents/Arduino/libraries
* On __Linux__: ˜/Documents/Arduino/libraries

<BR>

## Schematic

 In general, the RDA5807 can be found in kit or breakout form. In this case, the circuit below can help you to connect the arduino to the RDA5807 shield. If you are using just the IC, you might want to check the  [RDA5807M - SINGLE-CHIP BROADCAST FM RADIO TUNER - Rev.1.1–Aug.2015](https://datasheet.lcsc.com/szlcsc/RDA-Microelectronics-RDA5807MS_C167246.pdf); page 17.


![Basic Schematic](./extras/images/circuit_basic.png)


### Wire up on Arduino UNO, Pro mini or other based on ATmega 328.

| RDA5807 / Description |  Arduino Pin  |
| --------------------- | ------------  |
| SDA / SDIO            |     A4        |
| SCL / SCLK            |     A5        |




## RDA5807 breakout, ATtiny84, Encoder and Buttons schematic


![Basic Schematic](./extras/images/circuit_attiny84.png)


### RDA5807 breakout, ATtiny84, Encoder and Buttons  wireup  

<BR>

| RDA5807 pin     | Attiny84 REF pin | Physical pin  | 
| ----------------| -----------------| ------------- | 
| SEEK_UP         |     3            |    10         | 
| SEEK_DOWN       |     5            |     8         |
| ENCODER_PIN_A   |     0            |    13         |
| ENCODER_PIN_B   |     1            |    12         |  
| SDIO / SDA      |     SDA          |     7         |
| SCLK / CLK      |     SCL          |     9         |



# References 

* [RDA5807M - SINGLE-CHIP BROADCAST FMRADIO TUNER](https://www.electrodragon.com/w/images/5/5f/RDA5807M_datasheet_v1.pdf)
* [A small eagle library for popular RDA5807m Radio module](https://github.com/TigerBouli/RDA5807m-Module-)

