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
#include "nrf_drv_spi.h"
#include "oled.h"
#include "ssd1351.h" 
#include "fonts.h" 

#define SSD1306Addr 0b0111100

#define TWI_INSTANCE_ID 1 
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);
extern unsigned char ucFont[], ucSmallFont[];
static int iScreenOffset; // current write offset of screen data
static unsigned char ucScreen[1024]; // local copy of the image buffer
static int oled_type, oled_flip;

list_struct* list_singleton;

system_struct* system_singleton; //TODO init global system stuct based on file system


 uint8_t projectIterator = 0; 
 uint8_t chipIterator = 0; 
 uint8_t fileIterator = 0;
 uint8_t selectedProject; 
 

// Hardcoded Values
char systemFirmware[] = {"FlashBlaster V0"}; 

char projectHeader[] = {"Select Project:"}; 
char* projectNames[10] = {"Quake", "Jeep", "Nikola", "Tesla", "SpaceX", "Rockford Internal", "Subaru", "Nissan", "Ducati", "Toyota"}; // TODO: chop and hardcode as appropriate structs 

char chipHeader[] = {"Select Chip:"}; // or Project Name
char* chipNames[10] = {"Atmel", "Nordic", "STM32", "Cirrus Logic", "NXP", "Renesas", "Pic32", "AVR", "Silicon Labs", "Qualcomm"}; // 

char fileHeader[] = {"Select File:"}; // or Chip Name
char* fileNames[10] = {"pickthis.bin", "promotion.bin", "Bug.bin", "this.hex", "that.bin", "banshee.bin", "pastry.elf", "killme.hex", "reget.bin", "mistakes.hex"}; 




void gpio_init(void) // init gpio for oled drivers 
{
    //nrf_gpio_cfg_output(CS_PIN); //now defined as SS_PIN in SPIO Init
    nrf_gpio_cfg_output(RST_PIN); 
    nrf_gpio_cfg_output(DC_PIN); 
    nrf_gpio_cfg_output(FET_PIN); 
    //nrf_gpio_cfg_output(LDO_EN); // now config as inputs
    //nrf_gpio_cfg_output(BB_EN);
    nrf_gpio_cfg_output(iRST); 
}


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



static void oledWriteCommand(unsigned char);

void oled_init(void)
{
    nrf_gpio_pin_set(FET_PIN); 
    nrf_delay_ms(10); 

    SSD1351_init();
    
    oled_fill_black();
    oled_draw_target(50, COLOR_RED);
    nrf_delay_ms(500);
    oled_shoot_holes(3); 
    //oled_draw_logo();
   
}

void oled_fill_black(void)
{
    SSD1351_fill(COLOR_BLACK);
    SSD1351_update();
}


void oled_draw_logo(void)
{   
    oled_fill_black();
    SSD1351_set_cursor(21,44);
    SSD1351_printf(COLOR_YELLOW, Font_16x26, "FLASH");
    SSD1351_update();

    nrf_delay_ms(500);

    SSD1351_set_cursor(5,64);
    SSD1351_printf(COLOR_AQUA, Font_16x26, "BLASTER");
    SSD1351_update();

    nrf_delay_ms(500);

    //SSD1351_write_command(SSD1351_CMD_INVERTDISPLAY);
    SSD1351_update();

}

