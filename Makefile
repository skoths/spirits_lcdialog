CC=gcc
CFLAGS=-Wall -g
OBJS=gpio.o font.o spi.o lcd.o screen.o mode_ok_info.o mode_menu.o \
	mode_numbers.o mode_bar.o conv_str.o
LDLIBS=-lgd -lm -liconv
FONT_FILES=FONT7.h FONT11.h FONT23.h

all: lcdialog

lcdialog: config.h $(OBJS)

config.h:
	[ -f config.h ] || cp config.example.h config.h

screen.o: fonts.h $(FONT_FILES)

clean:
	rm -rf *.o
	rm -rf lcdialog
