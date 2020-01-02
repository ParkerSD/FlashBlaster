// OLED SSD1306 using the I2C interface
// Written by Larry Bank (bitbank@pobox.com)
// Project started 1/15/2017
//
// The I2C writes (through a file handle) can be single or multiple bytes.
// The write mode stays in effect throughout each call to write()
// To write commands to the OLED controller, start a byte sequence with 0x00,
// to write data, start a byte sequence with 0x40,
// The OLED controller is set to "page mode". This divides the display
// into 8 128x8 "pages" or strips. Each data write advances the output
// automatically to the next address. The bytes are arranged such that the LSB
// is the topmost pixel and the MSB is the bottom.
// The font data comes from another source and must be rotated 90 degrees
// (at init time) to match the orientation of the bits on the display memory.
// A copy of the display memory is maintained by this code so that single pixel
// writes can occur without having to read from the display controller.

#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <driver_examples.h>
#include "oled96.h"


#define SSD1306Addr 0b0111100 //8bit ADDR = 0x78 / "01111000", we actually chop off last 0 and use 7bit ADDR = 0b0111100

extern unsigned char ucFont[], ucSmallFont[];
static int iScreenOffset; // current write offset of screen data
static unsigned char ucScreen[1024]; // local copy of the image buffer
static int oled_type, oled_flip;

static void oledWriteCommand(unsigned char);
//
// Opens a file system handle to the I2C device
// Initializes the OLED controller into "page mode"
// Prepares the font data for the orientation of the display
// Returns 0 for success, 1 for failure
//
int oledInit(int iChannel, int iAddr, int iType, int bFlip, int bInvert)
{
	uint8_t oled64_initbuf[]={0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,0xaf,0x20,0x02};

	unsigned char uc[4];

	oled_type = iType;
	oled_flip = bFlip;
	
	I2C_Write(0, SSD1306Addr, oled64_initbuf, sizeof(oled64_initbuf));

	if (bInvert)
	{
		uc[0] = 0; // command
		uc[1] = 0xa7; // invert command
		I2C_Write(0, SSD1306Addr, uc, 2);
	}
	if (bFlip) // rotate display 180
	{
		uc[0] = 0; // command
		uc[1] = 0xa0;
		I2C_Write(0, SSD1306Addr, uc, 2);
		uc[1] = 0xc0;
		I2C_Write(0, SSD1306Addr, uc, 2);
	}
	return 0;
} /* oledInit() */

	// Sends a command to turn off the OLED display
	// Closes the I2C file handle
void oledShutdown()
{
	oledWriteCommand(0xaE); // turn off OLED
}

	// Send a single byte command to the OLED controller
static void oledWriteCommand(unsigned char c)
{
	unsigned char buf[2];

	buf[0] = 0x00; // command introducer
	buf[1] = c;
	I2C_Write(0, SSD1306Addr, buf, 2);

} /* oledWriteCommand() */

static void oledWriteCommand2(unsigned char c, unsigned char d)
{
	unsigned char buf[3];

	buf[0] = 0x00;
	buf[1] = c;
	buf[2] = d;
	I2C_Write(0, SSD1306Addr, buf, 3);

} /* oledWriteCommand2() */

int oledSetContrast(unsigned char ucContrast)
{
	oledWriteCommand2(0x81, ucContrast);
	return 0;
} 
	
	
				
static void oledSetPosition(int x, int y)
{
	iScreenOffset = (y*128)+x;

	oledWriteCommand(0xb0 | y); // go to page Y
	oledWriteCommand(0x00 | (x & 0xf)); // // lower col addr
	oledWriteCommand(0x10 | ((x >> 4) & 0xf)); // upper col addr
}


	// Write a block of pixel data to the OLED
	// Length can be anything from 1 to 1024 (whole display)
