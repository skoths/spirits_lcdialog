/**
@brief String Helper Functions

@author Stephan Ruloff
@date 26.09.2014
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
#include <string.h>
#include <errno.h>
#include <iconv.h>

/**
@brief Do the "strdup" stuff and iconv from UTF-8 to ISO-8859-1
@param[in] text The input in UTF-8.
@returns The new allocated buffer with the text in ISO-8859-1. 
@note Don't forget to free the buffer!
*/
char* StrdupConv(char *text)
{
	char *outbuffer;
	size_t len;
	char **inPtr;
	char *outPtr;
	iconv_t ic;
	size_t res;
	
	if (!text) {
		return 0;
	}
	len = strlen(text);
	// Guess the size, UTF-8 is always bigger than ISO.
	outbuffer = malloc(len + 1);
	if (!outbuffer) {
		return 0;
	}
	inPtr = &text;
	outPtr = outbuffer;
		
	ic = iconv_open("ISO-8859-1", "UTF-8");
	if (ic != (iconv_t)(-1)) {
		size_t inBytesLeft = len;
		size_t outBytesLeft = len;
	   
		res = iconv(ic, inPtr, &inBytesLeft, &outPtr, &outBytesLeft);
		if ((int)res != -1) {
			outPtr[0] = 0;
		} else {
			strncpy(outbuffer, text, len);
			outbuffer[len] = 0;
		}
		iconv_close(ic);
	} else {
		free(outbuffer);
		
		return 0;
	}

	return outbuffer;
}

