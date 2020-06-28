
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
#include "app_button.h"
#include "nrf_gpio.h"
#include "button.h" 
#include "ssd1351.h" 
#include "app_timer.h" 
#include "app_error.h" 
#include "system.h" 
#include "flash.h" 
#include "twi.h"
#include "ble.h"

APP_TIMER_DEF(long_press_timer_id);

static project_struct* project_selected; 
static chip_struct* chip_selected; 

//NOTE do any of these need to be volatile? 
volatile bool enterFlag = false; 
volatile bool upFlag = false;
volatile bool downFlag = false;
volatile bool longTimerStarted = false;
volatile bool rerender = false; //TODO: replace rerender flag with alt logic
volatile bool recentsFlag = false;

static int8_t itemHighlighted = 0;
static int8_t selectedItem = 0; 
static int8_t screenStack = 0; 



void reduce_itemHighlighted(void)
{
    itemHighlighted--; 
}


void button_up_callback(uint8_t pin_no, uint8_t button_action)
{  
    if(button_action == APP_BUTTON_PUSH)
    { 
        upFlag = true;
        timer_start();
    
        itemHighlighted--;
        rerender = true;
        if(itemHighlighted < 0)
        {
            itemHighlighted = 0;
            rerender = false; 
//            screenStack--;      //NOTE: for more granular backwards navigation, needs bug fixes with display logic
//            if(screenStack < 0)
//            {
//                screenStack = 0; 
//            }
        }
        if(rerender)
        {
            rerender_list(itemHighlighted);
        }
    }
    if(button_action == APP_BUTTON_RELEASE)
    {
        upFlag = false;
        timer_stop();
    }
}


void button_down_callback(uint8_t pin_no, uint8_t button_action) //TODO: long press timer 
{
    if(button_action == APP_BUTTON_PUSH)
    {   
        downFlag = true; 
        timer_start();

        itemHighlighted++;
        rerender = true; 
        if(itemHighlighted > MAX_ITEMS) 
        {
            itemHighlighted = MAX_ITEMS;
            rerender = false;
        }
        if(rerender) // rerender if flag set 
        {
            rerender_list(itemHighlighted);
        }
    }
    if(button_action == APP_BUTTON_RELEASE)
    {
        downFlag = false;
        timer_stop();
    }
}


