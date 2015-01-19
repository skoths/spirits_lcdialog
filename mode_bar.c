/**
@brief Mode: Bar (Progress-Bar, Percent-Bar)
@author Stephan Ruloff
@date 15.08.2014
*/

/*
This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; in version 2 only
of the License.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "lcd.h"
#include "screen.h"
#include "lcdialog.h"
#include "conv_str.h"

double GetPercent(char *str)
{
	double percent;
	int value;
	
	value = atoi(str);
	if (value < 0) {
		percent = 0.0;
	} else if (value > 100) {
		percent = 100.0;
	} else {
		percent = (double)value;
	}
	
	return percent;
}

void TrimCr(char *str)
{
	size_t len;
	
	if (!str) {
		return;
	}
	len = strlen(str);
	while (len) {
		if (str[len - 1] == '\n') {
			str[len - 1] = 0;
			len--;
		} else {
			break;
		}
	}
}

int Progress(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg, int optind, int argc, char **argv)
{
	int localEnd = 0;
	int result = EXITCODE_ERROR;
	ScreenData *screen;
	char output[30];
	char input[32];
	char *inputIso;
	int posY;
	int fontHeight;
	int barHeight;
	double percent;
	int value;
	int len;
	FILE *fIn;
	uint8_t mode = 0;
	int lineCount = 0;
	char *res;
	Rect r;
	Rect viewPort;
	
	if (!lcd || !f || !text || !screenBg) {
		return EXITCODE_ERROR;
	}
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return EXITCODE_ERROR;
	}
	
	len = argc - optind;
	if (len == 1) {
		value = atoi(argv[optind]);
		if (value < 0) {
			value = 0;
		}
		if (value > 100) {
			value = 100;
		}
	} else {
		return EXITCODE_ERROR;
	}
	percent = (double)value;
	CatchCtrlC();
	fIn = stdin;
	
	sprintf(output, "%i", (int)percent);
	fontHeight = FontStringY(f->mText, output);
	posY = 31 - fontHeight;
	barHeight = fontHeight + 2;
		
	ScreenCopy(screen, screenBg);
	viewPort.mX = 0;
	viewPort.mY = 0;
	viewPort.mHeight = LCD_Y - barHeight;
	viewPort.mWidth = LCD_X;
	FontScreenStringVp(f->mText, screen, viewPort, text, 0, JUSTIFY_LEFT);
	
	// Frame
	r.mX = 0;
	r.mY = posY - 1;
	r.mWidth = LCD_X;
	r.mHeight = barHeight;
	ScreenRect(screen, r, 0);
	// Bar
	r.mX = 1;
	r.mY = posY;
	r.mWidth = (double)(LCD_X - 2) * (percent / 100.0);
	r.mHeight = fontHeight;
	ScreenRect(screen, r, 1);
	
	FontScreenString(f->mText, screen, BUTTON_POS_CENTER, posY, output, 0, JUSTIFY_CENTER);
	LcdWriteImageScreen(lcd, screen);
		
	while (!gEnde && !localEnd && !feof(fIn)) {
		//FontScreenString(f->mText, screen, LCD_X, LCD_Y - barHeight, 0, 0, text, 0, JUSTIFY_LEFT);
		
		res = fgets(input, sizeof(input) - 1, fIn);
		if (!res) {
			if (feof(fIn)) {
				result = EXITCODE_OK;
			}
			break;
		}
		input[sizeof(input) - 1] = 0;
		TrimCr(input);
		//printf("Inp: '%s'\n", input);
		if (strstr(input, "XXX") == input) {
			if (mode == 0) {
				ScreenCopy(screen, screenBg);
				lineCount = 0;
				mode = 1;
			} else {
				mode = 0;
			}
		} else {
			if (mode == 0) {
				percent = GetPercent(input);
			} else {
				if (lineCount == 0) {
					percent = GetPercent(input);
				} else {
					inputIso = StrdupConv(input);
					fontHeight = FontStringY(f->mText, inputIso);
					posY = fontHeight * (lineCount - 1);
					if (lineCount > 1) {
						posY++;
					}
					viewPort.mX = 0;
					viewPort.mY = posY;
					viewPort.mHeight = LCD_Y - barHeight;
					viewPort.mWidth = LCD_X - viewPort.mY;
					FontScreenStringVp(f->mText, screen, viewPort, inputIso, 0, JUSTIFY_LEFT);
					free(inputIso);
				}
				lineCount++;	
			}
		}
		//printf("%i %i %i %i\n", (int)percent, value, val_max, val_min);
		sprintf(output, " %i ", (int)percent);
		fontHeight = FontStringY(f->mText, output);
		posY = 31 - fontHeight;
		
		// Frame
		r.mX = 0;
		r.mY = posY - 1;
		r.mWidth = LCD_X;
		r.mHeight = fontHeight + 2;
		ScreenRect(screen, r, 0);
		// Bar
		r.mX = 1;
		r.mY = posY;
		r.mWidth = (double)(LCD_X - 2) * (percent / 100.0);
		r.mHeight = fontHeight;
		ScreenRect(screen, r, 1);
		
		FontScreenString(f->mText, screen, BUTTON_POS_CENTER, posY, output, 0, JUSTIFY_CENTER);
		
		LcdWriteImageScreen(lcd, screen);
	}
	ScreenDestroy(screen);
	if (feof(fIn)) {
		return result;
	}
	
	return EXITCODE_CANCEL;
}

int Percent(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg, int optind, int argc, char **argv)
{
	int localEnd = 0;
	int result = EXITCODE_ERROR;
	ScreenData *screen;
	char output[30];
	int posY;
	int fontHeight;
	double percent;
	int value, valMin, valMax;
	int valueOld = 0;
	int len;
	int button;
	Rect r;
	Rect viewPort;
	
	if (!lcd || !f || !text || !screenBg) {
		return EXITCODE_ERROR;
	}
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return EXITCODE_ERROR;
	}
	
	len = argc - optind;
	if (len == 1) {
		value = atoi(argv[optind]);
		valMin = 0;
		valMax = 100;
	} else if (len == 3) {
		value = atoi(argv[optind]);
		valMin = atoi(argv[optind + 1]);
		valMax = atoi(argv[optind + 2]);
	} else {
		return EXITCODE_ERROR;
	}
	if (valMin >= valMax) {
		return EXITCODE_ERROR;
	}
	if (value < valMin) {
		value = valMin;
	}
	if (value > valMax) {
		value = valMax;
	}
	valueOld = value;
	if (!gConfig.mStepWidth) {
		gConfig.mStepWidth = 5;
	}
	
	CatchCtrlC();
	ButtonInit();
	while (!gEnde && !localEnd) {
		ScreenCopy(screen, screenBg);
		
		viewPort.mX = 0;
		viewPort.mY = 0;
		viewPort.mHeight = 16;
		viewPort.mWidth = LCD_X;
		FontScreenStringVp(f->mText, screen, viewPort, text, 0, JUSTIFY_LEFT);
		
		percent = ((double)(value - valMin) / (valMax - valMin)) * 100.0;
		//printf("%i %i %i %i\n", (int)percent, value, val_max, val_min);
		sprintf(output, " %i ", (int)percent);
		fontHeight = FontStringY(f->mText, output);
		posY = 23 - fontHeight;
		
		// Frame
		r.mX = 0;
		r.mY = posY - 1;
		r.mWidth = LCD_X;
		r.mHeight = fontHeight + 2;
		ScreenRect(screen, r, 0);
		// Bar
		r.mX = 1;
		r.mY = posY;
		r.mWidth = (double)(LCD_X - 2) * (percent / 100.0);
		r.mHeight = fontHeight;
		ScreenRect(screen, r, 1);
		
		FontScreenString(f->mText, screen, BUTTON_POS_CENTER, posY, output, 0, JUSTIFY_CENTER);
		
		FontScreenString(f->mSystem, screen, BUTTON_POS_LEFT, 24, "\x05", 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_CENTER, 24, "\x02", 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_RIGHT, 24, "\x04", 0, JUSTIFY_CENTER);
		LcdWriteImageScreen(lcd, screen);
		
		button = ButtonGet();
		switch (button) {
		case -1:
			gEnde = 1;
			break;
		case '3':
			value += gConfig.mStepWidth;
			if (value > valMax) {
				value = valMax;
			}
			break;
		case '2':
			localEnd = 1;
			result = EXITCODE_OK;
			fprintf(stderr, "%i\n", (int)percent);
			break;
		case '1':
			value -= gConfig.mStepWidth;
			if (value < valMin) {
				value = valMin;
			}
			break;
		case ASCII_CAN:
			localEnd = 1;
			result = EXITCODE_TIMEOUT;
			break;
		}
		if (gConfig.mSingleStep) {
			if (valueOld != value) {
				ScreenDestroy(screen);
				percent = ((double)(value - valMin) / (valMax - valMin)) * 100.0;
				fprintf(stderr, "%i\n", (int)percent);
				
				return EXITCODE_EXTRA;
			}
		}
		valueOld = value;
	}
	ScreenDestroy(screen);
	if (localEnd) {
		return result;
	}
	
	return EXITCODE_CANCEL;
}
