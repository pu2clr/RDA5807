# This script uses the arduino-cli to compile the arduino sketches using command line (without Arduino IDE).
# It is very useful to check the library on multiple board types after bug fixes and improvements.
# It runs on a MacOS but you can port it to Linux or Windows easily.
# Some compilation results (bin and hex files) will be stores in your Download directory (/Users/<username>/Downloads/hex)
# ATTENTION: 1) Be careful with --output-dir arduino-cli option. Before compiling, this option removes all the contents of the last level folder. 
#               For example: if you use "--output-dir ~/Downloads", all the current content of the Downloads folder will be lost. 
#                         if you use "--output-dir ~/Downloads/hex", all current content of the hex folder will be lost and the Downloads 
#                         content will be preserved. 
#            2) I have enabled all the compile warnings (--warnings all) to check some questionable situations that can be avoided or modified to prevent future warnings.  
#            3) I have enabled  the "--verbose" parameter to show the deteiled logs of the compiling process.
# Please, see the file config_libraries_and_boards.sh
# Ricardo Lima Caratti Mar 2023

# compiles POC
echo "********************"
echo "Arduino ATmega328 based board"
echo "********************"
arduino-cli compile -b arduino:avr:nano ./RDA5807_01_SERIAL_MONITOR/RDA5807_00_CIRCUIT_TEST --output-dir ~/Downloads/hex/atmega/RDA5807_00_CIRCUIT_TEST  --warnings all
arduino-cli compile -b arduino:avr:nano ./RDA5807_02_TFT_display --output-dir ~/Downloads/hex/atmega/RDA5807_02_TFT_display  --warnings all
arduino-cli compile -b arduino:avr:nano ./RDA5807_04_NOKIA5110_RDS --output-dir ~/Downloads/hex/atmega/RDA5807_04_NOKIA5110_RDS  --warnings all
arduino-cli compile -b arduino:avr:nano ./RDA5807_05_A_LCD16X02_NANO --output-dir ~/Downloads/hex/atmega/RDA5807_05_A_LCD16X02_NANO  --warnings all
arduino-cli compile -b arduino:avr:nano ./RDA5807_05_A_LCD20X04_NANO --output-dir ~/Downloads/hex/atmega/RDA5807_05_A_LCD20X04_NANO  --warnings all


arduino-cli compile -b arduino:avr:uno ./RDA5807_06_UNO_TM1638 --output-dir ~/Downloads/hex/atmega/RDA5807_06_UNO_TM1638  --warnings all
arduino-cli compile -b arduino:avr:uno ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_ALL_TEST_SERIAL_MONITOR --output-dir ~/Downloads/hex/atmega/RDA5807_01_ALL_TEST_SERIAL_MONITOR  --warnings all
arduino-cli compile -b arduino:avr:uno ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_BAND_TEST_SERIAL_MONITOR --output-dir ~/Downloads/hex/atmega/RDA5807_01_BAND_TEST_SERIAL_MONITOR  --warnings all
arduino-cli compile -b arduino:avr:uno ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_RDS_TEST_01_SERIAL_MONITOR --output-dir ~/Downloads/hex/atmega/RDA5807_01_RDS_TEST_01_SERIAL_MONITOR  --warnings all
arduino-cli compile -b arduino:avr:uno ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_RDS_TEST_02_SERIAL_MONITOR --output-dir ~/Downloads/hex/atmega/RDA5807_02_RDS_TEST_01_SERIAL_MONITOR  --warnings all
arduino-cli compile -b arduino:avr:uno ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_RDS_TEST_03_INTERRUPT_SERIAL_MONITOR --output-dir ~/Downloads/hex/atmega/RDA5807_01_RDS_TEST_03_INTERRUPT_SERIAL_MONITOR  --warnings all


arduino-cli compile -b arduino:avr:uno ./RDA5807_07_NANO_OLED --output-dir ~/Downloads/hex/atmega/RDA5807_07_NANO_OLED  --warnings all 
arduino-cli compile -b arduino:avr:uno ./RDA5807_07_NANO_OLED_V2 --output-dir ~/Downloads/hex/atmega/RDA5807_07_NANO_OLED_V2  --warnings all 


echo "********************"
echo "Arduino LGT8FX based board"
echo "********************"

arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_01_SERIAL_MONITOR/RDA5807_00_CIRCUIT_TEST --output-dir ~/Downloads/hex/lgt8fx/RDA5807_00_CIRCUIT_TEST  --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_02_TFT_display --output-dir ~/Downloads/hex/lgt8fx/RDA5807_02_TFT_display  
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_04_NOKIA5110_RDS --output-dir ~/Downloads/hex/lgt8fx/RDA5807_04_NOKIA5110_RDS  
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_05_A_LCD16X02_NANO --output-dir ~/Downloads/hex/lgt8fx/RDA5807_05_A_LCD16X02_NANO  --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_05_A_LCD20X04_NANO --output-dir ~/Downloads/hex/lgt8fx/RDA5807_05_A_LCD20X04_NANO  --warnings all

arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_06_UNO_TM1638 --output-dir ~/Downloads/hex/lgt8fx/RDA5807_06_UNO_TM1638  --warnings all

arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_ALL_TEST_SERIAL_MONITOR --output-dir ~/Downloads/hex/lgt8fx/RDA5807_01_ALL_TEST_SERIAL_MONITOR  --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_BAND_TEST_SERIAL_MONITOR --output-dir ~/Downloads/hex/lgt8fx/RDA5807_01_BAND_TEST_SERIAL_MONITOR  --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_RDS_TEST_01_SERIAL_MONITOR --output-dir ~/Downloads/hex/lgt8fx/RDA5807_01_RDS_TEST_01_SERIAL_MONITOR  --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_RDS_TEST_02_SERIAL_MONITOR --output-dir ~/Downloads/hex/lgt8fx/RDA5807_01_RDS_TEST_02_SERIAL_MONITOR  --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_01_SERIAL_MONITOR/RDA5807_01_RDS_TEST_03_INTERRUPT_SERIAL_MONITOR --output-dir ~/Downloads/hex/lgt8fx/RDA5807_01_RDS_TEST_03_INTERRUPT_SERIAL_MONITOR  --warnings all

arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_07_NANO_OLED_V2 --output-dir ~/Downloads/hex/lgt8fx/RDA5807_07_NANO_OLED_V2  

# compiles ESP32 LCD16x2_ALL_IN_ONE
echo "********************"
echo "ESP32"
echo "ESP32 LCD16x20"
echo "********************"
arduino-cli compile --fqbn esp32:esp32:esp32-poe-iso ./RDA5807_05_LCD16X02_ESP32 --output-dir ~/Downloads/hex/ESP32/DEVM/RDA5807_05_LCD16X02_ESP32  --warnings all
arduino-cli compile --fqbn esp32:esp32:esp32-poe-iso ./RDA5807_05_LCD16X02_ESP32_I2S --output-dir ~/Downloads/hex/ESP32/DEVM/RDA5807_05_LCD16X02_ESP32_I2S  --warnings all

# compiles STM32 SERIAL MONITOR
echo "********************"
echo "STM32"
echo "********************"
arduino-cli compile --fqbn stm32duino:STM32F1:genericSTM32F103C ./RDA5807_01_SERIAL_MONITOR/RDA5807_03_STM32 --output-dir ~/Downloads/hex/STM32/F1/RDA5807_03_STM32


echo "********************"
echo "STM32"
echo "********************"
# arduino-cli board -b stm32duino:STM32F1:genericSTM32F103C  details
arduino-cli compile  --fqbn stm32duino:STM32F1:genericSTM32F103C ./RDA5807_01_SERIAL_MONITOR/RDA5807_03_STM32 --output-dir ~/Downloads/hex/STM32/F1/RDA5807_03_STM32  --warnings all


# compiles ATtiny84 and ATtiny85
# echo "ATTINY84 and ATTINY84"
# arduino-cli board -b ATTinyCore:avr:attinyx4  details   
arduino-cli compile --fqbn ATTinyCore:avr:attinyx4:millis=enabled  ./RDA5807_03_ATTINY_84_85/RDA5807_ATTINY84 --output-dir ~/Downloads/hex/ATTIMY84/RDA5807_ATTINY84 --warnings all
arduino-cli compile --fqbn ATTinyCore:avr:attinyx4:millis=enabled  ./RDA5807_03_ATTINY_84_85/RDA5807_ATTINY84_RDS_OLED96_EEPROM --output-dir ~/Downloads/hex/ATTIMY84/RDA5807_ATTINY84_RDS_OLED96_EEPROM --warnings all
arduino-cli compile --fqbn ATTinyCore:avr:attinyx5:millis=enabled  ./RDA5807_03_ATTINY_84_85/RDA5807_ATTINY85_OLED91 --output-dir ~/Downloads/hex/ATTIMY85/RDA5807_ATTINY85_OLED91 --warnings all
arduino-cli compile --fqbn ATTinyCore:avr:attinyx5:millis=enabled  ./RDA5807_03_ATTINY_84_85/RDA5807_ATTINY85_ZIP --output-dir ~/Downloads/hex/ATTIMY85/RDA5807_ATTINY85_ZIP --warnings all
arduino-cli compile --fqbn ATTinyCore:avr:attinyx5:millis=enabled  ./RDA5807_03_ATTINY_84_85/RDA5807_ATTINY85_RDS_OLED96_EEPROM --output-dir ~/Downloads/hex/ATTIMY85/RDA5807_ATTINY85_RDS_OLED96_EEPROM --warnings all


# UNDER CONSTRUCTION...

# compile ESP8266
echo "ESP8266 OLED"
arduino-cli compile --fqbn esp8266:esp8266:generic  ./UNDER_CONSTRUCTION/RDA5807_06_ESP8266_OLED --output-dir ~/Downloads/hex/ESP8266/RDA5807_06_ESP8266_OLED  --warnings all

arduino-cli compile -b arduino:avr:uno  ./UNDER_CONSTRUCTION/RDA5807_TFT_TOUCH_SHIELD --output-dir ~/Downloads/hex/ESP8266/RDA5807_TFT_TOUCH_SHIELD  


echo "********************"
echo "Extending RDA5807 class example" 
echo "********************"

arduino-cli compile -b arduino:avr:uno ./RDA5807_01_SERIAL_MONITOR/RDA5807_90_EXTENDING_CLASS --output-dir ~/Downloads/hex/atmega/RDA5807_90_EXTENDING_CLASS  --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_01_SERIAL_MONITOR/RDA5807_91_EXTENDING_CLASS --output-dir ~/Downloads/hex/lgt8fx/RDA5807_91_EXTENDING_CLASS  --warnings all

