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
#include "nrf_drv_spi.h"
#include "oled.h"
#include "ssd1351.h" 
#include "fonts.h" 
#include "button.h" 
#include "system.h"

#define SSD1306Addr 0b0111100


extern unsigned char ucFont[], ucSmallFont[];
static int iScreenOffset; // current write offset of screen data
static unsigned char ucScreen[1024]; // local copy of the image buffer
static int oled_type, oled_flip;


void clear_leds(void)
{
    //nrf_gpio_pin_clear(LED_BLUE);
    nrf_gpio_pin_clear(LED_RED);
    nrf_gpio_pin_clear(LED_ORANGE);
}

void gpio_init(void) // init gpio for oled drivers 
{
    //nrf_gpio_cfg_output(CS_PIN); //now defined as SS_PIN in SPIO Init
    nrf_gpio_cfg_output(RST_PIN); 
    nrf_gpio_cfg_output(DC_PIN); 
    nrf_gpio_cfg_output(FET_PIN); 
    nrf_gpio_cfg_output(iRST);

    //atmel
    nrf_gpio_cfg_output(ATMEL_RESET); 
    nrf_gpio_pin_set(ATMEL_RESET);

    //nrf_gpio_cfg_output(LED_BLUE); 
    nrf_gpio_cfg_output(LED_RED); 
    nrf_gpio_cfg_output(LED_ORANGE);
    clear_leds();

    nrf_gpio_cfg_input(BTN_UP, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_DOWN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_ENTER, NRF_GPIO_PIN_PULLUP);

    nrf_gpio_cfg_input(BB_EN, NRF_GPIO_PIN_NOPULL);
    nrf_gpio_cfg_input(LDO_EN, NRF_GPIO_PIN_NOPULL);
}


static void oledWriteCommand(unsigned char);

void oled_init(void)
{
    nrf_gpio_pin_set(FET_PIN); 
    nrf_delay_ms(10); 

    SSD1351_init();
    
    clear_screen();

    oled_draw_logo();
   
}


void oled_draw_logo(void)
{   

    clear_screen();
    
    SSD1351_set_cursor(5,64);
    SSD1351_printf(COLOR_BLUE, big_font, "BLASTER");
    SSD1351_update();

    nrf_delay_ms(500);

    for(int x = 0; x < 6; x++)
    {
      SSD1351_set_cursor(21,44);
      SSD1351_printf(COLOR_BLACK, big_font, "FLASH");
      SSD1351_update();
      // nrf_delay_ms(x);
      SSD1351_set_cursor(21,44);
      SSD1351_printf(COLOR_YELLOW, big_font, "FLASH");
      SSD1351_update();
      // nrf_delay_ms(x);
    }

    SSD1351_write_command(SSD1351_CMD_INVERTDISPLAY);
    SSD1351_update();
    SSD1351_write_command(SSD1351_CMD_NORMALDISPLAY);
//    SSD1351_set_cursor(12,115);
//    SSD1351_printf(COLOR_WHITE, small_font, "By Parker Davis");
//    SSD1351_update();

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



void draw_text(int y, char* text) // 0 < y < 8
{   
//    oledWriteString(1, y, text, FONT_SMALL);
}

