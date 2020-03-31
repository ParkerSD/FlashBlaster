
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
#include "flash.h"


#define ADDR_NUM_PROJECTS 0
#define ADDR_PROJECT_PTR_FIRST 4

//flash sizes and offsets 
#define WORD_SIZE 4
#define MAX_STRING_SIZE 16

#define DIRECTORY_OFFSET 32
#define PROJECT_SECTOR_OFFSET 480 //NOTE this will change


#define PROJECT_HEADER_OFFSET 20 //chip size without pointers
#define MAX_PROJECT_SIZE 128 // allows for 27 chips
#define CHIP_LIST_SIZE 108 //chip list of project

#define CHIP_HEADER_OFFSET 24 // ship size without pointers
#define MAX_CHIP_SIZE 128
#define FILE_LIST_SIZE 108 //chip list of project





#define SYS_ system_singleton->
#define L_ list_singleton->
#define REC_ recents_singleton-> 

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
    recents_struct* recentsX = malloc(sizeof(recents_struct));
    recentsX->file0 = NULL;
    recentsX->file1 = NULL;
    recentsX->file2 = NULL;
    recentsX->file3 = NULL;
    return recentsX;
}


void push_file_to_recents(void) //shift recent files, pushing oldest off stack
{
    REC_ file0 = L_ recent; // member recent never assigned value
    REC_ file1 = REC_ file0;
    REC_ file2 = REC_ file1;
    REC_ file3 = REC_ file2;
}


void clear_screen(void)
{
    SSD1351_fill(COLOR_BLACK);
    SSD1351_update();
    L_ boxPresent = false;
    L_ headerPresent = false;
}


void draw_selection_box(void) // top left corner of full width box = point (0, y) 
{
    //SSD1351_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    SSD1351_draw_rect(2, 45, 124, 30, COLOR_BLUE);
    SSD1351_update();
    L_ boxPresent = true; 
}


void draw_header(void)
{
    SSD1351_set_cursor(1,1);
    SSD1351_printf(COLOR_WHITE, small_font, L_ header);
    SSD1351_update();
    L_ headerPresent = true;
}


void draw_initial_screen(void)
{   
   L_ currentList = project;
   L_ header = projectHeader; 
   L_ item0 = SYS_ project_first->project_name;
   L_ item1 = SYS_ project_first->project_next->project_name;
   L_ item2 = SYS_ project_first->project_next->project_next->project_name;
// L_ item3 = SYS_ project_first->project_next->project_next->project_next->project_name;
// L_ item4 = SYS_ project_first->project_next->project_next->project_next->project_next->project_name;

    recents_singleton = recents_init();
    
    clear_screen();
    draw_selection_box();
    draw_header();
    SSD1351_set_cursor(10,57);
    SSD1351_printf(COLOR_WHITE, curr_font, L_ item0);
    SSD1351_set_cursor(10,83);
    SSD1351_printf(COLOR_WHITE, curr_font, L_ item1);
    SSD1351_set_cursor(10,105);
    SSD1351_printf(COLOR_WHITE, curr_font, L_ item2);
    SSD1351_update();
}


list_struct* list_new(void)
{
    list_struct* listX = malloc(sizeof(list_struct)); 
    
    listX->currentList = NULL;
    listX->header = NULL;
    listX->recent = NULL;
    listX->item0 = NULL; // based on button presses the order will change and items will be rerendered
    listX->item1 = NULL; 
    listX->item2 = NULL; 
    listX->item3 = NULL; 
    listX->item4 = NULL; 

    listX->boxPresent = false; 
    listX->headerPresent = false; 

    return listX;
}


void list_init(void)
{
    list_singleton = list_new();
}

//TODO: implement set_current_project() function based on selected item, same for chip and file, render name strings only 

