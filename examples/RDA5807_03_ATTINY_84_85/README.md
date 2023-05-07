# ATTiny84 and ATTiny85 setup



## Attiny84 

![Schematic - Attiny84 with oled setup ](../../extras/images/circuit_attiny84.png)


### Attiny84 wireup

| RDA5807 pin     | Attiny84 REF pin | Physical pin  | 
| ----------------| -----------------| ------------- | 
| SEEK_UP         |     3            |    10         | 
| SEEK_DOWN       |     5            |     8         |
| ENCODER_PIN_A   |     0            |    13         |
| ENCODER_PIN_B   |     1            |    12         |  
| SDIO / SDA      |     SDA          |     7         |
| SCLK / CLK      |     SCL          |     9         |




## Attiny85 

![Schematic - Attiny85 with oled setup ](../../extras/images/circuit_attiny85.png)


### ATtiny85 and RDA5807 wireup  

| RDA5807 pin      | Attiny85 REF pin | Physical pin | 
| ----------------| -----------------| ------------- | 
| SEEK_UP         |     PB1          |     6         | 
| SEEK_DOWN       |     PB4          |     3         |
| SDIO / SDA      |     SDA          |     5         |
| SCLK / CLK      |     SCL          |     7         |
   
<BR>

### Installing ATtiny Core in Arduino IDE 

The ATtiny core board/plataform can be installed using the Arduino IDE boards manager. 
Inserts the URL http://drazzy.com/package_drazzy.com_index.json on board manager. To do that, go to Preferences, enter the above URL in "Additional Boards Manager URLs. To setup ATtiny85 on Arduino IDE, go to Tools Menu, Board, Board Manager and install "ATTinyCore by Spence Konde". 

See [ATtiny Core - 1634, x313, x4, x41, x5, x61, x7, x8 and 828 for Arduino](https://github.com/SpenceKonde/ATTinyCore).

See also [ATtiny85 pinout](https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf).


