
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
#include "button.h" 
#include "system.h"


#define curr_font small_font

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


void clear_screen(void)
{
    SSD1351_fill(COLOR_BLACK);
    SSD1351_update();
    list_singleton->boxPresent = false;
    list_singleton->headerPresent = false;
}

void draw_selection_box(void) // top left corner of full width box = point (0, y) 
{
    //SSD1351_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    SSD1351_draw_rect(2, 45, 124, 30, COLOR_BLUE);
    SSD1351_update();
    list_singleton->boxPresent = true; 
}

void draw_header(void)
{
    SSD1351_set_cursor(1,1);
    SSD1351_printf(COLOR_WHITE, small_font, list_singleton->header);
    SSD1351_update();
    list_singleton->headerPresent = true;
}


void draw_initial_screen(void)
{   
    list_singleton->currentList = project;
    list_singleton->header = projectHeader; 
    list_singleton->item0 = system_singleton->project1->projectName; 
    list_singleton->item1 = system_singleton->project2->projectName;
    list_singleton->item2 = system_singleton->project3->projectName;
    list_singleton->item3 = system_singleton->project4->projectName;
    list_singleton->item4 = system_singleton->project5->projectName;
    
    clear_screen();
    draw_selection_box();
    draw_header();
    SSD1351_set_cursor(10,57);
    SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item0);
    SSD1351_set_cursor(10,83);
    SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item1);
    SSD1351_set_cursor(10,105);
    SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item2);
    SSD1351_update();
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

void clear_list(int8_t itemHighlighted) //TODO: This argument for future optimizing, write over exact text with black
{

      //SSD1351_draw_filled_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
      SSD1351_draw_filled_rect(10, 53, 110, 20, COLOR_BLACK);
      SSD1351_draw_filled_rect(10, 80, 110, 20, COLOR_BLACK);
      SSD1351_draw_filled_rect(10, 105, 110, 20, COLOR_BLACK);

}


void rerender_list(int8_t itemHighlighted, uint8_t screenStack) // TODO: limit render to amount of existing items
{   

    if(itemHighlighted == 0)
    {
        clear_list(itemHighlighted);
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item0);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item1);
        SSD1351_set_cursor(10,105);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item2);
        SSD1351_update();
    }
    else if(itemHighlighted == 1)
    {   
        clear_list(itemHighlighted);
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item1);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item2);
        SSD1351_set_cursor(10,105);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item3);
        SSD1351_update();
    }
    else if(itemHighlighted == 2)
    {
        clear_list(itemHighlighted);
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item2);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item3);
        SSD1351_set_cursor(10,105);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item4);
        SSD1351_update();
    }
    else if(itemHighlighted == 3)
    {
        clear_list(itemHighlighted);
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item3);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item4);
        SSD1351_update();
    }
    else if(itemHighlighted == 4)
    {
        clear_list(itemHighlighted);
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item4);
        SSD1351_update();
    }
}


void rerender_screen(int8_t itemHighlighted, int8_t selectedItem, uint8_t screenStack) 
{   
    switch(screenStack)
    {
        case 0: 
            list_singleton->currentList = project;
            list_singleton->header = projectHeader; //set initial values for display 
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

    rerender_list(itemHighlighted,screenStack);

    if(list_singleton->boxPresent == false)
    {
        draw_selection_box();
    }

    if(list_singleton->headerPresent == false)
    {
        draw_header();
    }
}