system_struct* system_new(void) //TODO file system, shoulf be constructed in Flash, only NAMES fetched from flash here for later display (CURRENTLY INITING WHOLE FILESSYTEM IN RAM)
{                                                            
    system_struct* systemX = malloc(sizeof(system_struct)); 
    systemX->system_name = firmware_version_fetch(); 
    systemX->projects_total = NULL;
    systemX->project_first = NULL; //project_create();
    systemX->project_curr = 0;

    return systemX;
}


uint32_t bytes_to_word(uint8_t* bytes, uint32_t word) // bytes stored MSB 
{
    word = *bytes << 24 | *(bytes + 1) << 16 | *(bytes + 2) << 8 | *(bytes + 3);

    return word; 
}

void chips_sync(int8_t selectedItem)
{
    //index project                                        
    project_struct* project_selected = project_list_index(selectedItem); // NOTE: passing int8_t argument to int parameter
    uint16_t total_chips = project_selected->chip_num;
    uint32_t total_chip_list_size = total_chips * WORD_SIZE;

    if(total_chips != 0)
    { 
        // get chip list 
        uint32_t chip_list_address = project_selected->chip_list_addr;  
        char* chip_list_buffer = malloc(sizeof(char[total_chip_list_size]));
        flash_read(chip_list_buffer, chip_list_address, total_chip_list_size);
        
        //create first chip
        uint32_t chip_addr0 = *chip_list_buffer << 24 | *(chip_list_buffer + 1) << 16 | *(chip_list_buffer + 2) << 8 | *(chip_list_buffer + 3);
        char* chip_buffer = malloc(sizeof(char[CHIP_HEADER_OFFSET]));  //TODO FREE
        flash_read(chip_buffer, chip_addr0, CHIP_HEADER_OFFSET);
        project_selected->chip_first = chip_new(chip_buffer, project_selected); // create first project(head of list) 

        // create remainder of projects, first proejct already made 
        for(int i = 0; i < total_chips; i++) //NOTE i=0 maybe better here
        {   
            // convert 4 bytes into 32bit number 
            uint32_t chip_addr = *(chip_list_buffer + (i*4)) << 24 | *(chip_list_buffer + 1 + (i*4)) << 16 | *(chip_list_buffer + 2 + (i*4)) << 8 | *(chip_list_buffer + 3 + (i*4));
            flash_read(chip_buffer, chip_addr, CHIP_HEADER_OFFSET);// read project data 
            chip_new(chip_buffer, project_selected);
        }
    }
    else
    {
        project_selected->chip_first = chip_create(); 
        project_selected->chip_first->file_first = file_create();
    }
    
    // read num chips from selected item project address 
    // buffer all chip addresses 
    // buffer each chip and create structs
    // assign chips to each chip_next in order 
}


chip_struct* chip_list_index(int index, project_struct* project_curr) 
{ 
    chip_struct* chipN;
    chipN = project_curr->chip_first; 

    for(int i = 1; i < index; i++) // minus 1 loop since file_first was already factored in
    {
        chipN = chipN->chip_next; 
    }
    return chipN; 
}
/*
    char* chip_name; 
    chip_struct* chip_next; 
    uint16_t chip_index; 
    uint16_t file_num; //total num of files associated with the chip 
    uint32_t chip_type_id;
    file_struct* file_first; // pointer to head
    project_struct* project_parent;
    uint32_t file_list_addr;
*/
chip_struct* chip_new(char* data, project_struct* project_curr)
{  
    chip_struct* chipY = chip_create(); 
    chipY->chip_name = name_fetch(data);
    chipY->chip_type_id = *(data+16) << 24 | *(data+17) << 16 | *(data+18) << 8 | *(data+19);
    chipY->file_num = *(data+20) << 24 | *(data+21) << 16 | *(data+22) << 8 | *(data+23); 
    chipY->project_parent = project_curr; 

    chipY->file_first = NULL;
    chipY->chip_next = NULL;
    
    //TODO assign file_list_addr to chip based on address offsets, chipY->file_list_addr = DIRECTORY_OFFSET + PROJECT_SECTOR_OFFSET + 

    if(project_curr->chip_curr != 0) //push node, add to project_next
    {  
        int olderSiblingIndex = (project_curr->chip_curr - 1);  
        chip_struct* chipZ = chip_list_index(olderSiblingIndex, project_curr);    //index olderSibling and assign project to project_next node of last created project, older sibling is at index of SYS_ project_num
        chipZ->chip_next = chipY;  
    }

    chipY->chip_index = project_curr->chip_curr; // NOTE: POSSIBLE BUG HERE, should index from zero
    project_curr->chip_curr += 1; 

    return chipY; 
}


