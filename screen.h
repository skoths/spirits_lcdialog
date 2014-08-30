#ifndef _SCREEN_H_
#define _SCREEN_H_

/**
@brief Screen-Handling
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

#include <stdint.h>

typedef uint8_t PixelType;

typedef struct
{
	uint16_t mX;
	uint16_t mY;
	PixelType *mData;
} ScreenData;

typedef struct
{
	int16_t mX;
	int16_t mY;
	int16_t mWidth;
	int16_t mHeight;
} Rect;

typedef struct
{
	uint16_t mPosX;
	uint16_t mPosY;
	uint16_t mWidth;
	uint16_t mHeight;
} CursorData;

ScreenData* ScreenInit(uint16_t x, uint16_t y);
void ScreenDestroy(ScreenData* sd);

uint8_t ScreenLoadImage(ScreenData *screen, const char *filename, 
		uint16_t offX, uint16_t offY);
uint8_t ScreenRect(ScreenData *screen, Rect r, uint8_t isFill);
uint8_t ScreenCopy(ScreenData *dest, ScreenData *src);
uint8_t ScreenClear(ScreenData *scr);
void ScreenBlink(ScreenData *dest, CursorData cur);

#endif