static void oledWriteDataBlock(unsigned char *ucBuf, int iLen)
{
	unsigned char ucTemp[129];

	ucTemp[0] = 0x40; // data command
	memcpy(&ucTemp[1], ucBuf, iLen);
	I2C_Write(0, SSD1306Addr, ucTemp, iLen+1); //1024 for whole display?
					
	// Keep a copy in local buffer
	memcpy(&ucScreen[iScreenOffset], ucBuf, iLen);
	iScreenOffset += iLen;
}

	// Set (or clear) an individual pixel
	// The local copy of the frame buffer is used to avoid
	// reading data from the display controller
int oledSetPixel(int x, int y, unsigned char ucColor)
{
	int i;
	unsigned char uc, ucOld;

	i = ((y >> 3) * 128) + x;
	if (i < 0 || i > 1023) // off the screen
		return -1;
	uc = ucOld = ucScreen[i];
	uc &= ~(0x1 << (y & 7));
	if (ucColor)
	{
		uc |= (0x1 << (y & 7));
	}
	if (uc != ucOld) // pixel changed
	{
		oledSetPosition(x, y>>3);
		oledWriteDataBlock(&uc, 1);
	}
	return 0;
} /* oledSetPixel() */
	
	// Draw a string of small (8x8), large (16x24), or very small (6x8)  characters
	// At the given col+row
	// The X position is in character widths (8 or 16)
	// The Y position is in memory pages (8 lines each)
					
int oledWriteString(int x, int y, char *szMsg, int iSize)
{
	int i, iLen;
	unsigned char *s;

	if (iSize < FONT_NORMAL || iSize > FONT_SMALL)
	return -1;

	iLen = strlen(szMsg);
/*	if (iSize == FONT_BIG) // draw 16x32 font
	{
		if (iLen+x > 8) iLen = 8-x;
		if (iLen < 0) return -1;
		x *= 16;
		for (i=0; i<iLen; i++)
		{
			s = &ucFont[9728 + (unsigned char)szMsg[i]*64];
			oledSetPosition(x+(i*16), y);
			oledWriteDataBlock(s, 16);
			oledSetPosition(x+(i*16), y+1);
			oledWriteDataBlock(s+16, 16);
			oledSetPosition(x+(i*16), y+2);
			oledWriteDataBlock(s+32, 16);
			
		}
	}
	else if (iSize == FONT_NORMAL) // draw 8x8 font
	{
		oledSetPosition(x*8, y);
		if (iLen + x > 16) iLen = 16 - x; // can't display it
		if (iLen < 0)return -1;

		for (i=0; i<iLen; i++)
		{
			s = &ucFont[(unsigned char)szMsg[i] * 8];
			oledWriteDataBlock(s, 8); // write character pattern
		}
	}
*/
//	else
//	{
	// support for small font only
		oledSetPosition(x*6, y);
		if (iLen + x > 21) iLen = 21 - x;
		if (iLen < 0) return -1;
		for (i=0; i<iLen; i++)
		{
			s = &ucSmallFont[(unsigned char)szMsg[i]*6];
			oledWriteDataBlock(s, 6);
		}
//	}
	return 0;
} /* oledWriteString() */

// Fill the frame buffer with a byte pattern
// e.g. all off (0x00) or all on (0xff)
int oledFill(unsigned char ucData)
{
	int y;
	unsigned char temp[128];
	int iLines, iCols;

	iLines = 8; // iLines = (oled_type == OLED_128x32 || oled_type == OLED_64x32) ? 4:8;
	iCols = 8; // iCols = (oled_type == OLED_64x32) ? 4:8;

    memset(temp, ucData, 128);
	for (y=0; y<iLines; y++)
	{
		oledSetPosition(0,y); // set to (0,Y)
		oledWriteDataBlock(temp, iCols*16);
	} // for y
	
	return 0;
} /* oledFill() */