void projects_sync(void) //sync projects from flash
{
    //determine number of projects from directory value
    uint8_t* num_projects = malloc(sizeof(int)); //TODO FREE
    flash_read(num_projects, ADDR_NUM_PROJECTS, WORD_SIZE); // read num projects // flash_read(uint8_t* buffer_rx, uint32_t start_addr, size_t DATA_SIZE_BYTES)
    uint32_t project_count = bytes_to_word(num_projects, project_count); // function to convert byte array to 32bit word 
    SYS_ projects_total = project_count; // populate system struct with project num value from flash
    free(num_projects);

    if(project_count != 0)
    {   
        // buffer all project pointers 
        uint32_t total_projects_length = project_count * WORD_SIZE; 
        uint8_t project_addr[total_projects_length]; // array sized for all project addresses 
        flash_read(project_addr, ADDR_PROJECT_PTR_FIRST, total_projects_length); // buffer for all project addresses
        
        // create first project in RAM, extract to function
        char* project_buffer = malloc(sizeof(char[PROJECT_HEADER_OFFSET])); //TODO FREE,  name string plus num_chips malloc 
        uint32_t current_addr = project_addr[0] << 24 | project_addr[1] << 16 | project_addr[2] << 8 | project_addr[3]; // shift bytes into 32bit int
        flash_read(project_buffer, current_addr, PROJECT_HEADER_OFFSET);
        SYS_ project_first = project_new(project_buffer); // create first project(head of list) 

        // create remainder of projects, first project already made 
        for(int i = 0; i < project_count; i++) 
        {   
            // convert 4 bytes into 32bit number 
            current_addr = project_addr[0 + (i*4)]  << 24 | project_addr[1 + (i*4)] << 16 | project_addr[2 + (i*4)] << 8 | project_addr[3 + (i*4)];
            flash_read(project_buffer, current_addr, PROJECT_HEADER_OFFSET);// read project data 
            project_new(project_buffer);
        }
    }
    else
    {
        SYS_ project_first = project_create(); // create empty project - chip - file structures
        SYS_ project_first->chip_first = chip_create();
        SYS_ project_first->chip_first->file_first = file_create();
    }
}


project_struct* project_list_index(int index) 
{ 
    project_struct* projectN;
    projectN = SYS_ project_first; 

    for(int i = 1; i < index; i++) // minus 1 loop since file_first was already factored in
    {
        projectN = projectN->project_next; 
    }
    return projectN; 
}


project_struct* project_new(char* data) //input 24 byte project data, string, num_chips, and chip_first address
{
    project_struct* projectY = project_create(); //create project
    projectY->project_name = name_fetch(data);  // send in payload, fetch from flash
    projectY->chip_num = *(data+16) << 24 | *(data+17) << 16 | *(data+18) << 8 | *(data+19);//TODO: chip_num_fetch(data) - last 4 bytes of 20 bytes;
    projectY->project_next = NULL;
    projectY->chip_first = NULL; 
    projectY->chip_list_addr = DIRECTORY_OFFSET + PROJECT_HEADER_OFFSET + (SYS_ project_curr * MAX_PROJECT_SIZE); //was *(data+20) << 24 | *(data+21) << 16 | *(data+22) << 8 | *(data+23);// NOTE MAKE ACTUAL FLASH ADDRESS OF FIRST CHIP BASED ON MAX PROJECT LENGTH 
    projectY->chip_curr = 0;

    if(SYS_ project_curr != 0) //push node, add to project_next
    {  
        int olderSiblingIndex = (SYS_ project_curr - 1);  
        project_struct* projectZ = project_list_index(olderSiblingIndex);    //index olderSibling and assign project to project_next node of last created project, older sibling is at index of SYS_ project_num
        projectZ->project_next = projectY;  
    }
  
    projectY->project_index = SYS_ project_curr; // NOTE: POSSIBLE BUG HERE, should index from zero
    SYS_ project_curr += 1; 

    return projectY; //should return newly created to global
}

