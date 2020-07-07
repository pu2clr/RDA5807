
/**
 * @mainpage PU2CLR rda Arduino Library
 * @brief PU2CLR rda Arduino Library implementation. <br>
 * @details This library works with I2C protocol and can provide an easier interface for controlling the RDA5807M devices.<br>
 * @details This is an Arduino library was implemented based on RDA5807M - SINGLE-CHIP BROADCAST FMRADIO TUNER document from RDA microeletronics.
 *
 * @details IMPORTANT: 
 * @details The RDA5807M can be accessed by using two I2C bus address.
 * @details If you need to access a specific register, use the 0x11 buss adress. 
 * @details If you need to access a set o register, use the 0x10 buss adress. 
 * 
 * @see RDA5807M - SINGLE-CHIP BROADCAST FMRADIO TUNER; page 5.
 * @see the methods setRegister, getRegister and getStatusRegisters
 *  
 * This library can be freely distributed using the MIT Free Software model.
 * Copyright(c) 2020 Ricardo Lima Caratti.
 * Contact : pu2clr @gmail.com
 * 
 */

#include <Arduino.h>
#include <Wire.h>

#define MAX_DELAY_AFTER_OSCILLATOR 500  // Max delay after the crystal oscilator becomes active


#define I2C_ADDR_DIRECT_ACCESS  0x11    //!< Can be used to access a given register at a time.
#define I2C_ADDR_FULL_ACCESS    0x10    //!< Can be used to access a set of register at a time.


#define OSCILLATOR_TYPE_CRYSTAL  0  //!< Crystal
#define OSCILLATOR_TYPE_REFCLK   1  //!< Reference clock

#define CLOCK_32K                0  //!< 32.768kHz
#define CLOCK_12M                1  //!< 12Mhz
#define CLOCK_13M                2  //!< 13Mhz
#define CLOCK_19_2M              3  //!< 19.2Mhz
#define CLOCK_24M                5  //!< 24Mhz 
#define CLOCK_26M                6  //!< 26Mhz 
#define CLOCK_38_4M              7  //!< 38.4Mhz 

#define RDS_STANDARD     0  //!< RDS Mode.
#define RDS_VERBOSE      1  //!< RDS Mode.

#define RDA_FM_BAND_USA_EU       0  //!< 87.5–108 MHz (US / Europe, Default)
#define RDA_FM_BAND_JAPAN_WIDE   1  //!< 76–91 MHz (Japan wide band)
#define RDA_FM_BAND_WORLD        2  //!< 76–108 MHz (world wide)
#define RDA_FM_BAND_SPECIAL      3  //!< 65 –76 MHz(East Europe) or 50 - 65MHz(see bit 9 of gegister 0x06)

#define RDA_SEEK_WRAP  0     //!< Wrap at the upper or lower band limit and continue seeking
#define RDA_SEEK_STOP  1     //!< Stop seeking at the upper or lower band limit
#define RDA_SEEK_DOWN  0     //!< Seek Up
#define RDA_SEEK_UP    1     //!< Seek Down

#define REG00 0x00
#define REG02 0x02
#define REG03 0x03
#define REG04 0x04
#define REG05 0x05
#define REG06 0x06
#define REG07 0x07
#define REG0A 0x0A
#define REG0B 0x0B
#define REG0C 0x0C
#define REG0D 0x0D
#define REG0E 0x0E
#define REG0F 0x0F

#define SH_REG0A 0 // Shadow array position for register 0x0A
#define SH_REG0B 1 // Shadow array position for register 0x0B
#define SH_REG0C 2 // Shadow array position for register 0x0C - RDS Block A
#define SH_REG0D 3 // Shadow array position for register 0x0D - RDS Block B
#define SH_REG0E 4 // Shadow array position for register 0x0E - RDS Block C
#define SH_REG0F 5 // Shadow array position for register 0x0F - RDS Block D

/**
 * @defgroup GA01 Union, Structure and Defined Data Types
 * @brief   rda Defined Data Types
 * @details Defined Data Types is a way to represent the rda registers information
 * @details The information shown here was extracted from Datasheet:
 * @details rda stereo FM digital tuning radio documentation.
 */

/**
 * @ingroup GA01
 * @brief Register 0x00
 *
 */
    typedef union {
    struct {
        uint8_t CHIPID: 8; //!< Chip ifdef 
        uint8_t DUMMY: 8;
    } refined;
    uint16_t raw;
} rda_reg00;

