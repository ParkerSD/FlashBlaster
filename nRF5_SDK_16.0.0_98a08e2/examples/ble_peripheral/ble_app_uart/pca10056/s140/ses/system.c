
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

recents_struct* recents_singleton;
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


recents_struct* recents_init(void)
{
    recents_struct* recentsx = malloc(sizeof(recents_struct));
    recentsx->file0 = NULL;
    recentsx->file1 = NULL;
    recentsx->file2 = NULL;
    recentsx->file3 = NULL;
    return recentsx;
}


void push_file_to_recents(void) //shift recent files, pushing oldest off stack
{
    recents_singleton->file0 = list_singleton->recent; // member recent never assigned value
    recents_singleton->file1 = recents_singleton->file0;
    recents_singleton->file2 = recents_singleton->file1;
    recents_singleton->file3 = recents_singleton->file2;
}


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
   list_singleton->item0 = system_singleton->project_first->project_name;
//   list_singleton->item1 = system_singleton->project_first->project_next->project_name;
//   list_singleton->item2 = system_singleton->project_first->project_next->project_next->project_name;
//   list_singleton->item3 = system_singleton->project_first->project_next->project_next->project_next->project_name;
//   list_singleton->item4 = system_singleton->project_first->project_next->project_next->project_next->project_next->project_name;

    recents_singleton = recents_init();
    
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
    list_struct* listx = malloc(sizeof(list_struct)); 
    
    listx->currentList = NULL;
    listx->header = NULL;
    listx->recent = NULL;
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
    system_struct* systemx = malloc(sizeof(system_struct)); 
    systemx->system_name = firmware_version_fetch(); 
    systemx->project_num = 0;
    systemx->project_first = NULL; //project_init();

    return systemx;
}


void system_init(void)
{
    system_singleton = system_new();  // formerly global
    system_singleton->project_first = project_init();
    system_singleton->project_first->chip_first = chip_init();
    system_singleton->project_first->chip_first->file_first = file_init();
    //system_singleton->project_first->chip_first->file_first->file_name = fileNames[0];
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

//TODO: create file_new, project_new, chip_new functions for adding nodes, add menu item to allow creation, at bottom of menu? 

/*
    chip_struct* chip_parent; 
    uint8_t file_index; 
    file_struct* file_next; 
    char* file_name; 
    uint8_t* file_data; // pointer to program data
*/
file_struct* file_init(void)
{
    file_struct* filex = malloc(sizeof(file_struct)); 
    
    filex->file_name = "Empty"; //file_name_fetch(); 
    filex->file_index = 0;
    filex->file_next = NULL;
    filex->file_data = NULL;
    filex->chip_parent = NULL;

    fileIterator++; 
    return filex;
}


/*
    project_struct* project_parent;
    char* chip_name; 
    uint8_t chip_index; 
    chip_struct* chip_next; 
    uint8_t file_num; //total num of files associated with the chip 
    file_struct* file_first; // pointer to head
*/
chip_struct* chip_init(void) //TODO: render only existing files 
{
    chip_struct* chipx = malloc(sizeof(chip_struct));
    
    chipx->chip_name = "Empty"; //chip_name_fetch(); 
    chipx->chip_index = 0;
    chipx->chip_next = NULL; 
    chipx->file_num = 0;
    // do for files_num in chip, fetch from flash
    chipx->file_first = NULL; //file_init();
    chipx->project_parent = NULL;
    chipIterator++;
    return chipx;
}


    /*
    char* project_name; 
    uint8_t project_index; 
    project_struct* project_next;
    uint8_t chip_num; // total number of chips
    chip_struct* chip_first;
    */

project_struct* project_init(void) //TODO: render only existing chips
{
    project_struct* projectx = malloc(sizeof(project_struct));
    
    projectx->project_name = "Empty"; //project_name_fetch();
    projectx->project_index = 0;
    projectx->project_next = NULL;
    projectx->chip_num = 0;
    // do for chip_num in project, fetch from flash
    projectx->chip_first = NULL; // chip_init();
    
    projectIterator++;
    return projectx; 
}


void clear_list(void) //TODO: This argument for future optimizing, write over exact text with black
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
        clear_list();
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
        clear_list();
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
        clear_list();
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
        clear_list();
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item3);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item4);
        SSD1351_update();
    }
    else if(itemHighlighted == 4)
    {
        clear_list();
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, list_singleton->item4);
        SSD1351_update();
    }
}


