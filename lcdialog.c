/**
@brief LCDialog

@author Stephan Ruloff
@date 07.07.2014
*/

/*
LCDialog, a dialog like program to use it with a graphic LCD.
Copyright (C) 2014  Stephan Ruloff

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
#include <getopt.h>
#include <unistd.h>
#include <signal.h>
#include <poll.h>
#include <string.h>
#include <errno.h>
#include <arpa/inet.h>
#include <iconv.h>

#include "lcdialog.h"
#include "lcd.h"
#include "gpio.h"
#include "font.h"
#include "fonts.h"
#include "screen.h"
#include "mode.h"


typedef enum {
		OPT_YESNO = 1, OPT_OK, OPT_MENU, OPT_IPV4, OPT_SUBNETMASK, OPT_INFO,
		OPT_BUTTONWAIT, OPT_INTINPUT, OPT_PROGRESS, OPT_PERCENT,
		
		OPT_BGIMG, OPT_BGLIGHT, OPT_STEP_WIDTH, OPT_SINGLE_STEP,
		
		OPT_INIT, OPT_UNINIT, OPT_CLEAR,
		
		OPT_NOTAG, OPT_MENU_INDEX, OPT_FONT_MEDIUM, OPT_FONT_LARGE, OPT_HELP
} Options;

static struct option sLongOptions[] = {
		{"yesno", required_argument, 0, OPT_YESNO},
		{"ok", required_argument, 0, OPT_OK},
		{"menu", no_argument, 0, OPT_MENU},
		{"ipv4", required_argument, 0, OPT_IPV4},
		{"subnetmask", required_argument, 0, OPT_SUBNETMASK},
		{"info", required_argument, 0, OPT_INFO},
		{"buttonwait", optional_argument, 0, OPT_BUTTONWAIT},
		{"intinput", required_argument, 0, OPT_INTINPUT},
		{"progress", required_argument, 0, OPT_PROGRESS},
		{"percent", required_argument, 0, OPT_PERCENT},

		// Option
		{"bgimg", required_argument, 0, OPT_BGIMG},
		{"bglight", required_argument, 0, OPT_BGLIGHT},
		{"stepwidth", required_argument, 0, OPT_STEP_WIDTH},
		{"singlestep", no_argument, 0, OPT_SINGLE_STEP},

		{"init", no_argument, 0, OPT_INIT},
		{"uninit", no_argument, 0, OPT_UNINIT},
		{"clear", no_argument, 0, OPT_CLEAR},
		
		{"notag", no_argument, 0, OPT_NOTAG},
		{"menuindex", required_argument, 0, OPT_MENU_INDEX},
		{"font-medium", no_argument, 0, OPT_FONT_MEDIUM},
		{"font-large", no_argument, 0, OPT_FONT_LARGE},
		{"help", no_argument, 0, 'h'},

		{0, 0, 0, 0}
};

#ifndef VERSION
#define VERSION "1.0"
#endif

#define DEBOUNCE 100 /* ms */

#define COUNTER_MAX 10 /* 50ms units */

#define MAX_ISO8859_LEN 256 

Config gConfig;
uint8_t gEnde = 0;

void SignalHandler(int sig)
{
	printf("\n%s: %i\n", __FUNCTION__, sig);
	switch (sig) {
	case SIGTERM:
	case SIGKILL:
	case SIGINT:
		gEnde = 1;
		break;
	}
}

void CatchCtrlC(void)
{
	struct sigaction sa;
	
	sa.sa_handler = SignalHandler;
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGINT, &sa, NULL) == -1) {
		exit(EXITCODE_ERROR);
	}
}

/**
@brief Handling of "filename.jpg:+23+12"
+23 offset in x-axis
+12 offset in y-axis
The empty add-on will be treated as +0+0
@param[in] filename The filename string with add-on
@param{out] x The detected offset in x
@param{out] y The detected offset in y
@retval 1 Ok
@retval 0 Error
*/
uint8_t GetFilenameAddon(char* filename, int16_t *x, int16_t *y)
{
	char *pos;
	char sigX;
	char sigY;
	int offX;
	int offY;
	int ret;
	
	if (!x || !y || !filename) {
		return 0;
	}
	pos = strstr(filename, ":");
	if (pos) {
		ret = sscanf(pos, ":%c%i%c%i", &sigX, &offX, &sigY, &offY);
		if (ret == 4) {
			if (sigX == '-' || sigX == '+') {
				if (sigX == '-') {
					*x = offX * (-1);
				} else {
					*x = offX;
				}
			} else {
				return 0;
			}
			if (sigY == '-' || sigY == '+') {
				if (sigY == '-') {
					*y = offY * (-1);
				} else {
					*y = offY;
				}
			} else {
				return 0;
			}
			
			return 1;
		} else {
			return 0;
		}
	} else {
		*x = 0;
		*y = 0;
		
		return 1;
	}
}