void flash_init(void)
{   
    // ADDR_NUM_PROJECTS 0
    // ADDR_PROJECT_PTR_FIRST 4

    // init flash for test below 
    flash_erase(0, NRF_QSPI_ERASE_LEN_4KB); // erase all 

    //DIRECTORY                                                           //288
    uint8_t directory[32] = {0, 0, 0, 3, 0, 0, 0, 32, 0, 0, 0, 160, 0, 0, 1, 32, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0}; // num projects: 1, first project address: 32
    flash_write(directory, 0, 32); // write test directory

    //NOTE PROJECT 1 - 24 bytes (including first chip addr) 
    uint8_t project_string_test0[] = {"project0"}; //long form, single write {'p', 'r', 'o', 'j', 'e', 'c', 't', '0', 0, 0, 0, 1, 0, 0, 2, 0}; 
    uint8_t chip_num_test[WORD_SIZE] = {0, 0, 0, 3};
    uint8_t chip_ptr_first_test[WORD_SIZE] = {0, 0, 2, 0}; //address 512 when bit shifted
    uint8_t chip_ptr_sec_test[WORD_SIZE] = {0, 0, 2, 128}; //address 640 when bit shifted
    uint8_t chip_ptr_three_test[WORD_SIZE] = {0, 0, 3, 0}; //address 768 when bit shifted
    flash_write(project_string_test0, 32, sizeof(char[MAX_STRING_SIZE])); // name sting 16 bytes long 
    flash_write(chip_num_test, 48, WORD_SIZE); // chip num 4 bytes, address 48 (after string) 
    flash_write(chip_ptr_first_test, 52, WORD_SIZE); // write first chip ptr after chip_num value of first project
    flash_write(chip_ptr_sec_test, 56, WORD_SIZE);
    flash_write(chip_ptr_three_test, 60, WORD_SIZE);
    //CHIP 0 - 28 bytes (including first file addr) 
    uint8_t chip_string_test0[] = {"chip0"}; //16 bytes
    uint8_t chip_type_ID[WORD_SIZE] = {0, 0, 7, 0}; 
    uint8_t files_num[WORD_SIZE] = {0, 0, 7, 0}; 
    uint8_t files_first_addr[WORD_SIZE] = {0, 0, 255, 255}; 
    flash_write(chip_string_test0, 512, sizeof(char[MAX_STRING_SIZE]));
    flash_write(chip_type_ID, 528, WORD_SIZE);
    flash_write(files_num, 532, WORD_SIZE);
    flash_write(files_first_addr, 536, WORD_SIZE);
    //CHIP 1
    uint8_t chip_string_test1[] = {"chip1"}; //16 bytes
    uint8_t chip_type_ID1[WORD_SIZE] = {0, 0, 7, 0}; 
    uint8_t files_num1[WORD_SIZE] = {0, 0, 7, 0}; 
    uint8_t files_first_addr1[WORD_SIZE] = {0, 0, 255, 255}; 
    flash_write(chip_string_test1, 640, sizeof(char[MAX_STRING_SIZE]));
    flash_write(chip_type_ID1, 656, WORD_SIZE);
    flash_write(files_num1, 660, WORD_SIZE);
    flash_write(files_first_addr1, 664, WORD_SIZE);
    //CHIP 2
    uint8_t chip_string_test2[] = {"chip2"}; //16 bytes
    uint8_t chip_type_ID2[WORD_SIZE] = {0, 0, 7, 0}; 
    uint8_t files_num2[WORD_SIZE] = {0, 0, 7, 0}; 
    uint8_t files_first_addr2[WORD_SIZE] = {0, 0, 255, 255}; 
    flash_write(chip_string_test2, 768, sizeof(char[MAX_STRING_SIZE]));
    flash_write(chip_type_ID2, 784, WORD_SIZE);
    flash_write(files_num2, 788, WORD_SIZE);
    flash_write(files_first_addr2, 792, WORD_SIZE);
    
    //NOTE PROJECT 2
    uint8_t project_string_test1[] = {"project1"};
    uint8_t chip_num_test1[WORD_SIZE] = {0, 0, 0, 0};
    flash_write(project_string_test1, 160, sizeof(char[MAX_STRING_SIZE])); // name string 16 bytes long 
    flash_write(chip_num_test1, 176, WORD_SIZE);
    
    //NOTE PROJECT 3 
    uint8_t project_string_test2[] = {"project2"};
    uint8_t chip_num_test2[WORD_SIZE] = {0, 0, 0, 0};
    flash_write(project_string_test2, 288, sizeof(char[MAX_STRING_SIZE])); // name string 16 bytes long 
    flash_write(chip_num_test2, 304, WORD_SIZE);
    
}


