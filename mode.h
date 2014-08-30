#ifndef _MODE_H_
#define _MODE_H_
/**
@brief Header for all Mode's
@author Stephan Ruloff
@date 11.08.2014
*/

#include "screen.h"

int Ok(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg);
int Info(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg);
int YesNo(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg);

int Menu(LcdSpi *lcd, Fonts *f, ScreenData *screenBg, int optind, int argc, char **argv);

int Ipv4(LcdSpi *lcd, Fonts *f, char *ip, ScreenData *screenBg, int optind, int argc, char **argv);
int Subnetmask(LcdSpi *lcd, Fonts *f, char *nmip, ScreenData *screenBg, int optind, int argc, char **argv);
int IntInput(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg, int optind, int argc, char **argv);

int Progress(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg, int optind, int argc, char **argv);
int Percent(LcdSpi *lcd, Fonts *f, char *text, ScreenData *screenBg, int optind, int argc, char **argv);

#endif
