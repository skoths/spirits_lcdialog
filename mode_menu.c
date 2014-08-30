/**
@brief Mode: Menu
@author Stephan Ruloff
@date 11.08.2014
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

#include "lcd.h"
#include "font.h"
#include "lcdialog.h"
#include "screen.h"

int Menu(LcdSpi *lcd, Fonts *f, ScreenData *screenBg, int optind, int argc, char **argv)
{
	uint32_t len;
	uint32_t i, j;
	uint32_t maxTagLen = 0;
	uint32_t tagLen;
	uint32_t optionLen;
	ScreenData *screen;
	int button;
	int local_end = 0;
	int result = EXITCODE_ERROR;
	uint8_t lines;
	char **tags;
	char **items;
	uint8_t posPointer = 0;
	uint8_t startMenu = 0;
	uint8_t menuMax = 0;
	char textOk[10];
	char textCancel[10];
	char textMenu[10];
	uint8_t mode = 1;
	uint8_t posMode = 1;
	uint8_t lineHeight = 8;
	Rect viewPort;
	
	if (!lcd || !f || !screenBg) {
		printf("Null-pointer\n");
		
		return EXITCODE_ERROR;
	}
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return EXITCODE_ERROR;
	}
	
	lines = 3;	
	if (optind >= argc) {
		printf("No menu options\n");
		exit(EXITCODE_ERROR);
	}
	if (gConfig.mFontType == 1) {
		lines = 2;
		lineHeight = 12;
	} else if (gConfig.mFontType == 2) {
		lines = 1;
		lineHeight = 24;
	}
	
	len = argc - optind;
	//printf("%i %i %i\n", len, argc, optind);
	optionLen = len / 2;
	
	if (optionLen < lines) {
		menuMax = optionLen;
	} else {
		menuMax = lines;
	}
	tags = malloc(sizeof(char*) * optionLen);
	items = malloc(sizeof(char*) * optionLen);
	for (i = 0, j = 0; i < len; i += 2, j++) {
		//printf("%i, Tag: %s - Item: %s\n", i, argv[optind + i], argv[optind + i + 1]);
		tags[j] = argv[optind + i];
		items[j] = argv[optind + i + 1];
		tagLen = FontStringX(f->mText, tags[j]);
		if (tagLen > maxTagLen) {
			maxTagLen = tagLen;
		}
		if (gConfig.mMenuIndex && strcmp(tags[j], gConfig.mMenuIndex) == 0) {
			if (j >= lines) {
				startMenu = j - lines + 1;
				posPointer = lines - 1;
			} else {
				startMenu = 0;
				posPointer = j;
			}
		}
	}

	CatchCtrlC();
	ButtonInit();
	while (!gEnde && !local_end) {
		ScreenCopy(screen, screenBg);
	
		viewPort.mX = 0;
		viewPort.mY = posPointer * lineHeight;
		viewPort.mWidth = LCD_X;
		viewPort.mHeight = lineHeight;
			
		if (mode == 1) {
			FontScreenStringVp(f->mText, screen, viewPort, "\x04", 0, JUSTIFY_LEFT);
		} else if (mode == 2) {
			FontScreenStringVp(f->mText, screen, viewPort, "\x04", 0, JUSTIFY_LEFT);
		}
		
		for (i = 0; i < menuMax; i++) {
			viewPort.mY = i * lineHeight;
			if (gConfig.mIsNoTag) {
				viewPort.mX = 10;
				viewPort.mWidth = LCD_X - 10;
				FontScreenStringVp(f->mText, screen, viewPort, items[startMenu + i], 0, JUSTIFY_LEFT);
			} else {
				viewPort.mX = 10;
				viewPort.mWidth = LCD_X - 10;
				FontScreenStringVp(f->mText, screen, viewPort, tags[startMenu + i], 0, JUSTIFY_LEFT);
				viewPort.mX = 10 + maxTagLen + 3;
				viewPort.mWidth = LCD_X - viewPort.mX;
				FontScreenStringVp(f->mText, screen, viewPort, items[startMenu + i], 0, JUSTIFY_LEFT);
			}
			//printf("%i %s\n", i, tags[i]);
		}

		sprintf(textOk, "\x06");
		sprintf(textMenu, "\x02");
		sprintf(textCancel, "\x07");
		FontScreenString(f->mSystem, screen, BUTTON_POS_LEFT, 24, textOk, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_CENTER, 24, textMenu, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_RIGHT, 24, textCancel, 0, JUSTIFY_CENTER);
		LcdWriteImageScreen(lcd, screen);

		button = ButtonGet();
		//printf("Button: %i\n", button);
		switch (button) {
		case -1:
			gEnde = 1;
			break;
		case '2': // ok
			local_end = 1;
			result = EXITCODE_OK;
			if (startMenu + posPointer < optionLen) {
				fprintf(stderr, "%s\n", tags[startMenu + posPointer]);
			}
			break;
			
		case '3':
			if (mode == 1) {
				if (posPointer == (lines - 1)) {
					if (optionLen > (startMenu + lines)) {
						startMenu++;
					}
				}
				if (posPointer < (menuMax - 1)) {
					posPointer++;
				}
			} else if (mode == 2) {
				if (posMode < 2) {
					posMode++;
				}
			}
			break;
			
		case '1':
			if (mode == 1) {
				if (posPointer == 0) {
					if (startMenu > 0) {
						startMenu--;
					}
				}
				if (posPointer > 0) {
					posPointer--;
				}
			} else if (mode == 2) {
				if (posMode > 0) {
					posMode--;
				}
			}
			break;
		}
	}
	free(tags);
	free(items);
	
	return result;
}
