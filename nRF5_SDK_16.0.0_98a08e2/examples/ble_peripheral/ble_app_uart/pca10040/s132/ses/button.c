
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


volatile int8_t itemHighlighted = 0; 


void button_up_callback(uint8_t pin_no, uint8_t button_action) // TODO: minimize work button interrupts do 
{
    if (button_action == APP_BUTTON_PUSH) //or use APP_BUTTON_RELEASE
    {
        itemHighlighted--; // no wrap around 
        if(itemHighlighted < 0)
        {
            itemHighlighted = 0;
        }
        rerender_screen(itemHighlighted);
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
        rerender_screen(itemHighlighted);
    }
}


static app_button_cfg_t btn_config[] =
{
    {BTN_UP, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_up_callback}, // up
    {BTN_DOWN, APP_BUTTON_ACTIVE_LOW, NRF_GPIO_PIN_PULLUP, button_down_callback} // down
};


void button_init(void)
{
    nrf_gpio_cfg_input(BTN_UP, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_DOWN, NRF_GPIO_PIN_PULLUP);

    uint32_t err_code; 
    err_code = app_button_init(btn_config, BUTTON_COUNT, BUTTON_DEBOUNCE_DELAY);
    APP_ERROR_CHECK(err_code);
    err_code = app_button_enable();
    APP_ERROR_CHECK(err_code);
}