char* GetFilenameWithoutAddon(char* filename)
{
	char* result = 0;
	char* pos;
	
	if (!filename) {
		return NULL;
	}
	result = strdup(filename);
	pos = strstr(result, ":");
	if (pos) {
		*pos = 0;
	}
	
	return result;
}

void ButtonInit(void)
{
	GpioExport(PIN_KEY_1);
	GpioExport(PIN_KEY_2);
	GpioExport(PIN_KEY_3);
	GpioSetDirection(PIN_KEY_1, 0);
	GpioSetDirection(PIN_KEY_2, 0);
	GpioSetDirection(PIN_KEY_3, 0);
	GpioSetEdge(PIN_KEY_1, edge_falling);
	GpioSetEdge(PIN_KEY_2, edge_falling);
	GpioSetEdge(PIN_KEY_3, edge_falling);
	
#ifdef RPI_PULLUP
	GpioRpiSetup();
	GpioRpiSetPullUpDown(PIN_KEY_1, PUD_UP);
	GpioRpiSetPullUpDown(PIN_KEY_2, PUD_UP);
	GpioRpiSetPullUpDown(PIN_KEY_3, PUD_UP);
#endif
}

int ButtonGet(void)
{
	int ret;
	int fd1;
	int fd2;
	int fd3;
#if 0
	struct pollfd pollKey[3];
	int retPoll;
	char buffer[2];
#endif
	uint8_t val1;
	uint8_t val2;
	uint8_t val3;
	uint8_t val1_old = 0;
	uint8_t val2_old = 0;
	uint8_t val3_old = 0;
	int result = 0;
	
	ret = 0;
	ret += GpioFdOpen(PIN_KEY_1, &fd1);
	ret += GpioFdOpen(PIN_KEY_2, &fd2);
	ret += GpioFdOpen(PIN_KEY_3, &fd3);

	if (ret != 3) {
		return -1;
	}
#if 0
	bzero(pollKey, sizeof(pollKey));
	pollKey[0].fd = fd1;
	pollKey[0].events = POLLPRI;
	pollKey[1].fd = fd2;
	pollKey[1].events = POLLPRI;
	pollKey[2].fd = fd3;
	pollKey[2].events = POLLPRI;
	
	
	while (!ende) {
		retPoll = poll(pollKey, 3, 1000);
		if (retPoll < 0) {
			if (errno == EINTR) {
				continue;
			} else {
				printf("Error: %i\n", errno);
				break;
			}
		} else if (retPoll == 0) {
			// timeout
			//printf(".\n");
		} else {
			if (pollKey[0].revents & POLLPRI) {
				ret = read(fd1, buffer, 2);
				printf("Taste 1: %c\n", buffer[0]);
				usleep(DEBOUNCE * 1000);
				lseek(fd1, 0, SEEK_SET);
				result = '1';
				break;
			}
			if (pollKey[1].revents & POLLPRI) {
				ret = read(fd2, buffer, 2);
				printf("Taste 2: %c\n", buffer[0]);
				usleep(DEBOUNCE * 1000);
				lseek(fd2, 0, SEEK_SET);
				result = '2';
				break;
			}
			if (pollKey[2].revents & POLLPRI) {
				ret = read(fd3, buffer, 2);
				printf("Taste 3: %c\n", buffer[0]);
				usleep(DEBOUNCE * 1000);
				lseek(fd3, 0, SEEK_SET);
				result = '3';
				break;
			}
		}
	}
	GpioFdClose(fd1);
	GpioFdClose(fd2);
	GpioFdClose(fd3);
#endif

	while (!gEnde) {
		GpioGetValue(PIN_KEY_1, &val1);
		GpioGetValue(PIN_KEY_2, &val2);
		GpioGetValue(PIN_KEY_3, &val3);
		
		if (val1 != val1_old && val1 == 0) {
			usleep(DEBOUNCE * 1000);
			result = '1';
			break;
		}
		if (val2 != val2_old && val2 == 0) {
			usleep(DEBOUNCE * 1000);
			result = '2';
			break;
		}
		if (val3 != val3_old && val3 == 0) {
			usleep(DEBOUNCE * 1000);
			result = '3';
			break;
		}
		
		val1_old = val1;
		val2_old = val2;
		val3_old = val3;
		usleep(50 * 1000);
	}

	return result;
}