/**
 * @ingroup GA01
 * @brief Register 0x01 - Dummy
 * @details It is not documented by the RDA. 
 */
typedef union
{
    struct
    {
        uint8_t lowByte; 
        uint8_t highByte;
    } refined;
    uint16_t raw;
} rda_reg01;

/**
 * @ingroup GA01
 * @brief Register 0x02 - Basic setup: RESET configuration; 
 * @details Clock type configuration; seek operation; Sterio/Mono; Bass;  and Audio configuration 
 * @details The RDA5807M is RESET itself when power up. You also can use soft reset by triggering SOFT_RESET bit to 1. 
 * @details Seek begins in the direction specified by SEEKUP and ends when a channel is found, or the entire band has been searched.
 * @details The SEEK bit is set low and the STC bit is set high when the seek operation completes.
 * @details RCLK NON-CALIBRATE MODE. if 0=RCLK clock is always supply; if 1=RCLK clock is not always supply when FM work ( when 1, RDA5807M can’t directly support -20 °C ~70 °C temperature. 
 * @details Only suppory ±20°C temperature swing from tune point).
 * 
 *  Clock CLK_MODE table
 * 
 *  | Value | Clock     | 
 *  | ----- | --------- |
 *  | 000   | 32.768kHz | 
 *  | 001   | 12Mhz     | 
 *  | 101   | 24Mhz     | 
 *  | 010   | 13Mhz     | 
 *  | 110   | 26Mhz     | 
 *  | 011   | 19.2Mhz   | 
 *  | 111   | 38.4Mhz   |
 */
typedef union {
    struct
    {
        uint8_t ENABLE : 1;         //!< Power Up Enable; 0 = Disabled; 1 = Enabled
        uint8_t SOFT_RESET : 1;     //!< Soft reset; If 0, not reset; If 1, reset.
        uint8_t NEW_METHOD : 1;     //!< New Demodulate Method Enable, can improve 0 the receive sensitivity about 1dB.
        uint8_t RDS_EN : 1;         //!< RDS/RBDS enable; If 1, rds/rbds enable
        uint8_t CLK_MODE : 3;       //!<  See table above
        uint8_t SKMODE : 1;         //!< Seek Mode; 0 = wrap at the upper or lower band limit and continue seeking; 1 = stop seeking at the upper or lower band limit
        uint8_t SEEK : 1;           //!< Seek; 0 = Disable stop seek; 1 = Enable;
        uint8_t SEEKUP : 1;         //!< Seek Up; 0 = Seek down; 1 = Seek up
        uint8_t RCLK_DIRECT_IN : 1; //!< RCLK clock use the directly input mode. 1 = enable
        uint8_t NON_CALIBRATE : 1;  //!< 0=RCLK clock is always supply; 1=RCLK clock is not always supply when FM work
        uint8_t BASS : 1;           //!< Bass Boost; 0 = Disabled; 1 = Bass boost enabled
        uint8_t MONO : 1;           //!< Mono Select; 0 = Stereo; 1 = Force mono
        uint8_t DMUTE : 1;          //!< Mute Disable; 0 = Mute; 1 = Normal operation
        uint8_t DHIZ : 1;           //!< Audio Output High-Z Disable; 0 = High impedance; 1 = Normal operation
    } refined;
    uint16_t raw;
} rda_reg02;

/**
 * @ingroup GA01
 * @brief Register 0x03
 * @details Receiver configuratio
 * @details The tune operation begins when the TUNE bit is set high. The STC bit is set high when the tune operation completes.
 * @details The tune bit is reset to low automatically when the tune operation completes
 * 
 * Channel space table
 * 
 * | Value | Description | 
 * | ----- | ----------- | 
 * | 00    | 100KHz      |
 * | 01    | 200KHz      | 
 * | 10    | 50KHz       | 
 * | 11    | 25KHz       | 
 * 
 *  
 * FM band table 
 * 
 * | Value | Description                 | 
 * | ----- | --------------------------- | 
 * | 00    | 87–108 MHz (US/Europe)      |
 * | 01    | 76–91 MHz (Japan)           | 
 * | 10    | 76–108 MHz (world wide)     | 
 * | 11    | 65 –76 MHz (East Europe) or 50-65MHz (see bit 9 of gegister 0x06) | 
 * 
 *  Channel select table 
 * 
 * | BAND   | Description                                         | 
 * | ------ | --------------------------------------------------  | 
 * |  0     | Frequency = Channel Spacing (kHz) x CHAN+ 87.0 MHz  |
 * | 1 or 2 | Frequency = Channel Spacing (kHz) x CHAN + 76.0 MHz | 
 * | 3      | Frequency = Channel Spacing (kHz) x CHAN + 65.0 MHz | 
 * IMPORTANT: CHAN is updated after a seek operation. 
 * 
 */
