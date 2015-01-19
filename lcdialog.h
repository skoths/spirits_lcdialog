#ifndef _LCDLIALOG_H_
#define _LCDLIALOG_H_
/**
@brief The application types and definitions
@author Stephan Ruloff
@date 11.08.2014
*/

#include "font.h"
#include "lcd.h"

typedef struct
{
	FontMaster *mSystem;
	FontMaster *mText;
	FontMaster *mInternal;
} Fonts;

typedef struct 
{
	char *mBgFilename;
	char *mMenuIndex;
	uint8_t mBgOffX;
	uint8_t mBgOffY;
	uint8_t mIsBgLight;
	uint8_t mBgLight;
	uint8_t mIsInit;
	uint8_t mIsClear;
	uint8_t mIsNoTag;
	uint8_t mFontType;
	uint8_t mSingleStep;
	uint8_t mStepWidth;
	uint32_t mTimeout;
} Config;


#define PIN_KEY_1 23
#define PIN_KEY_2 24
#define PIN_KEY_3 25

#define BUTTON_POS_LEFT 21
#define BUTTON_POS_CENTER 66
#define BUTTON_POS_RIGHT 110

#define EXITCODE_ERROR -1
#define EXITCODE_OK 0
#define EXITCODE_CANCEL 1
#define EXITCODE_HELP 2
#define EXITCODE_EXTRA 3
#define EXITCODE_TIMEOUT 4

#define ASCII_CAN 0x18

extern Config gConfig;
extern uint8_t gEnde;

void SignalHandler(int sig);
void CatchCtrlC(void);
void ButtonInit(void);
int ButtonGet(void);
int ButtonGetCursor(LcdSpi *lcd, ScreenData *screen, CursorData cur);

#endif
