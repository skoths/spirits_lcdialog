/**
@brief Screen-Handling
@author Stephan Ruloff
@date July 2014
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
#include <errno.h>

#include <gd.h>

#include "screen.h"

static int IdentifyFileType(const char *filename)
{
	FILE *f;
	uint16_t magic16;
	uint32_t magic32;
	size_t ret;
	
	f = fopen(filename, "rb");
	if (f == NULL) {
		return -1;
	}
	ret = fread(&magic32, sizeof(magic32), 1, f);
	fclose(f);
	if (ret == 1) {
		magic16 = (uint16_t)magic32;
		
		switch (magic16) {
		case 0xD8FF:
			return 2; // JPEG
		}
		
		switch (magic32) {
		case 0x474e5089:
			return 1; // PNG
		}
		
		return 0;
	} else {
		return -2;
	}
}

static int minInt(int a, int b)
{
	return a > b ? b : a;
}

ScreenData* ScreenInit(uint16_t x, uint16_t y)
{
	ScreenData* sd;
	
	sd = (ScreenData*)malloc(sizeof(ScreenData));
	if (!sd) {
		return NULL;
	}
	
	sd->mX = x;
	sd->mY = y;
	sd->mData = (uint8_t*)malloc(sd->mX * sd->mY * sizeof(PixelType));
	if (!sd->mData) {
		free(sd);
		
		return NULL;
	}
	
	return sd;
}

void ScreenDestroy(ScreenData* sd)
{
	if (!sd) {
		return;
	}
	
	free(sd->mData);
	free(sd);
}

uint8_t ScreenLoadImage(ScreenData *screen, const char *filename, uint16_t offX, uint16_t offY)
{
	uint16_t x = 0;
	uint16_t y = 0;
	uint16_t xm = 0;
	uint16_t ym = 0;
	int c;
	gdImagePtr img;
	FILE *filePtr;
	int filetype;
	
	if (!screen) {
		return 1;
	}
	if (!filename) {
		return 2;
	}

	filetype = IdentifyFileType(filename);
	if (filetype < 0) {
		return 6;
	}
	filePtr = fopen(filename, "rb");
	if (filePtr == NULL) {
		return 3;
	}
	switch (filetype) {
	case 1:
		img = gdImageCreateFromPng(filePtr);
		break;
	
	case 2:
		img = gdImageCreateFromJpeg(filePtr);
		break;
	
	default:
		fclose(filePtr);

		return 5;
	}
	if (img == NULL) {
		fclose(filePtr);

		return 4;
	}
	xm = gdImageSX(img);
	ym = gdImageSY(img);
	//printf("%i x %i\n", xm, ym);
	
	xm = minInt(xm + offX, screen->mX);
	ym = minInt(ym + offY, screen->mY);
	
	xm -= offX;
	ym -= offY;

	ScreenClear(screen);
	for (y = 0; y < ym; y++) {
		for (x = 0; x < xm; x++) {
			c = gdImageGetPixel(img, x, y);
			if (gdImageRed(img, c) < 0x7f || gdImageGreen(img, c) < 0x7f || gdImageRed(img, c) < 0x7f) {
				screen->mData[(screen->mX * (y + offY)) + x + offX] = 1;
			}
		}
	}
	fclose(filePtr);
	gdImageDestroy(img);
	
	return 0;
}

uint8_t ScreenRect(ScreenData *screen, Rect r, uint8_t isFill)
{
	int16_t x = 0;
	int16_t y = 0;
		
	if (!screen) {
		return 1;
	}
	
	if (isFill) {
		for (y = r.mY; y < (r.mY + r.mHeight); y++) {
			for (x = r.mX; x < (r.mX + r.mWidth); x++) {
				if (x >= 0 && x < screen->mX && y >= 0 && y < screen->mY) {
					screen->mData[(screen->mX * (y)) + x] = 1;
				}
			}
		}
	} else {
		// verticals (left,right)
		for (y = r.mY; y < (r.mY + r.mHeight); y++) {
			x = r.mX;
			if (x >= 0 && x < screen->mX && y >= 0 && y < screen->mY) {
				screen->mData[(screen->mX * (y)) + x] = 1;
			}
			x = r.mX + r.mWidth - 1;
			if (x >= 0 && x < screen->mX && y >= 0 && y < screen->mY) {
				screen->mData[(screen->mX * (y)) + x] = 1;
			}
			
		}
		// horizontals (top,bottom)
		for (x = r.mX; x < (r.mX + r.mWidth); x++) {
			y = r.mY;
			if (x >= 0 && x < screen->mX && y >= 0 && y < screen->mY) {
				screen->mData[(screen->mX * (y)) + x] = 1;
			}
			y = r.mY + r.mHeight - 1;
			if (x >= 0 && x < screen->mX && y >= 0 && y < screen->mY) {
				screen->mData[(screen->mX * (y)) + x] = 1;
			}
			
		}
	}
	
	return 0;
}

uint8_t ScreenCopy(ScreenData *dest, ScreenData *src)
{
	if (!dest || !src) {
		return 1;
	}
	if (dest->mX != src->mX || dest->mY != src->mY) {
		return 2;
	}
	memcpy(dest->mData, src->mData, dest->mX * dest->mY * sizeof(PixelType));
	
	return 0;
}

uint8_t ScreenClear(ScreenData *scr)
{
	if (!scr) {
		return 1;
	}
	if (scr->mX == 0 || scr->mY == 0) {
		return 2;
	}
	memset(scr->mData, 0, scr->mX * scr->mY * sizeof(PixelType));
	
	return 0;
}

void ScreenBlink(ScreenData *dest, CursorData cur)
{
	uint8_t posX;
	uint8_t posY;
	uint8_t x, y;

	if (!dest) {
		return;
	}
	//printf("Blink %i %i %i %i\n", cur.mWidth, cur.mHeight, cur.mPosX, cur.mPosY);
	for (x = 0; x < cur.mWidth; x++) {
		for (y = 0; y < cur.mHeight; y++) {
			posX = x + cur.mPosX;
			posY = cur.mPosY - y;	
			if ((posX < dest->mX) && (posY < dest->mY)) {
				dest->mData[(posY * dest->mX) + posX] ^= 1;
			}
		}
	}
}
