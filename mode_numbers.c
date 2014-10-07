/**
@brief Mode: IPv4, Netmask and IntInput
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
#include <arpa/inet.h>
#include <math.h>

#include "lcd.h"
#include "font.h"
#include "lcdialog.h"
#include "conv_str.h"

#define IP_UP 0
#define IP_DOWN 1

typedef struct
{
	uint8_t mByte[4];
} IpBytes;


uint32_t CalcIntegerFromDigit(uint8_t *digit, uint8_t len)
{
	uint8_t i;
	uint32_t exp = 1;
	uint32_t result = 0;
	
	if (!digit) {
		return 0;
	}
	for (i = 0; i < len; i++) {
		result += digit[len - 1 - i] * exp;
		exp *= 10;
	}
	
	return result;
}

/**
@brief
digit1 = octet % 10;
digit10 = (octet / 10) % 10;
...	
@param[in] integer The Interger to convert
@param[out] digit The resulting digits.
@param[in] len Number of digits.
*/
void CalcIntegerToDigit(uint32_t integer, uint8_t *digit, uint8_t len)
{
	uint8_t i;
	uint32_t exp = 1;
	
	if (!digit) {
		return;
	}
	for (i = 0; i < len; i++) {
		digit[len - 1 - i] = (integer / exp) % 10;
		exp *= 10;
	}
}

void IntegerUpDown(uint8_t *digit, uint8_t dirDown)
{
	if (!digit) {
		return;
	}
	if (dirDown) {
		if (*digit == 0) {
			*digit = 9;
		} else {
			(*digit)--;
		}
	} else {
		if (*digit == 9) {
			*digit = 0;
		} else {
			(*digit)++;
		}
	}
}

/**
@brief manipulate a integer by it's position in decimal representation
@param[in,out] integer The integer to manipulate
@param[in] integerMax the maximal possible value of the integer
@param[in] pos the position from the left of the decimal (e.g. integreMax:1234 -> pos=0 -> 1)
@param[in] dirDown counting down?
*/
void SetInteger(uint32_t *integer, uint32_t integerMax, uint8_t pos, uint8_t dirDown)
{
	/* max. possible number is 4294967295, so 10 digits are right */
	uint8_t digit[10];
	int temp;
	int len;
	
	if (!integer || !integerMax) {
		return;
	}
	len = (int)ceil(log10(integerMax));
	if (pos >= len) {
		return;
	}
	
	CalcIntegerToDigit(*integer, digit, len);
	IntegerUpDown(&digit[pos], dirDown);
	temp = CalcIntegerFromDigit(digit, len);
	while (temp > integerMax) {
		IntegerUpDown(&digit[pos], dirDown);
		temp = CalcIntegerFromDigit(digit, len);
	}
	*integer = temp;
}

void SetIp(IpBytes *ipb, int pos, uint8_t dirDown)
{
	int posByte = 0;
	int pos10 = 0;
	uint32_t octet;
	
	if (!ipb) {
		return;
	}
	
	if (pos >= 4 && pos <= 6) {
		posByte = 0;
		pos10 = pos - 4;
	} else if (pos >= 8 && pos <= 10) {
		posByte = 1;
		pos10 = pos - 8;
	} else if (pos >= 12 && pos <= 14) {
		posByte = 2;
		pos10 = pos - 12;
	} else if (pos >= 16 && pos <= 18) {
		posByte = 3;
		pos10 = pos - 16;
	}

	octet = ipb->mByte[posByte];
	SetInteger(&octet, 255, pos10, dirDown);
	ipb->mByte[posByte] = octet;
}

