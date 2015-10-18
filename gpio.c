/**
@brief Linux GPIO control by /sys/class/gpio
@author Stephan Ruloff
@date 27.06.2014
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
#include <unistd.h>
#include <string.h>
#include <stdint.h>
#include <fcntl.h>
#include <errno.h>

#include <sys/types.h>
#include <sys/stat.h>

#include "gpio.h"
#include "config.h"


static char *triggerName[] = {
	"none",
	"nand-disk", 
	"mmc0", 
	"timer", 
	"heartbeat", 
	"default-on",
};

static char *edgeName[] = {
	"none",
	"rising", 
	"falling", 
	"both", 
};

static char *pullName[] = {
	"up",
	"down", 
	"disable", 
};

/**
@brief Check for directory
@retval 0 Ok
@retval 1 error
@retval 2 not a dir
@retval 3 does not exist 
*/
uint8_t DirectoryStatus(char *dir)
{
	struct stat s;
	int err = stat(dir, &s);
	
	if (err == -1) {
		if (ENOENT == errno) {
			return 3;
		} else {
			return 1;
		}
	} else {
		if (S_ISDIR(s.st_mode)) {
			return 0;
		} else {
			return 2;
		}
	}
}

/**
Den Trigger für das Aufleuchten einer der 4 LEDs einstellen
@param[in] led Die Nummer der LED (1..4)
@param[in] trigger Der Trigger
@return true=Erfolgreich eingestellt
*/
uint8_t LedSetTrigger(const char* led, Trigger trigger)
{
	char pfad[128];
	FILE *f;
	
	snprintf(pfad, 128, "/sys/class/leds/%s/trigger", led);
	f = fopen(pfad, "w");
	if (f == NULL) {
		return 0;
	}
	fprintf(f, "%s", triggerName[trigger]);
	fclose(f);
	
	return 1;
}

/**
Die Helligkeit einer LED festlegen.
@param[in] led Die Nummer der LED (1..4)
@param[in] brightness Die Helligkeit (0..100)
@return true=Erfolgreich eingestellt
*/	
uint8_t LedSetBrightness(const char* led, uint8_t brightness)
{
	char pfad[128];
	FILE *f;
	
	snprintf(pfad, 128, "/sys/class/leds/%s/brightness", led);	
	f = fopen(pfad, "w");
	if (f == NULL) {
		return 0;
	}
	fprintf(f, "%i", brightness);
	fclose(f);
	
	return 1;
}

/**
@brief Export des Pins in den Userspace
@param[in] pin Der Pin der CPU
@retval 0 Fehler
@retval 1 Ok
*/
uint8_t GpioExport(uint8_t pin)
{
	FILE *f;
	char pfad[128];
	
	sprintf(pfad, "/sys/class/gpio/gpio%i", pin);	
	if (DirectoryStatus(pfad) == 0) {
		return 1;
	}
	f = fopen("/sys/class/gpio/export", "w");
	if (f == NULL) {
		return 0;
	}
	fprintf(f, "%i", pin);
	fclose(f);
	
	return 1;
}

uint8_t GpioUnexport(uint8_t pin)
{
	FILE *f;
	
	f = fopen("/sys/class/gpio/unexport", "w");
	if (f == NULL) {
		return 0;
	}
	fprintf(f, "%i", pin);
	fclose(f);
	
	return 1;
}

uint8_t GpioGetDirection(uint8_t pin, uint8_t *isOut)
{
	FILE *f;
	char pfad[128];
	int c;
	
	if (isOut == NULL) {
		return 0;
	}
	sprintf(pfad, "/sys/class/gpio/gpio%i/direction", pin);	
	f = fopen(pfad, "r");
	if (f == NULL) {
		return 0;
	}
	c = fgetc(f);
	if (c == 'o') {
		*isOut = 1;
	} else {
		*isOut = 0;
	}
	fclose(f);
	
	return 1;
}

