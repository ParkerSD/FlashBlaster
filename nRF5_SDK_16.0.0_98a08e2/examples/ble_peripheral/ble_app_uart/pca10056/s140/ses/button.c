
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


volatile bool enterFlag = false;
volatile bool upFlag = false;
volatile bool dualEnter = false;
volatile bool dualUp = false;


void button_up_callback(uint8_t pin_no, uint8_t button_action)
{   
   upFlag = true;
   if(button_action == APP_BUTTON_PUSH)
   { 
      if(enterFlag && upFlag) //if enter duel press
      {
         if(!dualUp && !dualEnter) // only execute once
         { 
            //TODO: do dual press action (off/on device) 
            // dualUp = dualPress(dualUp);
            SSD1351_draw_filled_circle(64, 64, 50, COLOR_RED);
            SSD1351_update();
         }
         else
         {
            dualUp = false; //reset flag 
         }
      }
      else //button up only press 
      {

      }
   }
   if(button_action == APP_BUTTON_RELEASE)
   {
      upFlag = false;
   }
}

void button_down_callback(uint8_t pin_no, uint8_t button_action)
{
    if(button_action == APP_BUTTON_PUSH)
    {     
//        itemHighlighted++;
//        if(itemHighlighted > 5)
//        {
//            itemHighlighted = 5;
//        }
//        rerender_list(itemHighlighted);
    }
}

void enter_callback(uint8_t pin_no, uint8_t button_action)
{   
   enterFlag = true;
   if(button_action == APP_BUTTON_PUSH)
   { 
      if(enterFlag && upFlag) //if button up duel press
      {
         if(!dualEnter && !dualUp) // only execute once
         {
            //TODO: do dual press action (off/on device) 
            //dualEnter = dualPress(dualEnter);
            SSD1351_draw_filled_circle(64, 64, 50, COLOR_BLUE);
            SSD1351_update();
         }
         else
         {
            dualEnter = false; //reset flag 
         }
//       screenStack++;
//       selectedItem = itemHighlighted; 
//       itemHighlighted = 0;
//
//       if(screenStack > 2)
//       {   
//         //execute SWD programming on itemSelected
//         screenStack = 0; //reset to home screen?
//       }
//
//       clear_display(0);
//       rerender_screen(itemHighlighted, selectedItem, screenStack);
      }
      else //enter only press 
      {

      }
   }
   if(button_action == APP_BUTTON_RELEASE)
   {
      enterFlag = false;
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