void enter_callback(uint8_t pin_no, uint8_t button_action)
{   
    if(button_action == APP_BUTTON_PUSH)
    {   
        enterFlag = true;
        timer_start();

        selectedItem = itemHighlighted; 
       
        screenStack++;
        switch(screenStack)
        {   
            case splash_screen: //never reached
                break; 
            case project_screen:
                if(!selectedItem && recents_check()) // render recents
                {
                    recentsFlag = true; 
                    rerender = true;
                }
                else if(selectedItem) // render projects
                {   
                    recentsFlag = false;
                    rerender = true;
                }
                else
                {
                    screenStack--; //do not progress 
                    rerender = false;
                }
                break;
            case chip_screen:
                if(recentsFlag)//NOTE file selection screen of recents branch
                {
                    //TODO: execute programming for selected file from recents, use global recents_struct
                    //TODO: index recents_struct based on selectedItem and save file to flash
                    screenStack = 0;
                    rerender = true;
                    recentsFlag = false;
                    list_clear();
                }
                else 
                {
                    if(project_list_index(selectedItem) != NULL) // if project exists
                    {
                        project_selected = chips_sync(selectedItem);  // load chips of selected project 
                        rerender = true; 
                        list_clear();
                    }
                    else
                    {
                        screenStack--; //do not progress 
                    }
                }
                break;
            case file_screen: 
                if(chip_list_index(selectedItem, project_selected) != NULL) // if chip exists 
                {
                    chip_selected = files_sync(selectedItem, project_selected); 
                    rerender = true;
                    list_clear();
                }
                else
                {
                    screenStack--; //do not progress 
                }
                break;
            case exe_screen:
                if(file_list_index(selectedItem, chip_selected) != NULL)
                {
                                // execute programming //
                    // 1.) use selectedItem name to seek for file header in flash
                    // 2.) deinit qspi and its pins 
                    // 3.) send file data address and data length, target flash address, and chip type
                    // 4.) enter into progress bar write mode 
                    // 5.) receive progress updates from atmel over i2c until success or fail 
                    // 6.) reinit qspi 

                    file_struct* target_file = file_list_index(selectedItem, chip_selected);
                    uint32_t file_data_addr = target_file->file_data;
                    uint32_t file_data_len = target_file->data_length; 
                    uint32_t file_start_addr = target_file->start_addr; 
                    uint32_t chip_target_id = target_file->chip_parent->chip_type_id;
                    
                    if(!string_compare(target_file->file_name, "Empty", 5)) //begin programming process
                    {
                        qspi_deinit();
                    
                        uint8_t data_buff[16];                        //TODO Extract these operations to function 
                        data_buff[0] = (file_data_addr >> 24) & 0xFF; //bit shift 32bit address into 8bit array 
                        data_buff[1] = (file_data_addr >> 16) & 0xFF;
                        data_buff[2] = (file_data_addr >> 8) & 0xFF;
                        data_buff[3] = file_data_addr & 0xFF;

                        data_buff[4] = (file_data_len >> 24) & 0xFF;  
                        data_buff[5] = (file_data_len >> 16) & 0xFF;
                        data_buff[6] = (file_data_len >> 8) & 0xFF;
                        data_buff[7] = file_data_len & 0xFF;

                        data_buff[8] = (file_start_addr >> 24) & 0xFF;  
                        data_buff[9] = (file_start_addr >> 16) & 0xFF;
                        data_buff[10] = (file_start_addr >> 8) & 0xFF;
                        data_buff[11] = file_start_addr & 0xFF;

                        data_buff[12] = (chip_target_id >> 24) & 0xFF; 
                        data_buff[13] = (chip_target_id >> 16) & 0xFF;
                        data_buff[14] = (chip_target_id >> 8) & 0xFF;
                        data_buff[15] = chip_target_id & 0xFF;

                        twi_cmd_tx(target_cmd, data_buff, 16);

                        oled_draw_progress_bar(); //enter progress bar screen 
                    
                        //qspi_init(); //reinit and deinit atmel qspi

                    }

                    push_file_to_recents(target_file); //add file to recents, push last off stack
                    screenStack = 0;
                    rerender = true;
                    list_clear();
                }
                else
                {
                    screenStack--; //do not progress 
                }
                break;
            default: 
                break; 
        }
        if(rerender)
        { 
            itemHighlighted = 0; // reset to first item in list
            rerender_screen(itemHighlighted, selectedItem, screenStack, recentsFlag);
            rerender = false; 
        }
    }
    if(button_action == APP_BUTTON_RELEASE)
    {
        enterFlag = false;
        timer_stop();
    }
}


static app_button_cfg_t btn_config[] =
{
    {BTN_UP, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_up_callback}, // up
    {BTN_DOWN, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_down_callback}, // down
    {BTN_ENTER, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, enter_callback}
};


void button_init(void)
{
    timer_init();

    uint32_t err_code; 
    err_code = app_button_init(btn_config, BUTTON_COUNT, BUTTON_DEBOUNCE_DELAY);
    APP_ERROR_CHECK(err_code);
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}


void long_press_timeout_handler(void* p_context)
{
    if(upFlag && !enterFlag)// up long press
    {   
        recentsFlag = false;
        list_clear();
        screenStack = 0; 
        rerender_screen(itemHighlighted, selectedItem, screenStack, recentsFlag);

    }
    else if(enterFlag && !upFlag)// enter long press
    {
        clear_leds(); // for test
        // nrf_gpio_pin_set(LED_ORANGE);
    }
    else if(enterFlag && upFlag) //if dual press
    {
        atmel_reset();
        hibernate();
    }
    else if(downFlag) //down long press
    {
        clear_leds(); // for test 
        // nrf_gpio_pin_set(LED_BLUE);
    }
}


void timer_init(void)
{
    uint32_t ret = app_timer_create(&long_press_timer_id, APP_TIMER_MODE_SINGLE_SHOT, long_press_timeout_handler);
    APP_ERROR_CHECK(ret); 
}


void timer_start(void)
{
    if(!longTimerStarted)
    {
        uint32_t ret = app_timer_start(long_press_timer_id, APP_TIMER_TICKS(LONG_PRESS_THRESHOLD), NULL); 
        APP_ERROR_CHECK(ret); 
    }

    longTimerStarted = true; 
}


void timer_stop(void)
{
    longTimerStarted = false;
    uint32_t ret = app_timer_stop(long_press_timer_id);
    APP_ERROR_CHECK(ret); 
}
