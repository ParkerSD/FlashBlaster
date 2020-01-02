#include <atmel_start.h>
#include <driver_examples.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "oled96.h"

#define SSD1306Addr 0b0111100 //8bit ADDR = 0x78 / "01111000", we actually chop off last 0 and use 7bit ADDR = 0b0111100

int main(void)
{
	atmel_start_init();
	
	uint8_t oled64_initbuf[]={0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,0xaf,0x20,0x02};
	
	I2C_Write(0, SSD1306Addr, oled64_initbuf, sizeof(oled64_initbuf)); // init display (abstract into function)
	
	oledFill(0);
	oledWriteString(0,0,"Emily is a Cutie!!!!!",FONT_SMALL);
	//oledFill(1); //for test
	
	for (int i=0; i<64; i++)
	{
		for(int f=0; f<130; f += 5)
		{
			oledSetPixel(127-f, 16+i, 1);
		}
	}

	while (1) 
	{
		
	}
}