int Ipv4(LcdSpi *lcd, Fonts *f, char *ip, ScreenData *screenBg, int optind, int argc, char **argv)
{
	int button;
	int local_end = 0;
	int result = EXITCODE_ERROR;
	ScreenData *screen;
	char textSet[10];
	char navL[10];
	char navR[10];
	char output[30];
	IpBytes ipb;
	int posZiffer = 20;
	char cursor[2] = " ";
	int posX;
	int fontHeight;
	CursorData cur;
	int mode = 0;
	int setMode = 0;
	char *text = 0;
	int a, b, c, d;
	Rect viewPort;
	
	if (!lcd || !f || !screenBg) {
		return EXITCODE_ERROR;
	}
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return EXITCODE_ERROR;
	}
	
	if (optind < argc) {
		text = StrdupConv(argv[optind]);
	} else {
		text = strdup("Set IPv4");
	}

	if (4 != sscanf(ip, "%d.%d.%d.%d", &a, &b, &c, &d)) {
		memset(&ipb, 0, sizeof(ipb));
	} else {
		ipb.mByte[0] = a;
		ipb.mByte[1] = b;
		ipb.mByte[2] = c;
		ipb.mByte[3] = d;
	}
	
	CatchCtrlC();
	ButtonInit();
	while (!gEnde && !local_end) {
		ScreenCopy(screen, screenBg);
	
		viewPort.mX = 0;
		viewPort.mY = 0;
		viewPort.mWidth = LCD_X;
		viewPort.mHeight = 16;
		sprintf(output, "IP: %03d.%03d.%03d.%03d \x02\x03", ipb.mByte[0], ipb.mByte[1], ipb.mByte[2], ipb.mByte[3]);
		FontScreenStringVp(f->mText, screen, viewPort, text, 0, JUSTIFY_LEFT);
		FontScreenString(f->mSystem, screen, 0, 16, output, 0, JUSTIFY_LEFT);
		
		// Cursor
		cur.mPosY = 16 + FontStringY(f->mSystem, output);
		if (mode == 0) {
			cur.mHeight = 2;
		} else {
			cur.mHeight = fontHeight + 2;
		}
		fontHeight = FontStringY(f->mSystem, output);
		cursor[0] = output[posZiffer];
		output[posZiffer] = 0;
		cur.mWidth = FontCharX(f->mSystem, cursor[0]) + 2;
		posX = FontStringX(f->mSystem, output);
		cur.mPosX = posX;
		//FontScreenString(f->mSystem, screen, LCD_X, LCD_Y, posX, 16, cursor, 1, JUSTIFY_LEFT);
		
		if (posZiffer == 21) {
			setMode = 1;
		} else if (posZiffer == 20) {
			setMode = 0;
		} else {
			if (mode == 0) {
				setMode = 2;
			} else {
				setMode = 3;
			}
		}
		
		switch (setMode) {
		case 0: // ok
			sprintf(textSet, "%s", "\x02");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 1: // cancel
			sprintf(textSet, "%s", "\x03");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 2: // nav
			sprintf(textSet, "%s", "\x14");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 3: // set
			sprintf(textSet, "%s", "\x01");
			sprintf(navL, "%s", "\x06");
			sprintf(navR, "%s", "\x07");
			break;
		}
		FontScreenString(f->mSystem, screen, BUTTON_POS_LEFT, 24, navL, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_CENTER, 24, textSet, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_RIGHT, 24, navR, 0, JUSTIFY_CENTER);
		LcdWriteImageScreen(lcd, screen);
		
		button = ButtonGetCursor(lcd, screen, cur);
		switch (button) {
		case -1:
			gEnde = 1;
			break;
		
		case '2': // ok
			if (posZiffer == 20) {
				local_end = 1;
				result = EXITCODE_OK;
				if (inet_ntop(AF_INET, &ipb, output, sizeof(output) - 1)) {
					fprintf(stderr, "%s\n", output);
				} else {
					fprintf(stderr, "0.0.0.0\n");
				}
			} else if (posZiffer == 21) {
				local_end = 1;
				result = EXITCODE_CANCEL;
			} else {
				if (mode == 0) {
					mode = 1;
				} else {
					mode = 0;
				}
			}
			break;
		
		case '1':
			if (mode == 0) {
				if (posZiffer > 4) {
					posZiffer--;
				}
				switch (posZiffer) {
				case 7: posZiffer = 6; break;
				case 11: posZiffer = 10; break;
				case 15: posZiffer = 14; break;
				case 19: posZiffer = 18; break;
				}
			} else {
				SetIp(&ipb, posZiffer, IP_UP);
			}
			break;
		
		case '3':
			if (mode == 0) {
				if (posZiffer < 21) {
					posZiffer++;
				}
				switch (posZiffer) {
				case 7: posZiffer = 8; break;
				case 11: posZiffer = 12; break;
				case 15: posZiffer = 16; break;
				case 19: posZiffer = 20; break;
				}
			} else {
				SetIp(&ipb, posZiffer, IP_DOWN);
			}
			break;
		}
	}
	free(text);
	ScreenDestroy(screen);
	if (local_end) {
		return result;
	}
	
	return EXITCODE_CANCEL;
}

