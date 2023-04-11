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
echo "Arduino ATmega328 based board"
arduino-cli compile -b arduino:avr:nano ./RDA5807_00_CIRCUIT_TEST --output-dir ~/Downloads/hex/atmega/RDA5807_00_CIRCUIT_TEST -v --warnings all
arduino-cli compile -b arduino:avr:nano ./RDA5807_02_TFT_display --output-dir ~/Downloads/hex/atmega/RDA5807_02_TFT_display -v --warnings all
arduino-cli compile -b arduino:avr:nano ./RDA5807_04_NOKIA5110 --output-dir ~/Downloads/hex/atmega/RDA5807_04_NOKIA5110 -v --warnings all
arduino-cli compile -b arduino:avr:nano ./RDA5807_05_LCD16X02 --output-dir ~/Downloads/hex/atmega/RDA5807_05_LCD16X02 -v --warnings all


echo "Arduino LGT8FX based board"
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_00_CIRCUIT_TEST --output-dir ~/Downloads/hex/atmega/RDA5807_00_CIRCUIT_TEST -v --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_02_TFT_display --output-dir ~/Downloads/hex/atmega/RDA5807_02_TFT_display -v --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_04_NOKIA5110 --output-dir ~/Downloads/hex/atmega/RDA5807_04_NOKIA5110 -v --warnings all
arduino-cli compile -b lgt8fx:avr:328 ./RDA5807_05_LCD16X02 --output-dir ~/Downloads/hex/atmega/RDA5807_05_LCD16X02 -v --warnings all


# compiles ESP32 LCD16x2_ALL_IN_ONE
echo "ESP32"
echo "ESP32 LCD16x20"
arduino-cli compile --fqbn esp32:esp32:esp32-poe-iso ./RDA5807_05_LCD16X02_ESP32 --output-dir ~/Downloads/hex/ESP32/DEVM/RDA5807_05_LCD16X02_ESP32 -v --warnings all


# compile ESP8266
echo "ESP8266 OLED"
arduino-cli compile --fqbn esp8266:esp8266:generic  ./UNDER_CONSTRUCTION/OLED_ESP8266 --output-dir ~/Downloads/hex/ESP8266/OLED_ESP8266 -v --warnings all

echo "STM32"
# arduino-cli board -b stm32duino:STM32F1:genericSTM32F103C  details
arduino-cli compile  --fqbn stm32duino:STM32F1:genericSTM32F103C ./RDA5807_01_SERIAL_MONITOR/RDA5807_03_STM32 --output-dir ~/Downloads/hex/STM32/F1/RDA5807_03_STM32 -v --warnings all


# compiles ATTiny85
# echo "ATTINY84"
# arduino-cli board -b ATTinyCore:avr:attinyx4  details   
# arduino-cli compile --fqbn ATTinyCore:avr:attinyx4:millis=enabled  ./RDA5807_03_attimy84 --output-dir ~/Downloads/hex/ATTIMY85/RDA5807_03_attimy84
