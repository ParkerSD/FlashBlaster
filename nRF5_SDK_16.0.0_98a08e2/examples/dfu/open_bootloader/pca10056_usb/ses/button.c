
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_util_platform.h"
#include "oled.h"
#include "app_button.h"
#include "nrf_gpio.h"
#include "button.h" 
#include "ssd1351.h" 
#include "app_timer.h" 
#include "app_error.h" 


APP_TIMER_DEF(long_press_timer_id);
 
static bool enterFlag = false; 
static bool upFlag = false;
static bool downFlag = false;
static bool longTimerStarted = false;


void hibernate(void)
{
    nrf_gpio_pin_clear(RST_PIN); // turn off display
    nrf_delay_ms(1000); // delay to avoid reboot after turn off
    nrf_gpio_cfg_sense_input(BTN_ENTER, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    //NRF_WDT->TASKS_START = 0; // deactivate watchdog if needed
    NRF_POWER->SYSTEMOFF = 1;
}

void device_shutdown(void)
{
    nrf_gpio_pin_clear(BOOT_PIN); // turn of atmel
    hibernate();
}

void button_up_callback(uint8_t pin_no, uint8_t button_action)
{  
    if(button_action == APP_BUTTON_PUSH)
    { 
        upFlag = true;
        timer_start();
    }
    if(button_action == APP_BUTTON_RELEASE)
    {
        upFlag = false;
        timer_stop();
    }
}


void button_down_callback(uint8_t pin_no, uint8_t button_action) 
{
    if(button_action == APP_BUTTON_PUSH)
    {   
        downFlag = true; 
        timer_start();
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
//        recentsFlag = false;
//        list_clear();
//        return_home(); 
    }
    else if(enterFlag && !upFlag)// enter long press
    {
        device_shutdown();
    }
    else if(enterFlag && upFlag) //if dual press
    {
        //advertising_start();
    }
    else if(downFlag) //down long press
    {
        // do something
        // oled_draw_transfer_progress();
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