uint8_t GpioSetDirection(uint8_t pin, uint8_t isOut)
{
	FILE *f;
	char pfad[128];
	uint8_t outStatus;
	
	if (GpioGetDirection(pin, &outStatus)) {
		if (outStatus == isOut) {
			return 1;
		}
	}
	sprintf(pfad, "/sys/class/gpio/gpio%i/direction", pin);	
	f = fopen(pfad, "w");
	if (f == NULL) {
		return 0;
	}
	if (isOut) {
		fprintf(f, "out");
	} else {
		fprintf(f, "in");
	}
	fclose(f);
	
	return 1;
}

uint8_t GpioSetValue(uint8_t pin, uint8_t value)
{
	FILE *f;
	char pfad[128];
	
	sprintf(pfad, "/sys/class/gpio/gpio%i/value", pin);	
	f = fopen(pfad, "w");
	if (f == NULL) {
		return 0;
	}
	fprintf(f, "%i", value);
	fclose(f);
	
	return 1;
}

uint8_t GpioGetValue(uint8_t pin, uint8_t *value)
{
	FILE *f;
	char pfad[128];
	int anz;
	int valRaw;
	
	if (!value) {
		return 0;
	}
	sprintf(pfad, "/sys/class/gpio/gpio%i/value", pin);	
	f = fopen(pfad, "r");
	if (f == NULL) {
		return 0;
	}
	anz = fscanf(f, "%i", &valRaw);
	fclose(f);
	
	if (anz == 1) {
		*value = (uint8_t)valRaw;
		return 1;
	} else {
		return 0;
	}
}

/**
Die Flanke für den Interrupt einstellen
@param[in] pin Die Nummer des GPIO-Pins
@param[in] edge Die Flanke
@return 1=Erfolgreich eingestellt
*/
uint8_t GpioSetEdge(uint8_t pin, Edge edge)
{
	char pfad[128];
	FILE *f;
	
	snprintf(pfad, 128, "/sys/class/gpio/gpio%i/edge", pin);
	f = fopen(pfad, "w");
	if (f == NULL) {
		return 0;
	}
	fprintf(f, "%s", edgeName[edge]);
	fclose(f);
	
	return 1;
}

/**
Set the pull-up/down resistor
@param[in] pin The GPIO pin
@param[in] pull The pull-type
@retval 0 Error
@retval 1 Ok
*/
uint8_t GpioSetPullResistor(uint8_t pin, PullResistor pull)
{
	char pfad[128];
	FILE *f;
	
	snprintf(pfad, 128, "/sys/class/gpio/gpio%i/pull", pin);
	f = fopen(pfad, "w");
	if (f == NULL) {
		return 0;
	}
	fprintf(f, "%s", pullName[pull]);
	fclose(f);
	
	return 1;
}


uint8_t GpioFdOpen(uint8_t pin, int *fd)
{
	char pfad[128];
	
	if (!fd) {
		return 0;
	}
	sprintf(pfad, "/sys/class/gpio/gpio%i/value", pin);	
	*fd = open(pfad, O_RDONLY);
	if (*fd < 0) {
		return 0;
	}
	
	return 1;
}

uint8_t GpioFdClose(int fd)
{
	return close(fd);
}

#ifdef RPI_PULLUP
/* http://elinux.org/RPi_Low-level_peripherals */

//  How to access GPIO registers from C-code on the Raspberry-Pi
//  Example program
//  15-January-2012
//  Dom and Gert
//  Revised: 15-Feb-2013

#include <sys/mman.h>

#define BCM2708_PERI_BASE        0x20000000
#define BCM2709_PERI_BASE        0x3f000000
#define GPIO_BASE_OFFSET          0x200000

#define FSEL_OFFSET 0 // 0x0000
#define SET_OFFSET 7 // 0x001c / 4
#define CLR_OFFSET 10 // 0x0028 / 4
#define PINLEVEL_OFFSET 13 // 0x0034 / 4
#define EVENT_DETECT_OFFSET 16 // 0x0040 / 4
#define RISING_ED_OFFSET 19 // 0x004c / 4
#define FALLING_ED_OFFSET 22 // 0x0058 / 4
#define HIGH_DETECT_OFFSET 25 // 0x0064 / 4
#define LOW_DETECT_OFFSET 28 // 0x0070 / 4
#define PULLUPDN_OFFSET 37 // 0x0094 / 4
#define PULLUPDNCLK_OFFSET 38 // 0x0098 / 4 

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