typedef union {
    struct
    {
        uint16_t SPACE: 2;        //!< See Channel space table above
        uint16_t BAND: 2;         //!< Seet band table above
        uint16_t TUNE : 1;        //!< Tune; 0 = Disable; 1 = Enable
        uint16_t DIRECT_MODE : 1; //!< Directly Control Mode, Only used when test
        uint16_t CHAN : 10;       //!< Channel Select.
    } refined;
    uint16_t raw;
} rda_reg03;

/**
 * @ingroup GA01
 * @brief Register 0x04
 * @details Receiver properties 
 * @details Volume scale is logarithmic When 0000, output mute and output impedance is very large
 * @details Setting STCIEN = 1 will generate a low pulse on GPIO2 when the interrupt occurs.
 */
typedef union {
    struct
    {
        uint8_t GPIO1 : 2;        //!< General Purpose I/O 1. when gpio_sel=01; 00 = High impedance; 01 = Reserved; 10 = Low; 11 = High
        uint8_t GPIO2 : 2;        //!< General Purpose I/O 2. when gpio_sel=01; 00 = High impedance; 01 = Reserved; 10 = Low; 11 = High
        uint8_t GPIO3 : 2;        //!< General Purpose I/O 1. when gpio_sel=01; 00 = High impedance; 01 = Mono/Stereo indicator (ST); 10 = Low; 11 = High
        uint8_t I2S_ENABLE : 1;   //!< I2S enable; 0 = disabled; 1 = enabled.
        uint8_t RSVD1 : 1;
        uint8_t AFCD : 1;         //!< AFC disable; If 0, afc work; If 1, afc disabled.
        uint8_t SOFTMUTE_EN  : 1; //!< If 1, softmute enable.
        uint8_t RDS_FIFO_CLR : 1; //!< 1 = clear RDS fifo
        uint8_t DE : 1;           //!< De-emphasis; 0 = 75 μs; 1 = 50 μs
        uint8_t RDS_FIFO_EN : 1;  //!< 1 = RDS fifo mode enable.
        uint8_t RBDS : 1;         //!< 1 = RBDS mode enable; 0 = RDS mode only
        uint8_t STCIEN : 1;       //!< Seek/Tune Complete Interrupt Enable; 0 = Disable Interrupt; 1 = Enable Interrupt;
        uint8_t RSVD2 : 1;
    } refined;
    uint16_t raw;
} rda_reg04;

/**
 * @ingroup GA01
 * @brief Register 0x05
 */
typedef union {
    struct
    {
        uint8_t VOLUME : 4;         //!< DAC Gain Control Bits (Volume); 0000 = min volume; 1111 = max volume.
        uint8_t LNA_ICSEL_BIT : 2;  //!< Lna working current bit: 00=1.8mA; 01=2.1mA; 10=2.5mA; 11=3.0mA.
        uint8_t LNA_PORT_SEL : 2;   //!< LNA input port selection bit: 00: no input; 01: LNAN; 10: LNAP; 11: dual port input
        uint8_t SEEKTH : 4;         //!< Seek SNR Threshold value
        uint8_t RSVD2  : 1;
        uint8_t SEEK_MODE : 2;      //!< Default value is 00; When = 10, will add the RSSI seek mode
        uint8_t INT_MODE : 1;       //!< If 0, generate 5ms interrupt; If 1, interrupt last until read reg0CH action occurs.
    } refined;
    uint16_t raw;
} rda_reg05;

/**
 * @ingroup GA01
 * @brief Register 0x06
 * 
 * I2S_SW_CNT values table
 * 
 * |  Value  | Description        | 
 * | ------- | ------------------ | 
 * | 0b1000  | WS_STEP_48         | 
 * | 0b0111  | WS_STEP=44.1kbps   |
 * | 0b0110  | WS_STEP=32kbps     | 
 * | 0b0101  | WS_STEP=24kbps     | 
 * | 0b0100  | WS_STEP=22.05kbps  | 
 * | 0b0011  | WS_STEP=16kbps     | 
 * | 0b0010  | WS_STEP=12kbps     | 
 * | 0b0001  | WS_STEP=11.025kbps | 
 * | 0b0000  | WS_STEP=8kbps      | 
 */
