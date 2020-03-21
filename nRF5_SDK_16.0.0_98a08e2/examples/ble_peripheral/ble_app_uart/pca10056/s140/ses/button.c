
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

APP_TIMER_DEF(long_press_timer_id);

volatile bool enterFlag = false;
volatile bool upFlag = false;
volatile bool longTimerStarted = false;

int8_t itemHighlighted = 0;
int8_t selectedItem = 0; 
uint8_t screenStack = 0; 

void button_up_callback(uint8_t pin_no, uint8_t button_action)
{  
    if(button_action == APP_BUTTON_PUSH)
    { 
        upFlag = true;
        timer_start();
    
        itemHighlighted--;
        if(itemHighlighted < 0)
        {
            itemHighlighted = 0;
        }
        rerender_list(itemHighlighted, screenStack);
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
        itemHighlighted++;
        if(itemHighlighted > 5)
        {
            itemHighlighted = 5;
        }
        rerender_list(itemHighlighted, screenStack);
    }
}


void enter_callback(uint8_t pin_no, uint8_t button_action)
{   
    if(button_action == APP_BUTTON_PUSH)
    { 
        enterFlag = true;
        timer_start();

        screenStack++;
        selectedItem = itemHighlighted; 
        itemHighlighted = 0;
        
        if(screenStack > 2)
        {   
           //execute SWD programming on itemSelected
          screenStack = 0; //reset to home screen?
      
          push_file_to_recents(); //add file to recents, push last off stack
        }

        clear_screen();
        rerender_screen(itemHighlighted, selectedItem, screenStack);
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
        clear_leds();
        nrf_gpio_pin_set(LED_BLUE);
    }
    else if(enterFlag && !upFlag)// enter long press
    {
        clear_leds();
        nrf_gpio_pin_set(LED_GREEN);
    }
    else if(enterFlag && upFlag) //if dual press
    {
        hibernate();
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
