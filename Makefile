CC=gcc
CFLAGS=-Wall -g
OBJS=gpio.o font.o spi.o lcd.o screen.o mode_ok_info.o mode_menu.o \
	mode_numbers.o mode_bar.o conv_str.o
LDLIBS=-lgd -lm
FONT_FILES=FONT7.h FONT11.h FONT23.h

all: fontconvert convert lcdialog

lcdialog: config.h $(OBJS)

config.h:
	[ -f config.h ] || cp config.example.h config.h

screen.o: fonts.h $(FONT_FILES)

fontconvert: font.o

convert: $(FONT_FILES)

FONT7.h: font7.mpf
	./fontconvert font7.mpf FONT7.h

FONT11.h: font11.mpf
	./fontconvert font11.mpf FONT11.h

FONT23.h: font23.mpf
	./fontconvert font23.mpf FONT23.h

install:
	install -o root -m u+srwx,g+rx,o+rx lcdialog /usr/sbin
	mkdir -p /usr/local/share/man/man1/
	mkdir -p /usr/local/share/man/de/man1/
	gzip -c lcdialog.1 > /usr/local/man/man1/lcdialog.1.gz
	gzip -c lcdialog.de.1 > /usr/local/man/de/man1/lcdialog.1.gz

clean:
	rm -rf *.o
	rm -rf lcdialog
	rm -rf fontconvert
	rm -rf $(FONT_FILES)