typedef union {
    struct
    {
        uint8_t R_DELY : 1;       //!< If 1, R channel data delay 1T.
        uint8_t L_DELY : 1;       //!< If 1, L channel data delay 1T.
        uint8_t SCLK_O_EDGE : 1;  //!< If 1, invert sclk output when as master.
        uint8_t SW_O_EDGE : 1;    //!< If 1, invert ws output when as master.
        uint8_t I2S_SW_CNT : 4;   //!< Only valid in master mode. See table above
        uint8_t WS_I_EDGE : 1;    //!< If 0, use normal ws internally; If 1, inverte ws internally.
        uint8_t DATA_SIGNED : 1;  //!< If 0, I2S output unsigned 16-bit audio data. If 1, I2S output signed 16-bit audio data.
        uint8_t SCLK_I_EDGE : 1;  //!< If 0, use normal sclk internally;If 1, inverte sclk internally.
        uint8_t WS_LR : 1;        //!< Ws relation to l/r channel; If 0, ws=0 ->r, ws=1 ->l; If 1, ws=0 ->l, ws=1 ->r.
        uint8_t SLAVE_MASTER : 1; //!< I2S slave or master; 1 = slave; 0 = master.
        uint8_t OPEN_MODE : 2;    //!< Open reserved register mode;  11=open behind registers writing function others: only open behind registers reading function.
        uint8_t RSVD : 1;
    } refined;
    uint16_t raw;
} rda_reg06;

/**
 * @ingroup GA01
 * @brief Register 0x07
 */
typedef union {
    struct
    {
        uint8_t FREQ_MODE : 1;    //!< If 1, then freq setting changed. Freq = 76000(or 87000) kHz + freq_direct (08H) kHz.
        uint8_t SOFTBLEND_EN : 1; //!< If 1, Softblend enable
        uint8_t SEEK_TH_OLD : 6;  //!< Seek threshold for old seek mode, Valid when Seek_Mode=001
        uint8_t RSVD1 : 1;
        uint8_t MODE_50_60 : 1;   //!< 1 = 65~76 MHz;  0 = 50~76MHz
        uint8_t TH_SOFRBLEND : 5; //!< Threshold for noise soft blend setting, unit 2dB (default 0b10000).
        uint8_t RSVD2 : 1;
    } refined;
    uint16_t raw;
} rda_reg07;

/**
 * @ingroup GA01
 * @brief Register 0x08 - Direct Frequency 
 * @details Valid when freq_mode = 1
 * @details Freq = 7600(or 8700) kHz + freq_direct (08H) kHz.
 * @details Value to be stores is frequency - 7600 or 8700
 */
typedef union
{
   struct
    {
      uint8_t lowByte;
      uint8_t highByte;   
    } refined;
    uint16_t raw;
} rda_reg08;


/**
 * @ingroup GA01
 * @brief Register 0x0A - Device current status
 * @details The seek fail flag (SF) is set when the seek operation fails to find a channel with an RSSI level greater than SEEKTH[5:0].
 * @details The seek/tune complete (STC) flag is set when the seek or tune operation completes.
 * 
 * 
 *  Channel table 
 * 
 * | BAND   | Description                                         | 
 * | ------ | --------------------------------------------------  | 
 * |  0     | Frequency = Channel Spacing (kHz) x CHAN+ 87.0 MHz  |
 * | 1 or 2 | Frequency = Channel Spacing (kHz) x CHAN + 76.0 MHz | 
 * | 3      | Frequency = Channel Spacing (kHz) x CHAN + 65.0 MHz | 
 * 
 */
typedef union {
    struct
    {
        uint16_t READCHAN : 10;  //!< See Channel table . See table above
        uint16_t ST : 1;         //!< Stereo Indicator; 0 = Mono; 1 = Stereo
        uint16_t BLK_E : 1;      //!< When RDS enable: 1 = Block E has been found; 0 = no Block E has been found
        uint16_t RDSS : 1;       //!< RDS Synchronization; 0 = RDS decoder not synchronized(default); 1 = RDS decoder synchronized; Available only in RDS Verbose mode
        uint16_t SF : 1;         //!< Seek Fail. 0 = Seek successful; 1 = Seek failure;
        uint16_t STC : 1;        //!< Seek/Tune Complete. 0 = Not complete; 1 = Complete;
        uint16_t RDSR : 1;       //!< RDS ready; 0 = No RDS/RBDS group ready(default); 1 = New RDS/RBDS group ready.
    } refined;
    uint16_t raw;
} rda_reg0a;