unsigned char ucSmallFont[] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x3e,0x45,0x51,0x45,0x3e,0x00,0x3e,0x6b,0x6f,
	0x6b,0x3e,0x00,0x1c,0x3e,0x7c,0x3e,0x1c,0x00,0x18,0x3c,0x7e,0x3c,0x18,0x00,0x30,
	0x36,0x7f,0x36,0x30,0x00,0x18,0x5c,0x7e,0x5c,0x18,0x00,0x00,0x18,0x18,0x00,0x00,
	0x00,0xff,0xe7,0xe7,0xff,0xff,0x00,0x3c,0x24,0x24,0x3c,0x00,0x00,0xc3,0xdb,0xdb,
	0xc3,0xff,0x00,0x30,0x48,0x4a,0x36,0x0e,0x00,0x06,0x29,0x79,0x29,0x06,0x00,0x60,
	0x70,0x3f,0x02,0x04,0x00,0x60,0x7e,0x0a,0x35,0x3f,0x00,0x2a,0x1c,0x36,0x1c,0x2a,
	0x00,0x00,0x7f,0x3e,0x1c,0x08,0x00,0x08,0x1c,0x3e,0x7f,0x00,0x00,0x14,0x36,0x7f,
	0x36,0x14,0x00,0x00,0x5f,0x00,0x5f,0x00,0x00,0x06,0x09,0x7f,0x01,0x7f,0x00,0x22,
	0x4d,0x55,0x59,0x22,0x00,0x60,0x60,0x60,0x60,0x00,0x00,0x14,0xb6,0xff,0xb6,0x14,
	0x00,0x04,0x06,0x7f,0x06,0x04,0x00,0x10,0x30,0x7f,0x30,0x10,0x00,0x08,0x08,0x3e,
	0x1c,0x08,0x00,0x08,0x1c,0x3e,0x08,0x08,0x00,0x78,0x40,0x40,0x40,0x40,0x00,0x08,
	0x3e,0x08,0x3e,0x08,0x00,0x30,0x3c,0x3f,0x3c,0x30,0x00,0x03,0x0f,0x3f,0x0f,0x03,
	0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x06,0x5f,0x06,0x00,0x00,0x07,0x03,0x00,
	0x07,0x03,0x00,0x24,0x7e,0x24,0x7e,0x24,0x00,0x24,0x2b,0x6a,0x12,0x00,0x00,0x63,
	0x13,0x08,0x64,0x63,0x00,0x36,0x49,0x56,0x20,0x50,0x00,0x00,0x07,0x03,0x00,0x00,
	0x00,0x00,0x3e,0x41,0x00,0x00,0x00,0x00,0x41,0x3e,0x00,0x00,0x00,0x08,0x3e,0x1c,
	0x3e,0x08,0x00,0x08,0x08,0x3e,0x08,0x08,0x00,0x00,0xe0,0x60,0x00,0x00,0x00,0x08,
	0x08,0x08,0x08,0x08,0x00,0x00,0x60,0x60,0x00,0x00,0x00,0x20,0x10,0x08,0x04,0x02,
	0x00,0x3e,0x51,0x49,0x45,0x3e,0x00,0x00,0x42,0x7f,0x40,0x00,0x00,0x62,0x51,0x49,
	0x49,0x46,0x00,0x22,0x49,0x49,0x49,0x36,0x00,0x18,0x14,0x12,0x7f,0x10,0x00,0x2f,
	0x49,0x49,0x49,0x31,0x00,0x3c,0x4a,0x49,0x49,0x30,0x00,0x01,0x71,0x09,0x05,0x03,
	0x00,0x36,0x49,0x49,0x49,0x36,0x00,0x06,0x49,0x49,0x29,0x1e,0x00,0x00,0x6c,0x6c,
	0x00,0x00,0x00,0x00,0xec,0x6c,0x00,0x00,0x00,0x08,0x14,0x22,0x41,0x00,0x00,0x24,
	0x24,0x24,0x24,0x24,0x00,0x00,0x41,0x22,0x14,0x08,0x00,0x02,0x01,0x59,0x09,0x06,
	0x00,0x3e,0x41,0x5d,0x55,0x1e,0x00,0x7e,0x11,0x11,0x11,0x7e,0x00,0x7f,0x49,0x49,
	0x49,0x36,0x00,0x3e,0x41,0x41,0x41,0x22,0x00,0x7f,0x41,0x41,0x41,0x3e,0x00,0x7f,
	0x49,0x49,0x49,0x41,0x00,0x7f,0x09,0x09,0x09,0x01,0x00,0x3e,0x41,0x49,0x49,0x7a,
	0x00,0x7f,0x08,0x08,0x08,0x7f,0x00,0x00,0x41,0x7f,0x41,0x00,0x00,0x30,0x40,0x40,
	0x40,0x3f,0x00,0x7f,0x08,0x14,0x22,0x41,0x00,0x7f,0x40,0x40,0x40,0x40,0x00,0x7f,
	0x02,0x04,0x02,0x7f,0x00,0x7f,0x02,0x04,0x08,0x7f,0x00,0x3e,0x41,0x41,0x41,0x3e,
	0x00,0x7f,0x09,0x09,0x09,0x06,0x00,0x3e,0x41,0x51,0x21,0x5e,0x00,0x7f,0x09,0x09,
	0x19,0x66,0x00,0x26,0x49,0x49,0x49,0x32,0x00,0x01,0x01,0x7f,0x01,0x01,0x00,0x3f,
	0x40,0x40,0x40,0x3f,0x00,0x1f,0x20,0x40,0x20,0x1f,0x00,0x3f,0x40,0x3c,0x40,0x3f,
	0x00,0x63,0x14,0x08,0x14,0x63,0x00,0x07,0x08,0x70,0x08,0x07,0x00,0x71,0x49,0x45,
	0x43,0x00,0x00,0x00,0x7f,0x41,0x41,0x00,0x00,0x02,0x04,0x08,0x10,0x20,0x00,0x00,
	0x41,0x41,0x7f,0x00,0x00,0x04,0x02,0x01,0x02,0x04,0x00,0x80,0x80,0x80,0x80,0x80,
	0x00,0x00,0x03,0x07,0x00,0x00,0x00,0x20,0x54,0x54,0x54,0x78,0x00,0x7f,0x44,0x44,
	0x44,0x38,0x00,0x38,0x44,0x44,0x44,0x28,0x00,0x38,0x44,0x44,0x44,0x7f,0x00,0x38,
	0x54,0x54,0x54,0x08,0x00,0x08,0x7e,0x09,0x09,0x00,0x00,0x18,0xa4,0xa4,0xa4,0x7c,
	0x00,0x7f,0x04,0x04,0x78,0x00,0x00,0x00,0x00,0x7d,0x40,0x00,0x00,0x40,0x80,0x84,
	0x7d,0x00,0x00,0x7f,0x10,0x28,0x44,0x00,0x00,0x00,0x00,0x7f,0x40,0x00,0x00,0x7c,
	0x04,0x18,0x04,0x78,0x00,0x7c,0x04,0x04,0x78,0x00,0x00,0x38,0x44,0x44,0x44,0x38,
	0x00,0xfc,0x44,0x44,0x44,0x38,0x00,0x38,0x44,0x44,0x44,0xfc,0x00,0x44,0x78,0x44,
	0x04,0x08,0x00,0x08,0x54,0x54,0x54,0x20,0x00,0x04,0x3e,0x44,0x24,0x00,0x00,0x3c,
	0x40,0x20,0x7c,0x00,0x00,0x1c,0x20,0x40,0x20,0x1c,0x00,0x3c,0x60,0x30,0x60,0x3c,
	0x00,0x6c,0x10,0x10,0x6c,0x00,0x00,0x9c,0xa0,0x60,0x3c,0x00,0x00,0x64,0x54,0x54,
	0x4c,0x00,0x00,0x08,0x3e,0x41,0x41,0x00,0x00,0x00,0x00,0x77,0x00,0x00,0x00,0x00,
	0x41,0x41,0x3e,0x08,0x00,0x02,0x01,0x02,0x01,0x00,0x00,0x3c,0x26,0x23,0x26,0x3c};