int ButtonGetCursor(LcdSpi *lcd, ScreenData *screen, CursorData cur)
{
	int ret;
	int fd1;
	int fd2;
	int fd3;
	uint8_t val1;
	uint8_t val2;
	uint8_t val3;
	uint8_t val1_old = 0;
	uint8_t val2_old = 0;
	uint8_t val3_old = 0;
	int result = 0;
	int counter = COUNTER_MAX;
	ScreenData *screenOut;
	
	ret = 0;
	ret += GpioFdOpen(PIN_KEY_1, &fd1);
	ret += GpioFdOpen(PIN_KEY_2, &fd2);
	ret += GpioFdOpen(PIN_KEY_3, &fd3);

	if (ret != 3) {
		return -1;
	}

	if (!screen) {
		return 0;
	}
	screenOut = ScreenInit(screen->mX, screen->mY);
	ScreenCopy(screenOut, screen);

	while (!gEnde) {
		counter++;
		if (counter > COUNTER_MAX) {
			ScreenBlink(screenOut, cur);
			LcdWriteImageScreen(lcd, screenOut);
			counter = 0;
		}

		GpioGetValue(PIN_KEY_1, &val1);
		GpioGetValue(PIN_KEY_2, &val2);
		GpioGetValue(PIN_KEY_3, &val3);
		
		if (val1 != val1_old && val1 == 0) {
			usleep(DEBOUNCE * 1000);
			result = '1';
			break;
		}
		if (val2 != val2_old && val2 == 0) {
			usleep(DEBOUNCE * 1000);
			result = '2';
			break;
		}
		if (val3 != val3_old && val3 == 0) {
			usleep(DEBOUNCE * 1000);
			result = '3';
			break;
		}
		
		val1_old = val1;
		val2_old = val2;
		val3_old = val3;
		usleep(50 * 1000);
	}
	ScreenDestroy(screenOut);

	return result;
}

void ButtonRelease(void)
{
	// TODO !!
}

int ButtonWait(void)
{
	int button;
	
	ButtonInit();
	button = ButtonGet();
	switch (button) {
	case '1':
		return EXITCODE_OK;
	case '2':
		return EXITCODE_CANCEL;
	case '3':
		return EXITCODE_EXTRA;
	}
	
	return EXITCODE_ERROR;
}

#if 0
void LoadFonts(Fonts *f)
{
	int ret;
	
	if (!f) {
		printf("Null-Pointer\n");
		exit(EXITCODE_ERROR);
	}
	f->mSystem = FontCreate();
	if (!f->mSystem) {
		printf("Out of memory\n");
		
		exit(EXITCODE_ERROR);
	}
	ret = FontLoad(f->mSystem, "font7.mpf");
	if (!ret) {
		printf("Font-Error\n");
		
		exit(EXITCODE_ERROR);
	}
	switch (gConfig.mFontType) {
	case 1:
		f->mInternal = FontCreate();
		if (!f->mInternal) {
			printf("Out of memory\n");
			FontDestroy(f->mSystem);
			
			exit(EXITCODE_ERROR);
		}
		ret = FontLoad(f->mInternal, "font11.mpf");
		if (!ret) {
			printf("Font-Error\n");
			
			exit(EXITCODE_ERROR);
		}
		f->mText = f->mInternal;
		break;
	case 2:
		f->mInternal = FontCreate();
		if (!f->mInternal) {
			printf("Out of memory\n");
			FontDestroy(f->mSystem);
			
			exit(EXITCODE_ERROR);
		}
		ret = FontLoad(f->mInternal, "font23.mpf");
		if (!ret) {
			printf("Font-Error\n");
			
			exit(EXITCODE_ERROR);
		}
		f->mText = f->mInternal;
		break;
	default:
		f->mText = f->mSystem;
		break;
	}
}
#endif
void LoadFonts(Fonts *f)
{
	switch (gConfig.mFontType) {
	case 1:
		f->mText = &FONT11;
		break;
	case 2:
		f->mText = &FONT23;
		break;
	default:
		f->mText = &FONT7;
	}
	f->mSystem = &FONT7;
}

