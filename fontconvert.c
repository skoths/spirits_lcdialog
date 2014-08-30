/**
@brief FontConvert
Convert the mpf (Mini-Pixel-Font) file into a c-header file.

@author Stephan Ruloff
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <linux/limits.h>
#include <ctype.h>

#include "font.h"

int main(int argc, char **argv)
{
	char *fontname = 0;
	char *headername = 0;
	FontMaster *m;
	int ret;
	char font[PATH_MAX] = "";
	char *lastDot = 0;
	char *lastSlash = 0;
	int i;
	
	if (argc == 3) {
		fontname = argv[1];
		headername = argv[2];
	} else {
		printf("fontconvert <fontname> <headername>\n");
		
		return 1;
	}
	
	m = FontCreate();
	if (!m) {
		printf("Out of memory\n");
		
		exit(1);
	}
	ret = FontLoad(m, fontname);
	
	if (ret) {
		lastDot = strrchr(fontname, '.');
		lastSlash = strrchr(fontname, '/');
		
		if (lastSlash == 0) {
			lastSlash = fontname;
		} else {
			lastSlash++;
		}
		if (lastDot == 0) {
			lastDot = fontname + strlen(fontname);
		}
		for (i = 0; lastSlash < lastDot; lastSlash++) {
			font[i] = toupper(*lastSlash);
			i++;
		}
		font[i] = 0;
		printf("Font %s\n", font);
		ret = FontConvertToHeader(m, font, headername);
		if (ret) {
			printf("Error %i\n", ret);
		} else {
			printf("Ok\n");
		}
	} else {
		printf("No Character in FontFile!\n");
	}

	return 0;
}