void rerender_screen(int8_t itemHighlighted, int8_t selectedItem, uint8_t screenStack) 
{   
    switch(screenStack)
    {
        case 0: //project screen
            list_singleton->currentList = project;
            list_singleton->header = projectHeader; //set initial values for display 
            list_singleton->item0 = system_singleton->project_first->project_name; 
//            list_singleton->item1 = system_singleton->project_first->project_next->project_name;
//            list_singleton->item2 = system_singleton->project_first->project_next->project_next->project_name;
//            list_singleton->item3 = system_singleton->project_first->project_next->project_next->project_next->project_name;
//            list_singleton->item4 = system_singleton->project_first->project_next->project_next->project_next->project_next->project_name;
            break;

        case 1: //chip screen
            list_singleton->currentList = chip;
            list_singleton->header = chipHeader;
            if(selectedItem == 0)
            {
                selectedProject = 0; 
                list_singleton->item0 = system_singleton->project_first->chip_first->chip_name;
//                list_singleton->item1 = system_singleton->project_first->chip_first->chip_next->chip_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
            }
//            else if(selectedItem == 1)
//            {
//                selectedProject = 1; 
//                list_singleton->item0 = system_singleton->project_first->project_next->chip_first->chip_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->chip_first->chip_next->chip_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 2)
//            {
//                selectedProject = 2; 
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->chip_first->chip_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->chip_first->chip_next->chip_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 3)
//            {
//                selectedProject = 3; 
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->project_next->chip_first->chip_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->project_next->chip_first->chip_next->chip_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 4)
//            {
//                selectedProject = 4; 
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->project_next->project_next->chip_first->chip_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->project_next->project_next->chip_first->chip_next->chip_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
            break;

        case 2: //file screen
            list_singleton->currentList = file;
            list_singleton->header = fileHeader;
            if(selectedItem == 0 && selectedProject == 0) // project 1 chip 1 selected
            {
                list_singleton->item0 = system_singleton->project_first->chip_first->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->chip_first->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
            }
//            else if(selectedItem == 1 && selectedProject == 0) // project 1 chip 2 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->chip_first->chip_next->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->chip_first->chip_next->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 0 && selectedProject == 1) // project 2 chip 1 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->project_next->chip_first->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->chip_first->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 1 && selectedProject == 1) // project 2 chip 2 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->project_next->chip_first->chip_next->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->chip_first->chip_next->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 0 && selectedProject == 2) // project 3 chip 1 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->chip_first->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->chip_first->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 1 && selectedProject == 2) // project 3 chip 2 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->chip_first->chip_next->file_first->file_name; //runs out of filenames here
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->chip_first->chip_next->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 0 && selectedProject == 3) // project 4 chip 1 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->project_next->chip_first->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->project_next->chip_first->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 1 && selectedProject == 3) // project 4 chip 2 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->project_next->chip_first->chip_next->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->project_next->chip_first->chip_next->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 0 && selectedProject == 4) // project 5 chip 1 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->project_next->project_next->chip_first->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->project_next->project_next->chip_first->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
//            else if(selectedItem == 1 && selectedProject == 4) // project 5 chip 2 selected
//            {
//                list_singleton->item0 = system_singleton->project_first->project_next->project_next->project_next->project_next->chip_first->chip_next->file_first->file_name;
//                list_singleton->item1 = system_singleton->project_first->project_next->project_next->project_next->project_next->chip_first->chip_next->file_first->file_next->file_name;
//                list_singleton->item2 = NULL;
//                list_singleton->item3 = NULL;
//                list_singleton->item4 = NULL;
//            }
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