void system_init(void) // create global system struct and read directory info from flash, create project structs 
{   
    system_singleton = system_new();  
    flash_init(); 
    projects_sync(); 
}


char* firmware_version_fetch(void)
{
    //fetch FW version from flash 
    return systemFirmware;
}


char* name_fetch(char* data)
{
    char* name_string = malloc(sizeof(char[MAX_STRING_SIZE])); //TODO FREE 
    memcpy(name_string, data, MAX_STRING_SIZE); //16 bytes = max string length 
    return name_string;
}


chip_struct* chip_parent_fetch(void)
{
    // TODO: fetch chip_parent member from newly create file
    // read external flash here
    return NULL;
}

project_struct* project_parent_fetch(void)
{
    // TODO: fetch project_parent member from newly created chip
    // read external flash here
    return NULL; 
}


//TODO: create file_new, project_new, chip_new functions for adding nodes, add menu item to allow creation, at bottom of menu? 

file_struct* file_list_index(file_struct* file, int index) 
{ 
    file_struct* fileN;
    fileN = file->chip_parent->file_first; 

    for(int i = 1; i < index; i++) // minus 1 loop since file_first was already factored in
    {
        fileN = fileN->file_next; 
    }
    return fileN; 
}


file_struct* file_new(void) //fetches file from Flash and adds to linked list in RAM 
{   
    file_struct* fileY = file_create(); //create file
    
    fileY->chip_parent = chip_parent_fetch(); // data needs to be sent in payload and fetched from flash
    fileY->file_name = NULL;//name_fetch();  // send in payload, fetch from flash
    fileY->file_next = NULL;
    fileY->file_data = NULL; // send in payload, pointer to data in flash 
    
    fileY->file_index = fileY->chip_parent->file_num; // update file index
    fileY->chip_parent->file_num += 1; //increment file counter on chip_parent
    

    if(fileY->file_index != 0) //push node, add to file_next of last file
    {   
        int olderSiblingIndex = fileY->file_index - 1;  // older sibling is one made previously
        file_struct* fileZ = file_list_index(fileY, olderSiblingIndex); 
        fileZ->file_next = fileY; //assign newly created file as file_next of last file
    }

    return fileY; //should return newly created to global
}


/*
    chip_struct* chip_parent; 
    uint8_t file_index; 
    file_struct* file_next; 
    char* file_name; 
    uint8_t* file_data; // pointer to program data
*/
file_struct* file_create(void)
{
    file_struct* fileX = malloc(sizeof(file_struct)); 
    
    fileX->file_name = "Empty";
    fileX->file_index = NULL;
    fileX->file_next = NULL;
    fileX->file_data = NULL;
    fileX->chip_parent = NULL;
 
    return fileX;
}