/**
 * @ingroup GA01
 * @brief Register 0x0B
 * @details Current RDS and device status
 * 
 * Errors Level table (Block Errors Level of RDS_DATA_0 and RDS_DATA_1. 
 * Always read as Errors Level of RDS BLOCKs A and B (in RDS mode ) or E (in RBDS mode when ABCD_E flag is 1).
 * 
 * | Value |  Description | 
 * | ----- |  ----------- | 
 * |  00   |  0 errors requiring correction | 
 * |  01   |  1~2 errors requiring correction |
 * |  10   |  3~5 errors requiring correction | 
 * |  10   |  6+ errors or error in checkword, correction not possible |  
 * 
 */
typedef union {
    struct
    {
        uint8_t BLERB : 2;      //!< Block Errors Level of RDS_DATA_1
        uint8_t BLERA : 2;      //!< Block Errors Level of RDS_DATA_0
        uint8_t ABCD_E : 1;     //!< 1 = the block id of register 0cH,0dH,0eH,0fH is E;  0 = the block id of register 0cH, 0dH, 0eH,0fH is A, B, C, D
        uint8_t RSVD1  : 2;
        uint8_t FM_READY : 1;   //!< 1=ready; 0=not ready.
        uint8_t FM_TRUE : 1;    //!< 1 = the current channel is a station; 0 = the current channel is not a station.
        uint8_t RSSI : 7;       //!< RSSI; 000000 = min; 111111 = max; RSSI scale is logarithmic.
    } refined;
    uint16_t raw;
} rda_reg0b;


/**
 * @ingroup GA01
 * @brief Register 0x0C
 * @details BLOCK A ( in RDS mode) or BLOCK E (in RBDS mode when ABCD_E flag is 1)
 *
 */