volatile uint32_t *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0


/* Set up a memory regions to access GPIO */
void GpioRpiSetup(void)
{
	int mem_fd;
	uint32_t peri_base;
	uint32_t gpio_base;
	unsigned char buf[4];
	FILE *fp;
	char buffer[1024];
	char hardware[1024];
	int found = 0;            

	// Based on "pasberyy-gpio-python", file "c_gpio.c"
	// determine peri_base
	if ((fp = fopen("/proc/device-tree/soc/ranges", "rb")) != NULL) {
		// get peri base from device tree
		fseek(fp, 4, SEEK_SET);
		if (fread(buf, 1, sizeof buf, fp) == sizeof buf) {
			peri_base = buf[0] << 24 | buf[1] << 16 | buf[2] << 8 | buf[3] << 0;
		}
		fclose(fp);
	} else {
		// guess peri base based on /proc/cpuinfo hardware field
		if ((fp = fopen("/proc/cpuinfo", "r")) == NULL) {
			printf("can't open /proc/cpuinfo\n");
			exit(-1);
		}
	                                                                                            
		while(!feof(fp) && !found) {
			fgets(buffer, sizeof(buffer), fp);
			sscanf(buffer, "Hardware	: %s", hardware);
			if (strcmp(hardware, "BCM2708") == 0 || strcmp(hardware, "BCM2835") == 0) {
				// pi 1 hardware
				peri_base = BCM2708_PERI_BASE;
				found = 1;
			} else if (strcmp(hardware, "BCM2709") == 0 || strcmp(hardware, "BCM2836") == 0) {
				// pi 2 hardware
				peri_base = BCM2709_PERI_BASE;
				found = 1;
			}
		}
		fclose(fp);
		if (!found) {
			printf("can't find 'Hardware'\n");
			exit(-1);
		}
	}

	gpio_base = peri_base + GPIO_BASE_OFFSET;																																	    

	/* open /dev/mem */
	if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0) {
		printf("can't open /dev/mem\n");
		exit(-1);
	}

	/* mmap GPIO */
	gpio_map = mmap(
			NULL,             //Any adddress in our space will do
			BLOCK_SIZE,       //Map length
			PROT_READ | PROT_WRITE,// Enable reading & writting to mapped memory
			MAP_SHARED,       //Shared with other processes
			mem_fd,           //File to map
			gpio_base         //Offset to GPIO peripheral
	);

	close(mem_fd); //No need to keep mem_fd open after mmap

	if (gpio_map == MAP_FAILED) {
		printf("mmap error %d\n", (int)gpio_map);//errno also set!
		exit(-1);
	}
	
	// Always use volatile pointer!
	gpio = (volatile unsigned *)gpio_map;

} // setup_io

/* Simple Busy-Wait */
void short_wait(void)
{
	int i;
	for (i = 0; i < 150; i++) {
		asm volatile("nop");
	}
} 

void GpioRpiSetPullUpDown(int gpio, int pud)
{
	int clk_offset = PULLUPDNCLK_OFFSET + (gpio / 32);
	int shift = (gpio % 32);

	if (pud == PUD_DOWN) {
		*(gpio_map + PULLUPDN_OFFSET) = (*(gpio_map + PULLUPDN_OFFSET) & ~3) | PUD_DOWN;
	} else if (pud == PUD_UP) {
		*(gpio_map + PULLUPDN_OFFSET) = (*(gpio_map + PULLUPDN_OFFSET) & ~3) | PUD_UP;
	} else { // pud == PUD_OFF
		*(gpio_map + PULLUPDN_OFFSET) &= ~3;
	}
	
	short_wait();
	*(gpio_map + clk_offset) = 1 << shift;
	short_wait();
	*(gpio_map + PULLUPDN_OFFSET) &= ~3;
	*(gpio_map + clk_offset) = 0;
} 

#endif
