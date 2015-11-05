/**
@brief SPI control by Linux kernel driver (/dev/spidev)
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


#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <sys/ioctl.h>

#include "config.h"
#include "spi.h"

Spi* SpiCreate(int device)
{
	Spi *s;
	char path[MAXPATHNAME];
	uint8_t value8;
	uint32_t value32;

	s = (Spi*)malloc(sizeof(Spi));
	
	if (!s) {
		return NULL;
	}
	
	s->mDevice = device;
    if (snprintf(path, MAXPATHNAME, "/dev/spidev%d.%d", SPI_BUS, device) >= MAXPATHNAME) {
		free(s);
		
        return NULL;
    }

    if ((s->mFd = open(path, O_RDWR, 0)) < 0) {
		free(s);
		
        return NULL;
    }

    if (ioctl(s->mFd, SPI_IOC_RD_MODE, &value8) < 0) {
        free(s);
		
		return NULL;
    }
    s->mMode = value8;

    if (ioctl(s->mFd, SPI_IOC_RD_BITS_PER_WORD, &value8) < 0) {
        free(s);
		
		return NULL;
    }
    s->mBitsPerWord = value8;

    if (ioctl(s->mFd, SPI_IOC_RD_MAX_SPEED_HZ, &value32) < 0) {
        free(s);
		
		return NULL;
    }
    s->mMaxSpeed = value32;
	
#if 0
	printf("Fd = %i\n", s->mFd);
	printf("Mode = %i\n", s->mMode);
	printf("BitsPerWord = %i\n", s->mBitsPerWord);
	printf("MaxSpeed = %i\n", s->mMaxSpeed);
#endif
	
	return s;
}

uint8_t SpiTransfer(Spi* s, uint8_t* txBuffer, uint8_t* rxBuffer, size_t lenBuffer)
{
	if (!s) {
		return 0;
	}
    // set up some transfer information
    struct spi_ioc_transfer transfer_buffer = 
    {
        .tx_buf = (unsigned long) txBuffer,
        .rx_buf = (unsigned long) rxBuffer,
        .len = lenBuffer,
        .delay_usecs = TRANSFER_DELAY,
        .speed_hz = TRANSFER_SPEED,
        .bits_per_word = TRANSFER_BPW,
    };

    if (ioctl(s->mFd, SPI_IOC_MESSAGE(1), &transfer_buffer) < 1) {
        return 0;
    }
	
	return 1;
}

void SpiDestroy(Spi *s)
{
	if (s) {
		close(s->mFd);
		free(s);
	}
}