void oled_draw_target(uint16_t start_radius, uint16_t start_color)
{                   // x   y    w    h 
   //SSD1351_draw_rect(4, -20, 120, 40, COLOR_BLUE);

   uint16_t current_radius = start_radius; 
   uint16_t current_color = start_color; 
   int16_t Xcenter = 64;
   int16_t Ycenter = 64; 

    
   for(int x = 0; x < 10; x++)
   {
      SSD1351_draw_filled_circle(Xcenter, Ycenter, current_radius - (x * 5) , current_color + (x * 20));
   }

   SSD1351_update(); 
} 

 void oled_shoot_holes(uint8_t radius)
 {
 
//    SSD1351_draw_filled_circle(20, 64, radius, COLOR_BLACK);
//    SSD1351_update();
//    nrf_delay_ms(300);

    SSD1351_draw_filled_circle(40, 50, radius, COLOR_BLACK);
    SSD1351_update();
    nrf_delay_ms(300);

    SSD1351_draw_filled_circle(90, 40, radius, COLOR_BLACK);
    SSD1351_update();
    nrf_delay_ms(300);

    SSD1351_draw_filled_circle(80, 80, radius, COLOR_BLACK);
    SSD1351_update();
    nrf_delay_ms(800);

    SSD1351_draw_filled_circle(64, 64, radius, COLOR_BLACK);
    SSD1351_update();
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
        spi_tx (oled64_initbuf, sizeof(oled64_initbuf));
        //ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, oled64_initbuf, sizeof(oled64_initbuf), false);
        //APP_ERROR_CHECK(ret);

	if (bInvert)
	{
		uc[0] = 0; // command
		uc[1] = 0xa7; // invert command
                spi_tx (uc, 2);
                //ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, uc, 2, false);
                //APP_ERROR_CHECK(ret);
	}
	if (bFlip) // rotate display 180
	{
		uc[0] = 0; // command
		uc[1] = 0xa0;
                spi_tx (uc, 2);
		//ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, uc, 2, false);
		uc[1] = 0xc0;
                spi_tx (uc, 2);
		//ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, uc, 2, false);
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
        spi_tx (buf, 2);
        //uint32_t ret;
        //ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, buf, 2, false);
        //APP_ERROR_CHECK(ret);

} /* oledWriteCommand() */


static void oledWriteCommand2(unsigned char c, unsigned char d)
{
	unsigned char buf[3];

	buf[0] = 0x00;
	buf[1] = c;
	buf[2] = d;
        spi_tx (buf, 3);
        //uint32_t ret; 
        //ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, buf, 3, false);
        //APP_ERROR_CHECK(ret);

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
        spi_tx (ucTemp, iLen+1);
        //uint32_t ret; 
        //ret = nrf_drv_twi_tx(&m_twi, SSD1306Addr, ucTemp, iLen+1, false);
        //APP_ERROR_CHECK(ret);
					
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
	
        list_singleton->boxPresent = false;
        list_singleton->headerPresent = false;

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
        
    list_singleton->boxPresent = true; 
}

void draw_text(int y, char* text) // 0 < y < 8
{   
    oledWriteString(1, y, text, FONT_SMALL);
}

void draw_header(void)
{
    draw_text(1, list_singleton->header);
    list_singleton->headerPresent = true;
}

list_struct* list_new(void)
{
    list_struct* listx = malloc(sizeof(listx)); 
    
    listx->currentList = NULL;
    listx->header = NULL;
    listx->item0 = NULL; // based on button presses the order will change and items will be rerendered
    listx->item1 = NULL; 
    listx->item2 = NULL; 
    listx->item3 = NULL; 
    listx->item4 = NULL; 

    listx->boxPresent = false; 
    listx->headerPresent = false; 

    return listx;

    //x->item0 = flash_fetch(item0); //need to create entire list struct here from file system
    //x->item1 = flash_fetch(item1);
}

void list_init(void)
{
    list_singleton = list_new();
}

//TODO: implement set_current_project() function based on selected item, same for chip and file, render name strings only 

system_struct* system_new(void) //TODO file system, shoulf be constructed in Flash, only NAMES fetched from flash here for later display (CURRENTLY INITING WHOLE FILESSYTEM IN RAM)
{                                                            
    system_struct* systemx = malloc(sizeof(systemx)); 
    
    systemx->systemName = firmware_version_fetch(); 

    if (projectNames[0] != NULL) // replace with Flash value fetch function to determine presence
    {
        systemx->project1 = project_new();
    }
    else
    {
        systemx->project1 = NULL;
    }

        systemx->project2 = project_new();

        systemx->project3 = project_new();

        systemx->project4 = project_new();

        systemx->project5 = project_new();


    return systemx;

}

void system_init(void)
{
    system_singleton = system_new();  // formerly global
}

char* firmware_version_fetch(void)
{
    //fetch FW version from flash 

    return systemFirmware;
}

char* project_name_fetch(void)
{
    //fetch project name from flash 

    return projectNames[projectIterator];

}

char* chip_name_fetch(void)
{
    //fetch chip name from flash 

    return chipNames[chipIterator];

}

char* file_name_fetch(void)
{
    //fetch file name from flash 

    return fileNames[fileIterator];

}



file_struct* file_new(void)
{
    file_struct* filex = malloc(sizeof(filex)); 

    filex->fileName = file_name_fetch(); 
    filex->filePtr = NULL;

    fileIterator++; 
    return filex;
}



chip_struct* chip_new(void) //TODO: render only existing files 
{
    chip_struct* chipx = malloc(sizeof(chipx));

    chipx->chipName = chip_name_fetch(); 

    if (fileNames[0] != NULL) // replace with Flash value fetch function to determine presence
    {
        chipx->file1 = file_new();
    }
    else
    {
        chipx->file1 = NULL;
    }

    if (fileNames[1] != NULL)
    {
        chipx->file2 = file_new();
    }
    else
    {
        chipx->file2 = NULL;
    }
    
    chipIterator++;
    return chipx;
}



project_struct* project_new(void) //TODO: render only existing chips
{
    project_struct* projectx = malloc(sizeof(projectx));

    projectx->projectName = project_name_fetch();

    if (chipNames[0] != NULL) // replace with Flash value fetch function to determine presence
    {
        projectx->chip1 = chip_new();
    }
    else
    {
        projectx->chip1 = NULL;
    }

    if (chipNames[1] != NULL)
    {
        projectx->chip2 = chip_new();
    }
    else
    {
        projectx->chip2 = NULL;
    }
    
    projectIterator++;
    return projectx; 
}



void draw_initial_screen(void) //TODO Problem is here, with passing pointer within struct to pointer within another stuct, not related to global 
{   
    list_singleton->currentList = project;
    list_singleton->header = system_singleton->systemName; 
    list_singleton->item0 = system_singleton->project1->projectName; 
    list_singleton->item1 = system_singleton->project2->projectName;
    list_singleton->item2 = system_singleton->project3->projectName;
    list_singleton->item3 = system_singleton->project4->projectName;
    list_singleton->item4 = system_singleton->project5->projectName;

    draw_box(17);
    draw_header();
    oledWriteString(1, 3, list_singleton->item0, FONT_SMALL);
    oledWriteString(1, 5, list_singleton->item1, FONT_SMALL);
    oledWriteString(1, 7, list_singleton->item2, FONT_SMALL);
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



void rerender_list(int8_t itemHighlighted) // TODO: limit render to amount of existing items
{
    
    if(itemHighlighted == 0)
    {
        clear_list();
        oledWriteString(1, 3, list_singleton->item0, FONT_SMALL);
        oledWriteString(1, 5, list_singleton->item1, FONT_SMALL);
        oledWriteString(1, 7, list_singleton->item2, FONT_SMALL);
    }
    else if(itemHighlighted == 1)
    {   
        clear_list();
        oledWriteString(1, 3, list_singleton->item1, FONT_SMALL);
        oledWriteString(1, 5, list_singleton->item2, FONT_SMALL);
        oledWriteString(1, 7, list_singleton->item3, FONT_SMALL);
    }
    else if(itemHighlighted == 2)
    {
        clear_list();
        oledWriteString(1, 3, list_singleton->item2, FONT_SMALL);
        oledWriteString(1, 5, list_singleton->item3, FONT_SMALL);
        oledWriteString(1, 7, list_singleton->item4, FONT_SMALL);
    }
    else if(itemHighlighted == 3)
    {
        clear_list();
        oledWriteString(1, 3, list_singleton->item3, FONT_SMALL);
        oledWriteString(1, 5, list_singleton->item4, FONT_SMALL);
    }
    else if(itemHighlighted == 4)
    {
        clear_list();
        oledWriteString(1, 3, list_singleton->item4, FONT_SMALL);
    }
}



void rerender_screen(int8_t itemHighlighted, int8_t selectedItem, uint8_t screenStack) 
{   

    switch(screenStack)
    {
        case 0: 
            list_singleton->currentList = project;
            list_singleton->header = system_singleton->systemName; //set initial values for display 
            list_singleton->item0 = system_singleton->project1->projectName; 
            list_singleton->item1 = system_singleton->project2->projectName;
            list_singleton->item2 = system_singleton->project3->projectName;
            list_singleton->item3 = system_singleton->project4->projectName;
            list_singleton->item4 = system_singleton->project5->projectName;
            break;

        case 1: 
            list_singleton->currentList = chip;
            list_singleton->header = chipHeader;
            if(selectedItem == 0)
            {
                selectedProject = 0; 
                list_singleton->item0 = system_singleton->project1->chip1->chipName;
                list_singleton->item1 = system_singleton->project1->chip2->chipName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 1)
            {
                selectedProject = 1; 
                list_singleton->item0 = system_singleton->project2->chip1->chipName;
                list_singleton->item1 = system_singleton->project2->chip2->chipName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 2)
            {
                selectedProject = 2; 
                list_singleton->item0 = system_singleton->project3->chip1->chipName;
                list_singleton->item1 = system_singleton->project3->chip2->chipName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 3)
            {
                selectedProject = 3; 
                list_singleton->item0 = system_singleton->project4->chip1->chipName;
                list_singleton->item1 = system_singleton->project4->chip2->chipName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 4)
            {
                selectedProject = 4; 
                list_singleton->item0 = system_singleton->project5->chip1->chipName;
                list_singleton->item1 = system_singleton->project5->chip2->chipName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            break;

        case 2: 
            list_singleton->currentList = file;
            list_singleton->header = fileHeader;
            if(selectedItem == 0 && selectedProject == 0) // project 1 chip 1 selected
            {
                list_singleton->item0 = system_singleton->project1->chip1->file1->fileName;
                list_singleton->item1 = system_singleton->project1->chip1->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 1 && selectedProject == 0) // project 1 chip 2 selected
            {
                list_singleton->item0 = system_singleton->project1->chip2->file1->fileName;
                list_singleton->item1 = system_singleton->project1->chip2->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 0 && selectedProject == 1) // project 2 chip 1 selected
            {
                list_singleton->item0 = system_singleton->project2->chip1->file1->fileName;
                list_singleton->item1 = system_singleton->project2->chip1->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 1 && selectedProject == 1) // project 2 chip 2 selected
            {
                list_singleton->item0 = system_singleton->project2->chip2->file1->fileName;
                list_singleton->item1 = system_singleton->project2->chip2->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 0 && selectedProject == 2) // project 3 chip 1 selected
            {
                list_singleton->item0 = system_singleton->project3->chip1->file1->fileName;
                list_singleton->item1 = system_singleton->project3->chip1->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 1 && selectedProject == 2) // project 3 chip 2 selected
            {
                list_singleton->item0 = system_singleton->project3->chip2->file1->fileName; //runs out of filenames here
                list_singleton->item1 = system_singleton->project3->chip2->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 0 && selectedProject == 3) // project 4 chip 1 selected
            {
                list_singleton->item0 = system_singleton->project4->chip1->file1->fileName;
                list_singleton->item1 = system_singleton->project4->chip1->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 1 && selectedProject == 3) // project 4 chip 2 selected
            {
                list_singleton->item0 = system_singleton->project4->chip2->file1->fileName;
                list_singleton->item1 = system_singleton->project4->chip2->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 0 && selectedProject == 4) // project 5 chip 1 selected
            {
                list_singleton->item0 = system_singleton->project5->chip1->file1->fileName;
                list_singleton->item1 = system_singleton->project5->chip1->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            else if(selectedItem == 1 && selectedProject == 4) // project 5 chip 2 selected
            {
                list_singleton->item0 = system_singleton->project5->chip2->file1->fileName;
                list_singleton->item1 = system_singleton->project5->chip2->file2->fileName;
                list_singleton->item2 = NULL;
                list_singleton->item3 = NULL;
                list_singleton->item4 = NULL;
            }
            break; 

        default:
            break; 
    }

    rerender_list(itemHighlighted);

    if(list_singleton->boxPresent == false)
    {
        draw_box(17);
    }

    if(list_singleton->headerPresent == false)
    {
        draw_header();
    }
}



unsigned char ucSmallFont[] = {0x00};