int GetCidr(IpBytes *ipb)
{
	uint32_t nm = 0;
	uint32_t mask = 0xFFFFFFFF;
	int i;
	
	nm = (ipb->mByte[0] << 24) | (ipb->mByte[1] << 16) 
			| (ipb->mByte[2] << 8) | ipb->mByte[3];
	
	for (i = 32; i > 0; i--) {
		if (mask == nm) {
			return i;
		}
		mask <<= 1;
	}
	
	return 0;
}

int SetCidr(IpBytes *ipb, int cidr)
{
	uint32_t mask = 0xFFFFFFFF;
	
	if (!ipb) {
		return 0;
	}
	mask <<= 32 - (cidr % 33);
	ipb->mByte[0] = mask >> 24;
	ipb->mByte[1] = mask >> 16;
	ipb->mByte[2] = mask >> 8;
	ipb->mByte[3] = mask;
	
	return 1;
}

void SetIntegerCidr(uint32_t *integer, int pos, uint8_t dirDown)
{
	uint8_t pos10 = 0;
	
	if (pos == 17) {
		pos10 = 0;
	} else if (pos == 18) {
		pos10 = 1;
	}	
	SetInteger(integer, 32, pos10, dirDown);
}

int Subnetmask(LcdSpi *lcd, Fonts *f, char *nmip, ScreenData *screenBg, int optind, int argc, char **argv)
{
	int button;
	int localEnd = 0;
	int result = EXITCODE_ERROR;
	ScreenData *screen;
	char textSet[10];
	char navL[10];
	char navR[10];
	char output[30];
	IpBytes ipb;
	int posZiffer = 20;
	char cursor[2] = " ";
	int posX;
	int fontHeight;
	CursorData cur;
	int mode = 0;
	int setMode = 0;
	char *text = 0;
	uint32_t cidr = 0;
	int a, b, c, d;
	Rect viewPort;
	
	if (!lcd || !f || !screenBg) {
		return EXITCODE_ERROR;
	}
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return EXITCODE_ERROR;
	}
	
	if (optind < argc) {
		text = StrdupConv(argv[optind]);
	} else {
		text = strdup("Set subnet mask");
	}

	if (4 != sscanf(nmip, "%d.%d.%d.%d", &a, &b, &c, &d)) {
		memset(&ipb, 0, sizeof(ipb));
	} else {
		ipb.mByte[0] = a;
		ipb.mByte[1] = b;
		ipb.mByte[2] = c;
		ipb.mByte[3] = d;
	}
	//printf("%d.%d.%d.%d\n", a, b, c, d);
	cidr = GetCidr(&ipb);
	
	CatchCtrlC();
	ButtonInit();
	while (!gEnde && !localEnd) {
		ScreenCopy(screen, screenBg);
	
		viewPort.mX = 0;
		viewPort.mY = 0;
		viewPort.mWidth = LCD_X;
		viewPort.mHeight = 16;
		SetCidr(&ipb, cidr);
		sprintf(output, "%03d.%03d.%03d.%03d /%02i \x02\x03", 
				ipb.mByte[0], ipb.mByte[1], ipb.mByte[2], ipb.mByte[3], cidr);
		FontScreenStringVp(f->mText, screen, viewPort, text, 0, JUSTIFY_LEFT);
		FontScreenString(f->mSystem, screen, 0, 16, output, 0, JUSTIFY_LEFT);
		
		// Cursor
		cur.mPosY = 16 + FontStringY(f->mSystem, output);
		if (mode == 0) {
			cur.mHeight = 2;
		} else {
			cur.mHeight = fontHeight + 2;
		}
		fontHeight = FontStringY(f->mSystem, output);
		cursor[0] = output[posZiffer];
		output[posZiffer] = 0;
		cur.mWidth = FontCharX(f->mSystem, cursor[0]) + 2;
		posX = FontStringX(f->mSystem, output);
		cur.mPosX = posX;
		//FontScreenString(f->mSystem, screen, LCD_X, LCD_Y, posX, 16, cursor, 1, JUSTIFY_LEFT);
		
		if (posZiffer == 21) {
			setMode = 1;
		} else if (posZiffer == 20) {
			setMode = 0;
		} else {
			if (mode == 0) {
				setMode = 2;
			} else {
				setMode = 3;
			}
		}
		
		switch (setMode) {
		case 0: // ok
			sprintf(textSet, "%s", "\x02");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 1: // cancel
			sprintf(textSet, "%s", "\x03");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 2: // nav
			sprintf(textSet, "%s", "\x14");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 3: // set
			sprintf(textSet, "%s", "\x01");
			sprintf(navL, "%s", "\x06");
			sprintf(navR, "%s", "\x07");
			break;
		}
		FontScreenString(f->mSystem, screen, BUTTON_POS_LEFT, 24, navL, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_CENTER, 24, textSet, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_RIGHT, 24, navR, 0, JUSTIFY_CENTER);
		LcdWriteImageScreen(lcd, screen);
		
		button = ButtonGetCursor(lcd, screen, cur);
		switch (button) {
		case -1:
			gEnde = 1;
			break;
		case '2': // ok
			if (posZiffer == 20) {
				localEnd = 1;
				result = EXITCODE_OK;
				if (inet_ntop(AF_INET, &ipb, output, sizeof(output) - 1)) {
					fprintf(stderr, "%s\n", output);
				} else {
					fprintf(stderr, "0.0.0.0\n");
				}
			} else if (posZiffer == 21) {
				localEnd = 1;
				result = EXITCODE_CANCEL;
			} else {
				if (mode == 0) {
					mode = 1;
				} else {
					mode = 0;
				}
			}
			break;
		case '1':
			if (mode == 0) {
				if (posZiffer > 17) {
					posZiffer--;
				}
				switch (posZiffer) {
				case 19: posZiffer = 18; break;
				}
			} else {
				SetIntegerCidr(&cidr, posZiffer, IP_UP);
			}
			break;
		case '3':
			if (mode == 0) {
				if (posZiffer < 21) {
					posZiffer++;
				}
				switch (posZiffer) {
				case 19: posZiffer = 20; break;
				}
			} else {
				SetIntegerCidr(&cidr, posZiffer, IP_DOWN);
			}
			break;
		}
	}
	free(text);
	ScreenDestroy(screen);
	if (localEnd) {
		return result;
	}
	
	return EXITCODE_CANCEL;
}

