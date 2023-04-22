# Extras





## Schematic


### Basic circuit (examples  RDA5807_00_CIRCUIT_TEST and RDA5807_01_SERIAL_MONITOR)


![Basic Schematic](./images/circuit_basic.png)


### TFT Display (example RDA5807_02_TFT_display)

![Schematic - Arduino and TFT Display ](./images/circuit_tft.png)

![Basic Schematic with TFT](./extras/images/circuit_tft.png)


Arduino UNO, Nano or other based on Atmega328 and SPI TFT ST7735 1.8" wireup

| Device name       | Device Pin / Description  |  Arduino Pin  |
| ----------------  | --------------------      | ------------  |
| Display TFT       |                           |               |
|                   | RST (RESET)               |      8        |
|                   | RS  or DC                 |      9        |
|                   | CS  or SS                 |     10        |
|                   | SDI                       |     11        |
|                   | CLK                       |     13        |
|     RDA5807       |                           |               |
|                   | VCC                       |     3.3V      | 
|                   | SDIO (pin 8)              |     A4        |
|                   | SCLK (pin 7)              |     A5        |
|     Buttons       |                           |               |
|                   | Volume Up                 |      4        |
|                   | Volume Down               |      5        |
|                   | Stereo/Mono               |      6        |
|                   | RDS ON/off                |      7        |
|                   | SEEK (encoder button)     |     A0/14     |
|    Encoder        |                           |               |
|                   | A                         |       2       |
|                   | B                         |       3       |


### Attiny84 (example RDA5807_03_attimy84)

![Schematic - Arduino and TFT Display ](./images/circuit_attiny84.png)

#### RDA5807 breakout, ATtiny84, Encoder and Buttons  wireup  

<BR>

| RDA5807 pin     | Attiny84 REF pin | Physical pin  | 
| ----------------| -----------------| ------------- | 
| SEEK_UP         |     3            |    10         | 
| SEEK_DOWN       |     5            |     8         |
| ENCODER_PIN_A   |     0            |    13         |
| ENCODER_PIN_B   |     1            |    12         |  
| SDIO / SDA      |     SDA          |     7         |
| SCLK / CLK      |     SCL          |     9         |




### Arduino Nano and NOKIA5110 schematic


![Schematic -  Arduino Nano and NOKIA5110](./images/RDA5807_NOKIA5110_01.png)


#### RDA5807 breakout, Arduino Nano and Nokia 5110 display wireup


| Device name               | Nokia 5110                |  Arduino      |
| --------------------------| --------------------      | ------------  |
| NOKIA 5110                | Pin function              |  Nano Pin     |
|                           | (1) RST (RESET)           |     8         |
|                           | (2) CE or CS              |     9         |
|                           | (3) DC or DO              |    10         |
|                           | (4) DIN or DI or MOSI     |    11         |
|                           | (5) CLK                   |    13         |
|                           | (6) VCC  (3V-5V)          |    +VCC       |
|                           | (7) BL/DL/LIGHT           |    +VCC       |
|                           | (8) GND                   |    GND        |
| --------------------------| ------------------------- | --------------|
| RDA5807                   |       Pin Function        |               | 
|                           | VCC                       |   3.3V        | 
|                           | SDIO (pin 8)              |     A4        |
|                           | SCLK (pin 7)              |     A5        |
| --------------------------| --------------------------| --------------|
| Buttons                   |                           |               |
|                           | Volume Up                 |      4        |
|                           | Volume Down               |      5        |
|                           | Stereo/Mono               |      6        |
|                           | RDS ON/off                |      7        |
|                           | SEEK (encoder button)     |     A0/14     |
| --------------------------| --------------------------|---------------| 
| Encoder                   |                           |               |
|                           | A                         |       2       |
|                           | B                         |       3       |



### Arduino Nano and LCD 16x02 


![Schematic -  Arduino Nano and LCD 16x02](./images/RDA5807_LCD16X02.png)



### ESP32 and LCD 16x02 


![Schematic -  ESP32 and LCD 16x02](./images/RDA5807_LCD16X02_ESP32_RDA5807FP.png)


### Arduino UNO and display module TM1638


![Schematic -  Arduino UNO and display module TM1638](./images/TM1638_UNO_RDA5807FP.png)


### I2S and RDA5807FP Setup 


![Schematic - I2S and RDA5807FP Setup ](./images/RDA5807FP_I2S_MAX98357A.png)