/*
    project_struct* project_parent;
    char* chip_name; 
    uint8_t chip_index; 
    chip_struct* chip_next; 
    uint8_t file_num; //total num of files associated with the chip 
    file_struct* file_first; // pointer to head
*/
chip_struct* chip_create(void) //TODO: render only existing files 
{
    chip_struct* chipX = malloc(sizeof(chip_struct));
    
    chipX->chip_name = "Empty"; //chip_name_fetch(); 
    chipX->chip_index = NULL;
    chipX->chip_next = NULL; 
    chipX->file_num = 0;
    // do for files_num in chip, fetch from flash
    chipX->file_first = NULL; //file_init();
    chipX->project_parent = NULL;

    chipIterator++;
    return chipX;
}


    /*
    char* project_name; 
    uint8_t project_index; 
    project_struct* project_next;
    uint8_t chip_num; // total number of chips
    chip_struct* chip_first;
    */

project_struct* project_create(void) //TODO: render only existing chips
{
    project_struct* projectX = malloc(sizeof(project_struct));
    
    projectX->project_name = "Empty"; //project_name_fetch();
    projectX->project_index = NULL;
    projectX->project_next = NULL;
    projectX->chip_num = 0;
    // do for chip_num in project, fetch from flash
    projectX->chip_first = NULL; // chip_create();
    
    projectIterator++;
    return projectX; 
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
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item0);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item1);
        SSD1351_set_cursor(10,105);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item2);
        SSD1351_update();
    }
    else if(itemHighlighted == 1)
    {   
        clear_list();
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item1);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item2);
        SSD1351_set_cursor(10,105);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item3);
        SSD1351_update();
    }
    else if(itemHighlighted == 2)
    {
        clear_list();
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item2);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item3);
        SSD1351_set_cursor(10,105);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item4);
        SSD1351_update();
    }
    else if(itemHighlighted == 3)
    {
        clear_list();
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item3);
        SSD1351_set_cursor(10,83);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item4);
        SSD1351_update();
    }
    else if(itemHighlighted == 4)
    {
        clear_list();
        SSD1351_set_cursor(10,57);
        SSD1351_printf(COLOR_WHITE, curr_font, L_ item4);
        SSD1351_update();
    }
}


