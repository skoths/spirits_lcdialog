/**
@brief LCD control
@author Stephan Ruloff
@date 28.04.2014
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <gd.h>

#include "config.h"
#include "lcd.h"
#include "gpio.h"
#include "font.h"


static uint32_t gNextFileIndex = 0;
static uint8_t gDebuglevel = 0;

Rect ViewPortMax = {0, 0, LCD_X, LCD_Y};

void LcdSetDebuglevel(uint8_t level)
{
	gDebuglevel = level;
}

FILE* FindFreeFileName(void)
{
	char name[32];
	uint32_t index = gNextFileIndex;
	FILE *f;
	
	while (index < 1000) {
		sprintf(name, "IMG%03i.png", index);
		//printf("Name: %s\n", name);
		f = fopen(name, "rb");
		if (f) {
			fclose(f);
			index++;
		} else {
			f = fopen(name, "wb");
			if (f) {
				index++;
				gNextFileIndex = index;
				
				return f;
			}
		}
	}
	
	return NULL;
}

void DebugWriteScreen(LcdSpi *lcd, uint8_t *screen, uint16_t sx, uint16_t sy)
{
	gdImagePtr img;
	FILE *filePtr;
	/*int clBg;*/
	int clBlack;
	uint16_t x;
	uint16_t y;
	
	if (!lcd || !screen || !sx || !sy) {
		return;
	}
	img = gdImageCreate(sx, sy);
	if (!img) {
		return;
	}
	filePtr = FindFreeFileName();
	if (!filePtr) {
		return;
	}
	
	if (lcd->mRed == 0 && lcd->mGreen == 0 && lcd->mBlue == 0) {
		/*clBg = */gdImageColorAllocate(img, 0x80, 0x80, 0x80);
	} else {
		/*clBg = */gdImageColorAllocate(img, lcd->mRed, lcd->mGreen, lcd->mBlue);
	}
	clBlack = gdImageColorAllocate(img, 0, 0, 0);
	
	for (y = 0; y < sy; y++) {
		for (x = 0; x < sx; x++) {
			if (screen[(y * sx) + x]) {
				gdImageSetPixel(img, x, y, clBlack);
			}
		}
	}
	
	gdImagePng(img, filePtr);
	gdImageDestroy(img);
	fclose(filePtr);
}

void LcdWriteByte(Spi* s1, uint8_t c)
{
	uint8_t txBuf[1];
	uint8_t rxBuf[1];
	
	txBuf[0] = c;
	if (!SpiTransfer(s1, txBuf, rxBuf, 1)) {
		//printf("Err: %i\n", errno);
	}
}

LcdSpi* LcdOpen(Spi* s0)
{
	LcdSpi *lcd;

	lcd = (LcdSpi*)malloc(sizeof(LcdSpi));
	if (!lcd) {
		return NULL;
	}
	lcd->mS0 = s0;
	lcd->mRed = 0;
	lcd->mGreen = 0;
	lcd->mBlue = 0;
	
	// Export Kernel GPIO into Userspace
	GpioExport(PIN_LED_RED);
	GpioExport(PIN_LED_GREEN);
	GpioExport(PIN_LED_BLUE);
	
	GpioSetDirection(PIN_LED_RED, 1);
	GpioSetDirection(PIN_LED_GREEN, 1);
	GpioSetDirection(PIN_LED_BLUE, 1);
	
	return lcd;
}

void LcdInit(LcdSpi* lcd)
{
	uint8_t i;
	// 132x32
	uint8_t initSeqTx[] = {0x40, 0xA1, 0xC0, 0xA6, 0xA2, 0x2F, 0xF8, 0x00, 0x23, 0x81, 0x1F, 0xAC, 0x00, 0xAF};
	// 128x64
	//uint8_t initSeqTx[] = {0x40, 0xA1, 0xC0, 0xA6, 0xA2, 0x2F, 0xF8, 0x00, 0x27, 0x81, 0x10, 0xAC, 0x00, 0xAF};
	
	if (!lcd) {
		return;
	}
	
	GpioExport(PIN_LCD_nRST);
	GpioExport(PIN_LCD_A0);
	
	GpioSetDirection(PIN_LCD_nRST, 1);
	GpioSetDirection(PIN_LCD_A0, 1);

	GpioSetValue(PIN_LCD_nRST, 0);
	usleep(10000);
	GpioSetValue(PIN_LCD_nRST, 1);
	usleep(10000);	
	GpioSetValue(PIN_LCD_A0, 0);
	for (i = 0; i < sizeof(initSeqTx); i++) {
		LcdWriteByte(lcd->mS0, initSeqTx[i]);
		usleep(10000);
	}
}

void LcdCleanup(LcdSpi* lcd)
{
	if (!lcd) {
		return;
	}
	//GpioUnexport(PIN_LED_RED);
	//GpioUnexport(PIN_LED_GREEN);
	//GpioUnexport(PIN_LED_BLUE);
	
	// Don't release RST 
	//GpioUnexport(PIN_LCD_nRST);
	//GpioUnexport(PIN_LCD_A0);
	
	free(lcd);
}

