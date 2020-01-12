#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_twi.h"
#include "nrfx_twi.h"
#include "nrfx_twim.h"
#include "oled.h"

#define SSD1306Addr 0b0111100

#define TWI_INSTANCE_ID 0 

extern unsigned char ucFont[], ucSmallFont[];
static int iScreenOffset; // current write offset of screen data
static unsigned char ucScreen[1024]; // local copy of the image buffer
static int oled_type, oled_flip;
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


static list_t* singleton;

char item0[] = {"this is string one"}; // TODO: these names should come from file system in flash 
char item1[] = {"this is string two"};
char item2[] = {"this is string three"};


static void oledWriteCommand(unsigned char);

void twi_init(void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = ARDUINO_SCL_PIN,
       .sda                = ARDUINO_SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}

void oled_init(void)
{
    uint8_t oled64_initbuf[]={0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,0xaf,0x20,0x02};

    uint32_t ret;
    ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, oled64_initbuf, sizeof(oled64_initbuf), false);
    APP_ERROR_CHECK(ret);

    clear_display(0); // fill black
}

void oled_test(void)
{
	oledWriteString(0,0,"Emily is a Cutie!!!!!",FONT_SMALL);
	
	for (int i=0; i<64; i++)
	{
		for(int f=0; f<130; f += 5)
		{
			oledSetPixel(127-f, 16+i, 1);
		}
	}
}
int oledInit(int iChannel, int iAddr, int iType, int bFlip, int bInvert)
{
	uint8_t oled64_initbuf[]={0x00,0xae,0xa8,0x3f,0xd3,0x00,0x40,0xa1,0xc8,0xda,0x12,0x81,0xff,0xa4,0xa6,0xd5,0x80,0x8d,0x14,0xaf,0x20,0x02};

	unsigned char uc[4];

	oled_type = iType;
	oled_flip = bFlip;
	
        uint32_t ret;
        ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, oled64_initbuf, sizeof(oled64_initbuf), false);
        APP_ERROR_CHECK(ret);

	if (bInvert)
	{
		uc[0] = 0; // command
		uc[1] = 0xa7; // invert command
                ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, uc, 2, false);
                APP_ERROR_CHECK(ret);
	}
	if (bFlip) // rotate display 180
	{
		uc[0] = 0; // command
		uc[1] = 0xa0;
		ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, uc, 2, false);
		uc[1] = 0xc0;
		ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, uc, 2, false);
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
        uint32_t ret;
        ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, buf, 2, false);
        APP_ERROR_CHECK(ret);

} /* oledWriteCommand() */


static void oledWriteCommand2(unsigned char c, unsigned char d)
{
	unsigned char buf[3];

	buf[0] = 0x00;
	buf[1] = c;
	buf[2] = d;
        uint32_t ret; 
        ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, buf, 3, false);
        APP_ERROR_CHECK(ret);

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
        uint32_t ret; 
        ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, ucTemp, iLen+1, false);
        APP_ERROR_CHECK(ret);
					
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
int clear_display(unsigned char ucData)
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
} /* clear_display() */

void draw_box(int y) // top left corner of full width box = point (0, y) 
{
	for(int f=0; f<128; f++)
	{
		oledSetPixel(f, y, 1); //draw x lines
		oledSetPixel(f, y+20, 1); 
	}
	
	for(int g=0; g<20; g++)
	{
		oledSetPixel(0, y+g, 1); //draw y lines
		oledSetPixel(127, y+g, 1); 
	}
}

void draw_text(int y, char* text) // 0 < y < 8
{   
    oledWriteString(1, y, text, FONT_SMALL);
}

static list_t* new_list(void)
{
    list_t* list = malloc(sizeof(list)); 

    list->item0 = item0; // based on button presses the order will change and items will be rerendered
    list->item1 = item1;
    list->item2 = item2; // NULL for no render
    list->boxPresent = true; 

    return list;

    //x->item0 = flash_fetch(item0); //need to create entire list struct here from file system
    //x->item1 = flash_fetch(item1);
}

void init_list(void)
{
    singleton = new_list();
}


void draw_initial_screen(void)
{   
    draw_box(17);

    oledWriteString(1, 3, singleton->item0, FONT_SMALL);
    oledWriteString(1, 5, singleton->item1, FONT_SMALL);
    oledWriteString(1, 7, singleton->item2, FONT_SMALL);
}


void draw_screen(void) // wrapper for draw_initial_screen
{
    init_list();
    draw_initial_screen();
}

void clear_list(void)
{
    for(int f=2; f<126; f++)
    {
        //clear string one
        oledSetPixel(f, 24, 0);  
        oledSetPixel(f, 25, 0); 
        oledSetPixel(f, 26, 0); 
        oledSetPixel(f, 27, 0); 
        oledSetPixel(f, 28, 0); 
        oledSetPixel(f, 29, 0); 
        oledSetPixel(f, 30, 0); 
        oledSetPixel(f, 31, 0);

        //clear string two
        oledSetPixel(f, 40, 0);
        oledSetPixel(f, 41, 0); 
        oledSetPixel(f, 42, 0);
        oledSetPixel(f, 43, 0); 
        oledSetPixel(f, 44, 0); 
        oledSetPixel(f, 45, 0); 
        oledSetPixel(f, 46, 0); 
        oledSetPixel(f, 47, 0); 

        //clear string three
        oledSetPixel(f, 56, 0); 
        oledSetPixel(f, 57, 0); 
        oledSetPixel(f, 58, 0); 
        oledSetPixel(f, 59, 0); 
        oledSetPixel(f, 60, 0); 
        oledSetPixel(f, 61, 0); 
        oledSetPixel(f, 62, 0);
        oledSetPixel(f, 63, 0); 
    }
    
}

void rerender_screen(int8_t itemHighlighted)
{

    if(itemHighlighted == 0)
    {
        clear_list();
        oledWriteString(1, 3, singleton->item0, FONT_SMALL);
        oledWriteString(1, 5, singleton->item1, FONT_SMALL);
        oledWriteString(1, 7, singleton->item2, FONT_SMALL);
    }
    else if(itemHighlighted == 1)
    {   
        clear_list();
        //oledWriteString(1, 3, singleton->item0, FONT_SMALL);
        oledWriteString(1, 3, singleton->item1, FONT_SMALL);
        oledWriteString(1, 5, singleton->item2, FONT_SMALL);
    }
    else if(itemHighlighted == 2)
    {
        clear_list();
        //oledWriteString(1, 3, singleton->item0, FONT_SMALL);
        //oledWriteString(1, 5, singleton->item1, FONT_SMALL);
        oledWriteString(1, 3, singleton->item2, FONT_SMALL);
    }
}


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