int ModeHandler(int mode, char *textIn, int argc, char **argv)
{
	LcdSpi *lcd;
	Spi *spiBus0;
	ScreenData *screenBg;
	int result = 0;
	Fonts font;
	iconv_t ic;
    size_t res;
    char text[MAX_ISO8859_LEN] = "";
	
	memset(&font, 0, sizeof(Fonts));
	spiBus0 = SpiCreate(0);
	if (spiBus0 == NULL) {
		printf("SPI-Error\n");
		exit(EXITCODE_ERROR);
	}
	lcd = LcdOpen(spiBus0);
	if (!lcd) {
		printf("LCD-Error\n");
		exit(EXITCODE_ERROR);
	}
	if (gConfig.mIsInit == 1) {
		LcdInit(lcd);
	} else if (gConfig.mIsInit == 2) {
		LcdUninit(lcd);
		exit(EXITCODE_OK);
	}
	if (gConfig.mIsBgLight) {
		LcdSetBgLight(lcd, gConfig.mBgLight & 1, gConfig.mBgLight & 2, gConfig.mBgLight & 4);
	}
	screenBg = ScreenInit(LCD_X, LCD_Y);
	if (!screenBg) {
		printf("Screen-Error\n");
		exit(EXITCODE_ERROR);
	}
	ScreenClear(screenBg);
	if (gConfig.mBgFilename) {
		if (ScreenLoadImage(screenBg, gConfig.mBgFilename, gConfig.mBgOffX, gConfig.mBgOffY) != 0) {
			ScreenClear(screenBg);
		}
	}
	
	if (textIn) {
		int testInLen = strlen(textIn);
		char **inPtr = &textIn;
		char *outPtr = &text[0];
		
		ic = iconv_open("ISO-8859-1", "UTF-8");
		if (ic != (iconv_t)(-1)) {
			size_t inBytesLeft = testInLen;
			size_t outBytesLeft = sizeof(text) - 1;
		   
			res = iconv(ic, inPtr, &inBytesLeft, &outPtr, &outBytesLeft);
			if ((int)res != -1 && outBytesLeft) {
				outPtr[0] = 0;
			} else {
				strncpy(text, textIn, sizeof(text) - 1);
				text[sizeof(text) - 1] = 0;
			}
			iconv_close(ic);
		}
	}
	
	//printf("Mode: %i\n", mode);
	switch (mode) {
	case OPT_YESNO:
		LoadFonts(&font);
		result = YesNo(lcd, &font, text, screenBg);
		break;
	case OPT_OK:
		LoadFonts(&font);
		result = Ok(lcd, &font, text, screenBg);
		break;
	case OPT_MENU:
		LoadFonts(&font);
		result = Menu(lcd, &font, screenBg, optind, argc, argv);
		break;
	case OPT_IPV4:
		LoadFonts(&font);
		result = Ipv4(lcd, &font, text, screenBg, optind, argc, argv);
		break;
	case OPT_SUBNETMASK:
		LoadFonts(&font);
		result = Subnetmask(lcd, &font, text, screenBg, optind, argc, argv);
		break;
	case OPT_INFO:
		LoadFonts(&font);
		result = Info(lcd, &font, text, screenBg);
		break;
	case OPT_BUTTONWAIT:
		result = ButtonWait();
		break;
	case OPT_INTINPUT:
		LoadFonts(&font);
		result = IntInput(lcd, &font, text, screenBg, optind, argc, argv);
		break;
	case OPT_PROGRESS:
		LoadFonts(&font);
		result = Progress(lcd, &font, text, screenBg, optind, argc, argv);
		break;
	case OPT_PERCENT:
		LoadFonts(&font);
		result = Percent(lcd, &font, text, screenBg, optind, argc, argv);
		break;
	default:
		break;
	}
	
	if (font.mSystem) {
		//FontDestroy(font.mSystem);
	}
	if (font.mInternal) {
		//FontDestroy(font.mInternal);
	}

	if (gConfig.mIsClear) {
		LcdCls(lcd);
	}
	ScreenDestroy(screenBg);
	LcdCleanup(lcd);
	SpiDestroy(spiBus0);
	
	return result;
}

