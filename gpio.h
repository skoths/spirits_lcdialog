#ifndef _GPIO_H_
#define _GPIO_H_
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

#include "config.h"

#include <stdint.h>

typedef enum {none, nand_disk, mmc0, timer, heartbeat, default_on} Trigger;
typedef enum {edge_none, edge_rising, edge_falling, edge_both} Edge;
typedef enum {pull_up, pull_down, pull_disable} PullResistor;

uint8_t LedSetTrigger(const char* led, Trigger trigger);
uint8_t LedSetBrightness(const char* led, uint8_t brightness);
uint8_t GpioExport(uint8_t pin);
uint8_t GpioUnexport(uint8_t pin);
uint8_t GpioSetDirection(uint8_t pin, uint8_t isOut);
uint8_t GpioSetValue(uint8_t pin, uint8_t value);
uint8_t GpioGetValue(uint8_t pin, uint8_t *value);
uint8_t GpioSetEdge(uint8_t pin, Edge edge);
uint8_t GpioSetPullResistor(uint8_t pin, PullResistor pull);
uint8_t GpioFdOpen(uint8_t pin, int *fd);
uint8_t GpioFdClose(int fd);

#ifdef RPI_PULLUP

#define PUD_OFF 0
#define PUD_DOWN 1
#define PUD_UP 2 

void GpioRpiSetup(void);
void GpioRpiSetPullUpDown(int gpio, int pud);

#endif

#endif