void rerender_screen(int8_t itemHighlighted, int8_t selectedItem, uint8_t screenStack) 
{   
    switch(screenStack)
    {
        case project_screen: 
            L_ currentList = project;
            L_ header = projectHeader; //set initial values for display 
            L_ item0 = SYS_ project_first->project_name; 
            L_ item1 = SYS_ project_first->project_next->project_name;
            L_ item2 = SYS_ project_first->project_next->project_next->project_name;
//            L_ item3 = SYS_ project_first->project_next->project_next->project_next->project_name;
//            L_ item4 = SYS_ project_first->project_next->project_next->project_next->project_next->project_name;
            break;

        case chip_screen: //chip screen
            L_ currentList = chip;
            L_ header = chipHeader;
            if(selectedItem == 0)
            {
                selectedProject = 0; 
                L_ item0 = SYS_ project_first->chip_first->chip_name;
                L_ item1 = SYS_ project_first->chip_first->chip_next->chip_name;
                L_ item2 = SYS_ project_first->chip_first->chip_next->chip_next->chip_name;
                L_ item3 = NULL;
                L_ item4 = NULL;
            }
//            else if(selectedItem == 1)
//            {
//                selectedProject = 1; 
//                L_ item0 = SYS_ project_first->project_next->chip_first->chip_name;
//                L_ item1 = SYS_ project_first->project_next->chip_first->chip_next->chip_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 2)
//            {
//                selectedProject = 2; 
//                L_ item0 = SYS_ project_first->project_next->project_next->chip_first->chip_name;
//                L_ item1 = SYS_ project_first->project_next->project_next->chip_first->chip_next->chip_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 3)
//            {
//                selectedProject = 3; 
//                L_ item0 = SYS_ project_first->project_next->project_next->project_next->chip_first->chip_name;
//                L_ item1 = SYS_ project_first->project_next->project_next->project_next->chip_first->chip_next->chip_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 4)
//            {
//                selectedProject = 4; 
//                L_ item0 = SYS_ project_first->project_next->project_next->project_next->project_next->chip_first->chip_name;
//                L_ item1 = SYS_ project_first->project_next->project_next->project_next->project_next->chip_first->chip_next->chip_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
            break;

        case file_screen: //file screen
            L_ currentList = file;
            L_ header = fileHeader;
            if(selectedItem == 0 && selectedProject == 0) // project 1 chip 1 selected
            {
                L_ item0 = SYS_ project_first->chip_first->file_first->file_name;
//                L_ item1 = SYS_ project_first->chip_first->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
            }
//            else if(selectedItem == 1 && selectedProject == 0) // project 1 chip 2 selected
//            {
//                L_ item0 = SYS_ project_first->chip_first->chip_next->file_first->file_name;
//                L_ item1 = SYS_ project_first->chip_first->chip_next->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 0 && selectedProject == 1) // project 2 chip 1 selected
//            {
//                L_ item0 = SYS_ project_first->project_next->chip_first->file_first->file_name;
//                L_ item1 = SYS_ project_first->project_next->chip_first->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 1 && selectedProject == 1) // project 2 chip 2 selected
//            {
//                L_ item0 = SYS_ project_first->project_next->chip_first->chip_next->file_first->file_name;
//                L_ item1 = SYS_ project_first->project_next->chip_first->chip_next->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 0 && selectedProject == 2) // project 3 chip 1 selected
//            {
//                L_ item0 = SYS_ project_first->project_next->project_next->chip_first->file_first->file_name;
//                L_ item1 = SYS_ project_first->project_next->project_next->chip_first->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 1 && selectedProject == 2) // project 3 chip 2 selected
//            {
//                L_ item0 = SYS_ project_first->project_next->project_next->chip_first->chip_next->file_first->file_name; //runs out of filenames here
//                L_ item1 = SYS_ project_first->project_next->project_next->chip_first->chip_next->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 0 && selectedProject == 3) // project 4 chip 1 selected
//            {
//                L_ item0 = SYS_ project_first->project_next->project_next->project_next->chip_first->file_first->file_name;
//                L_ item1 = SYS_ project_first->project_next->project_next->project_next->chip_first->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 1 && selectedProject == 3) // project 4 chip 2 selected
//            {
//                L_ item0 = SYS_ project_first->project_next->project_next->project_next->chip_first->chip_next->file_first->file_name;
//                L_ item1 = SYS_ project_first->project_next->project_next->project_next->chip_first->chip_next->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 0 && selectedProject == 4) // project 5 chip 1 selected
//            {
//                L_ item0 = SYS_ project_first->project_next->project_next->project_next->project_next->chip_first->file_first->file_name;
//                L_ item1 = SYS_ project_first->project_next->project_next->project_next->project_next->chip_first->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
//            else if(selectedItem == 1 && selectedProject == 4) // project 5 chip 2 selected
//            {
//                L_ item0 = SYS_ project_first->project_next->project_next->project_next->project_next->chip_first->chip_next->file_first->file_name;
//                L_ item1 = SYS_ project_first->project_next->project_next->project_next->project_next->chip_first->chip_next->file_first->file_next->file_name;
//                L_ item2 = NULL;
//                L_ item3 = NULL;
//                L_ item4 = NULL;
//            }
            break; 

        case exe_screen:
            break; 

        default:
            break; 
    }

    rerender_list(itemHighlighted,screenStack);

    if(L_ boxPresent == false)
    {
        draw_selection_box();
    }

    if(L_ headerPresent == false)
    {
        draw_header();
    }
}