void LcdUninit(LcdSpi* lcd)
{
	if (!lcd) {
		return;
	}
	GpioUnexport(PIN_LED_RED);
	GpioUnexport(PIN_LED_GREEN);
	GpioUnexport(PIN_LED_BLUE);
	
	GpioUnexport(PIN_LCD_nRST);
	GpioUnexport(PIN_LCD_A0);
	
	free(lcd);
}

void LcdSetPos(LcdSpi* lcd, uint8_t page, uint8_t col)
{
	if (!lcd) {
		return;
	}
	GpioSetValue(PIN_LCD_A0, 0);
	LcdWriteByte(lcd->mS0, 0xB0 + page);
	LcdWriteByte(lcd->mS0, 0x10 + ((col >> 4) & 0xf0));
	LcdWriteByte(lcd->mS0, 0x00 + (col & 0x0f));
}

void LcdCls(LcdSpi* lcd)
{
	uint8_t i;
	uint8_t j;
	
	if (!lcd) {
		return;
	}
	
	for (i = 0; i < 8; i++) {
		LcdSetPos(lcd, i, 0);
		
		GpioSetValue(PIN_LCD_A0, 1);
		for (j = 0; j < 132; j++) {
			LcdWriteByte(lcd->mS0, 0x00);
		}
	}
}

uint8_t LcdWriteImagePng(LcdSpi* lcd, const char *filename)
{
	uint16_t x = 100;
	uint16_t y = 100;
	uint16_t xm = 0;
	uint16_t ym = 0;
	uint16_t i = 0;
	int c;
	gdImagePtr img;
	FILE *filePtr;
	uint8_t b;
	
	if (!lcd) {
		return 1;
	}
	if (!filename) {
		return 2;
	}

	filePtr = fopen(filename, "rb");
	if (filePtr == NULL) {
		return 3;
	}
	img = gdImageCreateFromPng(filePtr);
	if (img == NULL) {
		return 4;
	}
	xm = gdImageSX(img);
	ym = gdImageSY(img);
	printf("%i x %i\n", xm, ym);
	
	if (xm != 132 || ym != 32) {
		return 5;
	}
	
	for (y = 0; y < ym; y += 8) {
		LcdSetPos(lcd, y / 8, 0);
		
		GpioSetValue(PIN_LCD_A0, 1);
		for (x = 0; x < xm; x++) {
			b = 0;
			for (i = 0; i < 8; i++) {
				c = gdImageGetPixel(img, x, y + i);
				if (img->red[c] < 0x7f || img->green[c] < 0x7f || img->blue[c] < 0x7f) {
				//if (c) {
					b |= (1 << i);
				}
			}
			LcdWriteByte(lcd->mS0, b);
		}
	}
	fclose(filePtr);
	gdImageDestroy(img);
	
	return 0;
}

uint8_t LcdWriteImageScreen(LcdSpi* lcd, ScreenData *screen)
{
	uint8_t x = 0;
	uint8_t y = 0;
	uint16_t i = 0;
	uint8_t c;
	uint8_t b;
	uint8_t xScreen;
	uint8_t yScreen;
	
	if (!lcd) {
		return 1;
	}
	if (!screen) {
		return 2;
	}
	xScreen = screen->mX;
	yScreen = screen->mY;
	
	for (y = 0; y < yScreen; y += 8) {
		LcdSetPos(lcd, y / 8, 0);
		
		GpioSetValue(PIN_LCD_A0, 1);
		for (x = 0; x < xScreen; x++) {
			b = 0;
			for (i = 0; i < 8; i++) {
				c = screen->mData[((y + i) * xScreen) + x];
				if (c) {
					b |= (1 << i);
				}
			}
			LcdWriteByte(lcd->mS0, b);
		}
	}
	if (gDebuglevel >= 100) {
		DebugWriteScreen(lcd, screen->mData, xScreen, yScreen);
	}
	
	return 0;
}

void LcdSetBgLight(LcdSpi* lcd, uint8_t r, uint8_t g, uint8_t b)
{
	if (!lcd) {
		return;
	}
	lcd->mRed = r ? 0xFF : 0x00;
	lcd->mGreen = g ? 0xFF : 0x00;
	lcd->mBlue = b ? 0xFF : 0x00;	
	if (r) {
		GpioSetValue(PIN_LED_RED, 1);
	} else {
		GpioSetValue(PIN_LED_RED, 0);
	}
	if (g) {
		GpioSetValue(PIN_LED_GREEN, 1);
	} else {
		GpioSetValue(PIN_LED_GREEN, 0);
	}
	if (b) {
		GpioSetValue(PIN_LED_BLUE, 1);
	} else {
		GpioSetValue(PIN_LED_BLUE, 0);
	}
}
