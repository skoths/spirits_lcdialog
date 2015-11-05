/**
@brief Configuration of I/O
@author Stephan Ruloff
@date 19.01.2015
*/

// Switches under the LCD
#define PIN_KEY_1 23
#define PIN_KEY_2 24
#define PIN_KEY_3 25

// LCD backlight
#define PIN_LED_RED 22
#define PIN_LED_GREEN 17
#define PIN_LED_BLUE 27

// Aux. pins for the LCD
#define PIN_LCD_nRST 4
#define PIN_LCD_A0 18

// SPI for the LCD
#define SPI_BUS 0
#define SPI_DEVICE 0

// Pull-Up style
// Raspberry Pi, pull-up by Broadcom registery
#define RPI_PULLUP

// Banana Pi, pull-up by GPIO system
//#define BPI_PULLUP