void Usage(void)
{
	printf("LCDialog version %s, Copyright (C) 2014 Stephan Ruloff\n", VERSION);
	printf("LCDialog comes with ABSOLUTELY NO WARRANTY; for details\n");
	printf("http://www.gnu.org/licenses/gpl-2.0\n");
	printf("This is free software, and you are welcome to redistribute\n");
	printf("it under certain conditions.\n");

	printf("\nUsage: lcdialog [option] <mode> [<arg1> ...]\n\n");
	printf("  --ok <text>\n");
	printf("  --info <text>\n");
	printf("  --yesno <text>\n");
	printf("  --menu <tag> <item> [ <tag> <item> ...]\n");
	printf("  --ipv4 <ipv4> [<text>]\n");
	printf("  --subnetmask <subnet mask>\n");
	printf("  --buttonwait\n");
	printf("  --intinput <text> <value> <max>\n");
	
	printf("\nOptions:\n");
	printf("  --init\n");
	printf("  --uninit\n");
	printf("  --clear\n");
	printf("  --notag\n");
	printf("  --stepwidth <witdh>\n");
	printf("  --singlestep\n");
	printf("  --menuindex <tag>\n");
	printf("  --bgimg <filename>\n");
	printf("  --bglight <color-code>\n");
	printf("  --font-medium\n");
	printf("  --font-large\n");
	
	printf("\nsee documentation for more details.\n");
	
	exit(EXITCODE_HELP);
}

int main(int argc, char **argv)
{
	int c = 0;
	int optionIndex = 0;
	int mode = -1;
	char *text = 0;
	int16_t x, y;
	int debug = 0;
	
	if (argc == 1) {
		Usage();
	}

	bzero(&gConfig, sizeof(gConfig));
	while (c != -1) {
		c = getopt_long(argc, argv, "d:h", sLongOptions, &optionIndex);
		switch (c) {
		case OPT_YESNO:
		case OPT_OK:
		case OPT_MENU:
		case OPT_IPV4:
		case OPT_SUBNETMASK:
		case OPT_INFO:
		case OPT_BUTTONWAIT:
		case OPT_INTINPUT:
		case OPT_PROGRESS:
		case OPT_PERCENT:
			if (mode == -1) {
				mode = c;
				text = optarg; 
			} else {
				printf("Only one mode\n");
				exit(EXITCODE_ERROR);
			}
			break;

		case OPT_BGIMG:
			gConfig.mBgFilename = GetFilenameWithoutAddon(optarg);
			if (GetFilenameAddon(optarg, &x, &y)) {
				gConfig.mBgOffX = x;
				gConfig.mBgOffY = y;
			}
			//printf("Name: %s Off: %ix%i\n", gConfig.mBgFilename, x, y);
			break;
			
		case OPT_BGLIGHT:
			gConfig.mIsBgLight = 1;
			gConfig.mBgLight = atoi(optarg);
			break;
			
		case OPT_INIT:
			gConfig.mIsInit = 1;
			break;
		case OPT_UNINIT:
			gConfig.mIsInit = 2;
			break;
		case OPT_CLEAR:
			gConfig.mIsClear = 1;
			break;
		case OPT_STEP_WIDTH:
			gConfig.mStepWidth = atoi(optarg);
			break;
		case OPT_SINGLE_STEP:
			gConfig.mSingleStep = 1;
			break;
		case OPT_NOTAG:
			gConfig.mIsNoTag = 1;
			break;
		case OPT_MENU_INDEX:
			gConfig.mMenuIndex = optarg;
			break;
		case OPT_FONT_MEDIUM:
			gConfig.mFontType = 1;
			break;
		case OPT_FONT_LARGE:
			gConfig.mFontType = 2;
			break;
			
		case 'd':
			debug = atoi(optarg);
			break;
			
		case '?':
			break;

		case -1:
			break;
			
		case 'h':
		default:
			Usage();
		}
	}
	
	if (debug) {
		LcdSetDebuglevel(debug);
	}
	
	return ModeHandler(mode, text, argc, argv);
}