typedef union {
    struct {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t RDSA; //!< BLOCK A ( in RDS mode) or BLOCK E (in RBDS mode when ABCD_E flag is 1)
} rda_reg0c;

/**
 * @ingroup GA01
 * @brief Register 0x0D
 * @details BLOCK B ( in RDS mode) or BLOCK E (in RBDS mode when ABCD_E flag is 1)
 *
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t RDSB; 
} rda_reg0d;


/**
 * @ingroup GA01
 * @brief Register 0x0E
 * @details BLOCK C ( in RDS mode) or BLOCK E (in RBDS mode when ABCD_E flag is 1)
 *
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t RDSC;
} rda_reg0e;

/**
 * @ingroup GA01
 * @brief Register 0x0F
 * @details BLOCK D ( in RDS mode) or BLOCK E (in RBDS mode when ABCD_E flag is 1)
 *
 */
typedef union
{
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t RDSD;
} rda_reg0f;

/**
 * @ingroup GA01
 * @brief RDS Block B data type
 *
 * @details For GCC on System-V ABI on 386-compatible (32-bit processors), the following stands:
 *
 * 1) Bit-fields are allocated from right to left (least to most significant).
 * 2) A bit-field must entirely reside in a storage unit appropriate for its declared type.
 *    Thus a bit-field never crosses its unit boundary.
 * 3) Bit-fields may share a storage unit with other struct/union members, including members that are not bit-fields.
 *    Of course, struct members occupy different parts of the storage unit.
 * 4) Unnamed bit-fields' types do not affect the alignment of a structure or union, although individual
 *    bit-fields' member offsets obey the alignment constraints.
 *
 * @see also https://en.wikipedia.org/wiki/Radio_Data_System
 */
typedef union {
    struct
    {
        uint8_t address : 2;            // Depends on Group Type and Version codes. If 0A or 0B it is the Text Segment Address.
        uint8_t DI : 1;                 // Decoder Controll bit
        uint8_t MS : 1;                 // Music/Speech
        uint8_t TA : 1;                 // Traffic Announcement
        uint8_t programType : 5;        // PTY (Program Type) code
        uint8_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint8_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint8_t groupType : 4;          // Group Type code.
    } group0;
    struct
    {
        uint8_t address : 4;            // Depends on Group Type and Version codes. If 2A or 2B it is the Text Segment Address.
        uint8_t textABFlag : 1;         // Do something if it chanhes from binary "0" to binary "1" or vice-versa
        uint8_t programType : 5;        // PTY (Program Type) code
        uint8_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint8_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint8_t groupType : 4;          // Group Type code.
    } group2;
    struct
    {
        uint8_t content : 4;            // Depends on Group Type and Version codes.
        uint8_t textABFlag : 1;         // Do something if it chanhes from binary "0" to binary "1" or vice-versa
        uint8_t programType : 5;        // PTY (Program Type) code
        uint8_t trafficProgramCode : 1; // (TP) => 0 = No Traffic Alerts; 1 = Station gives Traffic Alerts
        uint8_t versionCode : 1;        // (B0) => 0=A; 1=B
        uint8_t groupType : 4;          // Group Type code.
    } refined;
    uint16_t blockB;
} rds_blockb;

/**
 * @ingroup GA01
 * Group RDS type 4A ( RDS Date and Time)
 * When group type 4A is used by the station, it shall be transmitted every minute according to EN 50067.
 * This Structure uses blocks 2,3 and 5 (B,C,D)
 *
 * ATTENTION:
 * To make it compatible with 8, 16 and 32 bits platforms and avoid Crosses boundary, it was necessary to
 * split minute and hour representation.
 */
typedef union {
    struct
    {
        uint8_t offset : 5;       // Local Time Offset
        uint8_t offset_sense : 1; // Local Offset Sign ( 0 = + , 1 = - )
        uint8_t minute1 : 2;      // UTC Minutes - 2 bits less significant (void “Crosses boundary”).
        uint8_t minute2 : 4;      // UTC Minutes - 4 bits  more significant  (void “Crosses boundary”)
        uint8_t hour1 : 4;        // UTC Hours - 4 bits less significant (void “Crosses boundary”)
        uint8_t hour2 : 1;        // UTC Hours - 4 bits more significant (void “Crosses boundary”)
        uint32_t mjd : 17;        // Modified Julian Day Code
    } refined;
    uint8_t raw[6];
} rds_date_time;

/**
 * @ingroup GA01
 * @brief Converts 16 bits word to two bytes
 */
typedef union {
    struct
    {
        uint8_t lowByte;
        uint8_t highByte;
    } refined;
    uint16_t raw;
} word16_to_bytes;

/**
 * @ingroup GA01
 * @brief KT0915 Class
 * @details This class implements all functions that will help you to control the KT0915 devices.
 *
 * @author PU2CLR - Ricardo Lima Caratti
 */
class RDA5807 {

    private:

        uint16_t shadowStatusRegisters[6]; //!< shadow status registers
        uint16_t shadowRegisters[9];       //!< shadow configuration registers

        // Shadow device configuration register references  (writable registers)
        rda_reg00 *reg00 = (rda_reg00 *)&shadowRegisters[0]; // REG01;
        rda_reg01 *reg01 = (rda_reg01 *)&shadowRegisters[1]; // REG01;
        rda_reg02 *reg02 = (rda_reg02 *)&shadowRegisters[2]; // REG02;
        rda_reg03 *reg03 = (rda_reg03 *)&shadowRegisters[3]; // REG03;
        rda_reg04 *reg04 = (rda_reg04 *)&shadowRegisters[4]; // REG04;
        rda_reg05 *reg05 = (rda_reg05 *)&shadowRegisters[5]; // REG05;
        rda_reg06 *reg06 = (rda_reg06 *)&shadowRegisters[6]; // REG06;
        rda_reg07 *reg07 = (rda_reg07 *)&shadowRegisters[7]; // REG07;
        rda_reg08 *reg08 = (rda_reg08 *)&shadowRegisters[8]; // REG08;

        // Shadow device status register references (read only registers)
        rda_reg0a *reg0a = (rda_reg0a *)&shadowStatusRegisters[0]; // SH_REG0A; 
        rda_reg0b *reg0b = (rda_reg0b *)&shadowStatusRegisters[1]; // SH_REG0B;
        rda_reg0c *reg0c = (rda_reg0c *)&shadowStatusRegisters[2]; // SH_REG0C;
        rda_reg0d *reg0d = (rda_reg0d *)&shadowStatusRegisters[3]; // SH_REG0D;
        rda_reg0e *reg0e = (rda_reg0e *)&shadowStatusRegisters[4]; // SH_REG0E;
        rda_reg0f *reg0f = (rda_reg0f *)&shadowStatusRegisters[5]; // SH_REG0F;


        uint16_t startBand[4] = {8700, 7600, 7600, 6500};
        uint16_t endBand[4] = {10800, 9100, 10800, 7600}; //!< End FM band limit
        uint16_t fmSpace[4] = {100, 200, 50, 25};

        char rds_buffer2A[65]; //!<  RDS Radio Text buffer - Program Information
        char rds_buffer2B[33]; //!<  RDS Radio Text buffer - Station Informaation
        char rds_buffer0A[9];  //!<  RDS Basic tuning and switching information (Type 0 groups)
        char rds_time[20];     //!<  RDS date time received information


    protected:
        int deviceAddressDirectAccess = I2C_ADDR_DIRECT_ACCESS;
        int deviceAddressFullAccess = I2C_ADDR_FULL_ACCESS;

        int resetPin;
        uint16_t currentFrequency;
        uint8_t currentFMBand = 0;
        uint8_t currentFMSpace = 0;
        uint8_t currentVolume = 0;
    
        int gpio1Control = -1;      //!< Can be used to add control to the  system via GPIO1 pin
        int gpio2Control = -1;      //!< Can be used to add control to the  system via GPIO2 pin
        int gpio3Control = -1;      //!< Can be used to add control to the  system via GPIO3 pin

        uint8_t clockType = CLOCK_32K;
        uint8_t oscillatorType = OSCILLATOR_TYPE_CRYSTAL;

        uint16_t maxDelayAftarCrystalOn = MAX_DELAY_AFTER_OSCILLATOR;

        public :

            /**
             * @brief Set the Delay After Crystal On (default 500ms)
             *
             * @param ms_value  Value in milliseconds
             */
            inline void setDelayAfterCrystalOn(uint8_t ms_value) { maxDelayAftarCrystalOn = ms_value; };

            /**
             * @ingroup GA03
             * @brief Sets alternatives I2C bus address 
             * @details You do not need use this function on RDA5807M
             * @param directAccess 
             * @param fullAccess 
             */
            inline void setI2CBusAddrs(int directAccess, int fullAccess) {
                this->deviceAddressDirectAccess = directAccess;
                this->deviceAddressFullAccess = fullAccess;
            }

            void getStatusRegisters();
            void *getStatus(uint8_t reg);
            void setRegister(uint8_t reg, uint16_t value);

            void setGpio(uint8_t gpioPin, uint8_t gpioSetup = 0, int mcuPin = -1);

            void waitAndFinishTune();

            void softReset();

            void powerUp();
            void powerDown();

            void setup(uint8_t clock_type = CLOCK_32K, uint8_t oscillator_type = OSCILLATOR_TYPE_CRYSTAL);

            void setFrequency(uint16_t frequency);


            uint16_t getFrequency();
            void setFrequencyUp();
            void setFrequencyDown();

            uint16_t getRealFrequency();
            uint16_t getRealChannel();
            void setChannel(uint16_t channel);
            void seek(uint8_t seek_mode, uint8_t direction);
            void seek(uint8_t seek_mode, uint8_t direction, void (*showFunc)());
            void setSeekThreshold(uint8_t value);

            void setBand(uint8_t band = 1);
            void setSpace(uint8_t space = 0);
            int getRssi();

            void setSoftmute(bool value);

            void setMono(bool value);
            bool isStereo();

            uint8_t getDeviceId();

            void setMute(bool value);
            void setVolume(uint8_t value);
            uint8_t getVolume();
            void setVolumeUp();
            void setVolumeDown();

            void setFmDeemphasis(uint8_t de);

            //******** RDS methods 
            void setRDS(bool value);
            void setRBDS(bool value);
            void setRdsFifo(bool value);
            void clearRdsFifo();

            bool getRdsReady();
            uint8_t getRdsFlagAB(void);
            uint8_t getRdsVersionCode(void);
            uint16_t getRdsGroupType();
            uint8_t getRdsProgramType(void);
            void getNext2Block(char *c);
            void getNext4Block(char *c);
            char *getRdsText(void);
            char *getRdsText0A(void);
            char *getRdsText2A(void);
            char *getRdsText2B(void);
            char *getRdsTime();
            bool getRdsSync();
            uint8_t getBlockId();
            uint8_t getErrorBlockB();
            bool hasRdsInfo();
};
