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
#include "twi.h"
#include "ble.h"

#define SSD1306Addr 0b0111100

APP_TIMER_DEF(ad_timer_id);
static bool bt_sym_present = false;
static uint16_t oled_ad_count = 0;
static uint32_t ad_seconds; 


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
    nrf_gpio_cfg_output(FET_PIN); //not used 
    nrf_gpio_cfg_output(iRST); //not used

    //atmel
    nrf_gpio_cfg_output(ATMEL_RESET_PIN);
    nrf_gpio_pin_set(ATMEL_RESET_PIN);

    nrf_gpio_cfg_output(BOOT_PIN); //set true before reset to boot atmel
    nrf_gpio_pin_clear(BOOT_PIN);

    nrf_gpio_cfg_input(I2CS_INT, NRF_GPIO_PIN_PULLDOWN);

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

    SSD1351_init();
    
    clear_screen();
    
    //oled_draw_logo();
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

void oled_draw_err(uint8_t err_id)
{
    clear_screen();
    SSD1351_set_cursor(33,50);
    SSD1351_printf(COLOR_RED, med_font, "ERROR:");//draw error
    SSD1351_set_cursor(5,70);
    switch(err_id)
    {
        case error_no_target:
            SSD1351_printf(COLOR_WHITE, small_font, "Target Not Found");
            break;
        case error_no_dbg_pwr:
            SSD1351_printf(COLOR_WHITE, small_font, "Failure To Init");
            break;
        case error_dbg_locked:
            SSD1351_printf(COLOR_WHITE, small_font, "Debug Port Locked");
            break;
        default:
            break;
    }

    SSD1351_update();
    nrf_delay_ms(2000);
}

void oled_stop_ad_timer(void)
{
    uint32_t ret = app_timer_stop(ad_timer_id);
    APP_ERROR_CHECK(ret);
}

void oled_ad_callback(void* p_context)
{
    if(bt_sym_present)
    {
        ble_draw_icon(COLOR_BLACK);//erase
        bt_sym_present = false;
    }
    else
    {
        ble_draw_icon(COLOR_BLUE);//draw
        bt_sym_present = true;
    }

    oled_ad_count++; 
    if(oled_ad_count == ad_seconds)
    {
        oled_stop_ad_timer(); 
        oled_ad_count = 0;
    }

}
void oled_advertising_indicate(uint32_t ad_duration)
{
 
    ad_seconds = ((ad_duration*10)/1000);
    uint32_t ret = app_timer_create(&ad_timer_id, APP_TIMER_MODE_REPEATED, oled_ad_callback);
    APP_ERROR_CHECK(ret); 
    ret = app_timer_start(ad_timer_id, APP_TIMER_TICKS(1000), NULL); //fire every half second
    APP_ERROR_CHECK(ret); 

}

void oled_draw_progress_bar(void)
{
    uint16_t width = 124;
    uint16_t height = 15;
    uint16_t y_point = 63;
    uint16_t x_point = 2;

    clear_screen();
    SSD1351_set_cursor(28,50);
    SSD1351_printf(COLOR_WHITE, small_font, "Flashing...");
    SSD1351_draw_rect(x_point, y_point, width, height, COLOR_WHITE);
    SSD1351_update();
    
    //read programming status / error status from atmel via twi
    uint8_t prog_status;
    uint8_t rx_buf[3];
    while(prog_status != prog_complete && prog_status != prog_error) // 124 = prog_complete
    {
        twi_cmd_rx(rx_buf, 3);
        if(rx_buf[0] == start_byte)
        {
            if(rx_buf[1] == error_cmd)
            {
                oled_draw_err(rx_buf[2]);
                atmel_shutdown(); 
                prog_status = prog_error; // exit loop
            }
            else if(rx_buf[1] == progress_cmd)
            {
                prog_status = rx_buf[2]; //logic broken unless every num from 1-124 is sent
                SSD1351_draw_line(prog_status + x_point + 1, y_point + 1, prog_status + x_point + 1, y_point + height - 1, COLOR_GREEN);
                SSD1351_update();
                
             
                if(rx_buf[2] == prog_complete)
                {
                    prog_status = prog_complete; // exit loop
                }
             
            }
        }
    }
    
    /*
    for(int i = 0; i < 124; i++)
    {
        SSD1351_draw_line(i + x_point + 1, y_point + 1, i + x_point + 1, y_point + height - 1, COLOR_GREEN);
        SSD1351_update();
    }
    */

    clear_screen();
}