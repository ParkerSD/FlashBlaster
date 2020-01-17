
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

#define OFF 1
#define ON 0 

volatile int8_t itemHighlighted = 0; 
volatile uint8_t screenStack = 0; 

//volatile uint8_t buttonUp;

//volatile uint8_t buttonDown; 

//volatile uint8_t state; 

/*
void get_button_state(void)
{
    if(nrf_gpio_pin_read(BTN_UP)) // high level is logic false
    {
        buttonUp = 0; // ON
    }
    else 
    {
        buttonUp = 1; //OFF
    }
    if(nrf_gpio_pin_read(BTN_DOWN))
    {
        buttonDown = 0; 
    }
    else 
    {
        buttonDown = 1; 
    }

    state = (buttonUp << 1) | buttonDown; 
}
*/

/*        // too slow 
void encoder_callback(uint8_t pin_no, uint8_t button_action) // TODO: minimize work button interrupts do
{
    get_button_state();

    if (pin_no == BTN_UP && button_action == APP_BUTTON_PUSH) // low level 
    {
        buttonUp = OFF; // 1 = off, 0 = on 
        if(buttonDown == ON) //&& state == 3)
        {
            itemHighlighted--;
        }
    }

    else if (pin_no == BTN_UP && button_action == APP_BUTTON_RELEASE) // high level
    {
        buttonUp = ON;
        if(buttonDown == OFF) //&& state == 0)
        {
            itemHighlighted--;
        }
    }

    else if (pin_no == BTN_DOWN && button_action == APP_BUTTON_PUSH) 
    {
        buttonDown == OFF;
        if(buttonUp == ON) //&& state == 3)
        {
            itemHighlighted++;
        }
    }
    
    else if (pin_no == BTN_DOWN && button_action == APP_BUTTON_RELEASE) 
    {
        buttonDown == ON;
        if(buttonUp == OFF) //&& state == 0)
        {
            itemHighlighted++;
        }
    }
    
    if(itemHighlighted < 0)
    {
        itemHighlighted = 0;
    }

    else if(itemHighlighted > 2)
    {
        itemHighlighted = 2;
    }

    rerender_screen(itemHighlighted);
}
*/

void button_up_callback(uint8_t pin_no, uint8_t button_action)
{
    if(button_action == APP_BUTTON_PUSH)
    {
        itemHighlighted--;
        if(itemHighlighted < 0)
        {
            itemHighlighted = 0;
        }
        rerender_screen(itemHighlighted, screenStack);
    }
}

void button_down_callback(uint8_t pin_no, uint8_t button_action)
{
    if(button_action == APP_BUTTON_PUSH)
    {
        itemHighlighted++;
        if(itemHighlighted > 2)
        {
            itemHighlighted = 2;
        }
        rerender_screen(itemHighlighted, screenStack);
    }
}

void enter_callback(uint8_t pin_no, uint8_t button_action)
{
    if(button_action == APP_BUTTON_PUSH)
    {
        screenStack++;
        itemHighlighted = 0;

        if(screenStack > 2)
        {   
          //execute SWD programming on itemSelected
          screenStack = 0; //reset to home screen?
        }

        clear_display(0);
        rerender_screen(itemHighlighted, screenStack);
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
    nrf_gpio_cfg_input(BTN_UP, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_DOWN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_ENTER, NRF_GPIO_PIN_PULLUP);

    uint32_t err_code; 
    err_code = app_button_init(btn_config, BUTTON_COUNT, BUTTON_DEBOUNCE_DELAY);
    APP_ERROR_CHECK(err_code);
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}