#ifndef _FONT_H_
#define _FONT_H_

#include "screen.h"

#include <stdint.h>

typedef struct {
	uint8_t mX;
	uint8_t mY;
	uint8_t *mData;
} FontData;

typedef struct {
	FontData mChars[256];
} FontMaster;

#define JUSTIFY_LEFT 0
#define JUSTIFY_RIGHT 1
#define JUSTIFY_CENTER 2


FontMaster* FontCreate(void);
uint8_t FontLoad(FontMaster *m, const char *filename);
void FontDestroy(FontMaster *m);
void FontDebugPrint(FontMaster *m, char c);
int FontConvertToHeader(FontMaster *m, const char *fontname, const char *headername);
uint16_t FontStringX(FontMaster *m, const char *text);
uint16_t FontStringY(FontMaster *m, const char *text);
uint16_t FontCharX(FontMaster *m, char ch);
uint16_t FontCharY(FontMaster *m, char ch);
uint8_t FontScreenStringVp(FontMaster *m, ScreenData *screen, Rect viewPort, 
		const char *text, uint8_t isInv, uint8_t justify);
uint8_t FontScreenString(FontMaster *m, ScreenData *screen, uint8_t txtX, 
		uint8_t txtY, const char *text, uint8_t isInv, uint8_t justify);


#endif
