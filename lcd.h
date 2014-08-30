/**
@brief LCD control
@author Stephan Ruloff
@date 13.03.2014
*/

#ifndef _LCD_H_
#define _LCD_H_

#include "spi.h"
#include "screen.h"

#define LCD_X 132
#define LCD_Y 32

typedef struct
{
	Spi* mS0;
	uint8_t mRed;
	uint8_t mGreen;
	uint8_t mBlue;
} LcdSpi;

extern Rect ViewPortMax;

LcdSpi* LcdOpen(Spi* s0);
void LcdInit(LcdSpi* lcd);
void LcdCleanup(LcdSpi* lcd);
void LcdUninit(LcdSpi* lcd);
void LcdSetPos(LcdSpi* lcd, uint8_t page, uint8_t col);
void LcdCls(LcdSpi* lcd);
uint8_t LcdWriteImagePng(LcdSpi* lcd, const char *filename);
uint8_t LcdWriteImageScreen(LcdSpi* lcd, ScreenData *screen);
void LcdSetBgLight(LcdSpi* lcd, uint8_t r, uint8_t g, uint8_t b);
void LcdSetDebuglevel(uint8_t level);
void DebugWriteScreen(LcdSpi *lcd, uint8_t *screen, uint16_t sx, uint16_t sy);


#endif
