/**
@brief Mode: Ok and Info
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
#include <string.h>

#include "lcd.h"
#include "font.h"
#include "lcdialog.h"

void WriteText(LcdSpi *lcd, Fonts *f, const char* text, ScreenData *screenBg, uint8_t lines)
{
	ScreenData *screen;
	Rect viewPort;
	
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return;
	}
	ScreenCopy(screen, screenBg);
	viewPort.mX = 0;
	viewPort.mY = 0;
	viewPort.mWidth = LCD_X;
	viewPort.mHeight = lines * 8;
	FontScreenStringVp(f->mText, screen, viewPort, text, 0, JUSTIFY_LEFT);
	LcdWriteImageScreen(lcd, screen);
}

int Ok(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg)
{
	int button;
	int local_end = 0;
	int result = EXITCODE_ERROR;
	ScreenData *screen;
	char textOk[10];
	Rect viewPort;
	
	if (!lcd || !f || !text || !screenBg) {
		return EXITCODE_ERROR;
	}
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return EXITCODE_ERROR;
	}
	
	CatchCtrlC();
	ButtonInit();
	while (!gEnde && !local_end) {
		ScreenCopy(screen, screenBg);
	
		viewPort.mX = 0;
		viewPort.mY = 0;
		viewPort.mWidth = LCD_X;
		viewPort.mHeight = 24;
		FontScreenStringVp(f->mText, screen, viewPort, text, 0, JUSTIFY_LEFT);
		
		sprintf(textOk, "%s", "\x02");
		FontScreenString(f->mSystem, screen, BUTTON_POS_CENTER, 24, textOk, 0, JUSTIFY_CENTER);
		LcdWriteImageScreen(lcd, screen);
		
		button = ButtonGet();
		switch (button) {
		case -1:
			gEnde = 1;
			break;
		case '2': // ok
			local_end = 1;
			result = EXITCODE_OK;
			break;
		}
	}
	if (local_end) {
		return result;
	}
	
	return EXITCODE_CANCEL;
}

int Info(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg)
{
	WriteText(lcd, f, text, screenBg, 4);
	
	return EXITCODE_OK;
}

int YesNo(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg)
{
	int button;
	int local_end = 0;
	int result = EXITCODE_ERROR;
	ScreenData *screen;
	char textYes[10];
	char textNo[10];
	Rect viewPort;

	if (!lcd || !text || !screenBg) {	
		return EXITCODE_ERROR;
	}
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return EXITCODE_ERROR;
	}
	
	CatchCtrlC();
	ButtonInit();
	while (!gEnde && !local_end) {
		ScreenCopy(screen, screenBg);
		viewPort.mX = 0;
		viewPort.mY = 0;
		viewPort.mWidth = LCD_X;
		viewPort.mHeight = 24;
		FontScreenStringVp(f->mText, screen, viewPort, text, 0, JUSTIFY_LEFT);
		
		sprintf(textYes, "\x02");
		sprintf(textNo, "\x03");
		FontScreenString(f->mSystem, screen, BUTTON_POS_LEFT, 24, textYes, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_RIGHT, 24, textNo, 0, JUSTIFY_CENTER);
		LcdWriteImageScreen(lcd, screen);
		
		button = ButtonGet();
		switch (button) {
		case -1:
			gEnde = 1;
			break;
		case '1':
			result = EXITCODE_OK;
			local_end = 1;
			break;
		case '3':
			result = EXITCODE_CANCEL;
			local_end = 1;
			break;
		}
	}
	if (local_end) {
		return result;
	}
	
	return 0;
}
