#ifndef _SPI_H_
#define _SPI_H_

/**
@breif SPI control
@author Stephan Ruloff
@date 13.03.2014
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

#include <stdint.h>
#include <unistd.h>

#define MAXPATHNAME    16

//#define TRANSFER_LEN   3
#define TRANSFER_DELAY 5
#define TRANSFER_SPEED 1000000
#define TRANSFER_BPW   8

#define SPI_WRITE_CMD 0x40
#define SPI_READ_CMD 0x41

typedef struct {
    int mFd;
    int mMode;
    int mBitsPerWord;
    int mMaxSpeed; // max speed [Hz]
	int mDevice; // Device ID (Chip-Select)
} Spi;

Spi* SpiCreate(int device);
uint8_t SpiTransfer(Spi* s, uint8_t* txBuffer, uint8_t* rxBuffer, size_t lenBuffer);
void SpiDestroy(Spi* s);

#endif