int IntInput(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg, int optind, int argc, char **argv)
{
	int button;
	int localEnd = 0;
	int result = EXITCODE_ERROR;
	ScreenData *screen;
	char textSet[10];
	char navL[10];
	char navR[10];
	char output[30];
	int posZiffer = 0;
	char cursor[2] = " ";
	int posX;
	int fontHeight;
	CursorData cur;
	int mode = 0;
	int setMode = 0;
	uint32_t value = 0;
	uint32_t valueMax = 0;
	uint32_t valueLen = 0;
	uint8_t posOk;
	uint8_t posCancel;
	uint8_t posSpace;
	uint8_t posValue;
	int len;
	Rect viewPort;
	
	if (!lcd || !f || !text || !screenBg) {
		return EXITCODE_ERROR;
	}
	screen = ScreenInit(LCD_X, LCD_Y);
	if (!screen) {
		return EXITCODE_ERROR;
	}

	len = argc - optind;
	if (len == 2) {
		value = atoi(argv[optind]);
		valueMax = atoi(argv[optind + 1]);
		//printf("%i %i\n", value, valueMax);
	} else {
		return EXITCODE_ERROR;
	}
	if (value > valueMax) {
		return EXITCODE_ERROR;
	}
	valueLen = ceil(log10(valueMax));
	posOk = valueLen + 2;
	posCancel = posOk + 1;
	posSpace = posOk - 1;
	posValue = 1;
	posZiffer = posOk;
	
	CatchCtrlC();
	ButtonInit();
	while (!gEnde && !localEnd) {
		ScreenCopy(screen, screenBg);
	
		viewPort.mX = 0;
		viewPort.mY = 0;
		viewPort.mWidth = LCD_X;
		viewPort.mHeight = 16;
		sprintf(output, " %0*d \x02\x03", valueLen, value);
		FontScreenStringVp(f->mText, screen, viewPort, text, 0, JUSTIFY_LEFT);
		FontScreenString(f->mSystem, screen, 0, 16, output, 0, JUSTIFY_LEFT);
		
		// Cursor
		cur.mPosY = 16 + FontStringY(f->mSystem, output);
		if (mode == 0) {
			cur.mHeight = 2;
		} else {
			cur.mHeight = fontHeight + 2;
		}
		fontHeight = FontStringY(f->mSystem, output);
		cursor[0] = output[posZiffer];
		output[posZiffer] = 0;
		cur.mWidth = FontCharX(f->mSystem, cursor[0]) + 2;
		posX = FontStringX(f->mSystem, output);
		cur.mPosX = posX;
		//FontScreenString(f->mSystem, screen, LCD_X, LCD_Y, posX, 16, cursor, 1, JUSTIFY_LEFT);
		
		if (posZiffer == posCancel) {
			setMode = 1;
		} else if (posZiffer == posOk) {
			setMode = 0;
		} else {
			if (mode == 0) {
				setMode = 2;
			} else {
				setMode = 3;
			}
		}
		
		switch (setMode) {
		case 0: // ok
			sprintf(textSet, "%s", "\x02");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 1: // cancel
			sprintf(textSet, "%s", "\x03");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 2: // nav
			sprintf(textSet, "%s", "\x14");
			sprintf(navL, "%s", "\x05");
			sprintf(navR, "%s", "\x04");
			break;
		case 3: // set
			sprintf(textSet, "%s", "\x01");
			sprintf(navL, "%s", "\x06");
			sprintf(navR, "%s", "\x07");
			break;
		}
		FontScreenString(f->mSystem, screen, BUTTON_POS_LEFT, 24, navL, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_CENTER, 24, textSet, 0, JUSTIFY_CENTER);
		FontScreenString(f->mSystem, screen, BUTTON_POS_RIGHT, 24, navR, 0, JUSTIFY_CENTER);
		LcdWriteImageScreen(lcd, screen);
		
		button = ButtonGetCursor(lcd, screen, cur);
		switch (button) {
		case -1:
			gEnde = 1;
			break;
		case '2': // ok
			//printf("ok\n");
			if (posZiffer == posOk) {
				localEnd = 1;
				result = EXITCODE_OK;
				fprintf(stderr, "%d\n", value);
			} else if (posZiffer == posCancel) {
				localEnd = 1;
				result = EXITCODE_CANCEL;
			} else {
				if (mode == 0) {
					mode = 1;
				} else {
					mode = 0;
				}
			}
			break;
		case '1':
			if (mode == 0) {
				if (posZiffer > posValue) {
					posZiffer--;
				}
				if (posZiffer == posSpace) {
					posZiffer = posSpace - 1;
				}
			} else {
				SetInteger(&value, valueMax, posZiffer - posValue, IP_UP);
			}
			break;
		case '3':
			if (mode == 0) {
				if (posZiffer < posCancel) {
					posZiffer++;
				}
				if (posZiffer == posSpace) {
					posZiffer = posSpace + 1;
				}
			} else {
				SetInteger(&value, valueMax, posZiffer - posValue, IP_DOWN);
			}
			break;
		}
	}
	ScreenDestroy(screen);
	if (localEnd) {
		return result;
	}
	
	return EXITCODE_CANCEL;
}
