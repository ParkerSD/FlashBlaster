#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_timer.h"
#include "app_util_platform.h"
#include "nrf_drv_spi.h"
#include "oled.h"
#include "ssd1351.h" 
#include "fonts.h" 


#define SSD1306Addr 0b0111100

APP_TIMER_DEF(ad_timer_id);
static bool bt_sym_present = false;
static uint16_t oled_ad_count = 0;
static uint32_t ad_seconds; 


extern unsigned char ucFont[], ucSmallFont[];
static int iScreenOffset; // current write offset of screen data
static unsigned char ucScreen[1024]; // local copy of the image buffer
static int oled_type, oled_flip;




void gpio_init(void) // init gpio for oled drivers 
{
    //nrf_gpio_cfg_output(CS_PIN); //now defined as SS_PIN in SPIO Init
    nrf_gpio_cfg_output(RST_PIN); 
    nrf_gpio_cfg_output(DC_PIN); 
    nrf_gpio_cfg_output(FET_PIN); //not used, removed 
    nrf_gpio_cfg_output(iRST); //not used, moved to atmel

    //atmel
    nrf_gpio_cfg_output(ATMEL_RESET_PIN);
    nrf_gpio_pin_set(ATMEL_RESET_PIN);

    nrf_gpio_cfg_output(BOOT_PIN); //set true before reset to boot atmel
    //nrf_gpio_pin_clear(BOOT_PIN);

    nrf_gpio_cfg_input(I2CS_INT, NRF_GPIO_PIN_PULLDOWN);

    nrf_gpio_cfg_input(BTN_UP, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_DOWN, NRF_GPIO_PIN_PULLUP);
    nrf_gpio_cfg_input(BTN_ENTER, NRF_GPIO_PIN_PULLUP);

    nrf_gpio_cfg_input(BB_EN, NRF_GPIO_PIN_NOPULL); //NOTE not used 
    nrf_gpio_cfg_input(LDO_EN, NRF_GPIO_PIN_NOPULL);

}

void clear_screen(void)
{
    SSD1351_fill(COLOR_BLACK);
    SSD1351_update();
}

static void oledWriteCommand(unsigned char);

void oled_init(void)
{

    SSD1351_init();
    
    clear_screen();
    
    //oled_draw_logo();
}

void oled_draw_bootloader(void)
{
    oled_center_x(strlen("USB"), 40, MED_CHAR_WIDTH);
    SSD1351_printf(COLOR_WHITE, med_font, "USB");
    oled_center_x(strlen("BOOTLOADER"), 60, MED_CHAR_WIDTH);
    SSD1351_printf(COLOR_BLUE, med_font, "BOOTLOADER");
    oled_center_x(strlen("ACTIVE"), 80, MED_CHAR_WIDTH);
    SSD1351_printf(COLOR_BLUE, med_font, "ACTIVE");
    oled_center_x(strlen("Via NRFConnect"), 115, SM_CHAR_WIDTH);
    SSD1351_printf(COLOR_WHITE, small_font, "Use NRFConnect");
    SSD1351_update();
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

void oled_draw_transfer_progress(void)
{
    clear_screen();
    SSD1351_set_cursor(5,60);
    SSD1351_printf(COLOR_YELLOW, med_font, "Transfering");//draw error
}

void oled_draw_transfer_complete(void)
{
    clear_screen();
    SSD1351_set_cursor(20,50);
    SSD1351_printf(COLOR_GREEN, med_font, "Transfer");//draw error
    SSD1351_set_cursor(20,70);
    SSD1351_printf(COLOR_GREEN, med_font, "Complete");//draw error
}


void oled_center_x(uint8_t num_chars, uint8_t y_point, uint8_t char_width)
{
    //x-axis 128 pixels wide, small char is 7 pixels wide, med 11, big 16 
    uint8_t x_point;
    uint8_t string_width = num_chars * char_width; 
    x_point = (SCREEN_WIDTH - string_width)/2; 

    SSD1351_set_cursor(x_point, y_point);
}

/*
void oled_draw_err(uint8_t err_id)
{
    clear_screen();
    SSD1351_set_cursor(33,50);
    SSD1351_printf(COLOR_RED, med_font, "ERROR:");//draw error
    
    switch(err_id)
    {
        case ERROR_NO_TARGET:
            oled_center_x(strlen("Target Not Found"), 70, SM_CHAR_WIDTH);
            SSD1351_printf(COLOR_WHITE, small_font, "Target Not Found");
            break;
        case ERROR_NO_DBG_PWR:
            oled_center_x(strlen("Failure To Init"), 70, SM_CHAR_WIDTH);
            SSD1351_printf(COLOR_WHITE, small_font, "Failure To Init");
            break;
        case ERROR_DBG_LOCKED:
            oled_center_x(strlen("Debug Port Locked"), 70, SM_CHAR_WIDTH);
            SSD1351_printf(COLOR_WHITE, small_font, "Debug Port Locked");
            break;
        case ERROR_CLIENT_TIMEOUT: // PC(central) is client
            oled_center_x(strlen("Client Timeout"), 70, SM_CHAR_WIDTH);
            SSD1351_printf(COLOR_WHITE, small_font, "Client Timeout");
            break;
        case ERROR_SERVER_TIMEOUT: // Flashblaster(peripheral) is server
            oled_center_x(strlen("Server Timeout"), 70, SM_CHAR_WIDTH);
            SSD1351_printf(COLOR_WHITE, small_font, "Server Timeout");
            break;
        case PROJECT_LIMIT_REACHED:
            oled_center_x(strlen("8 Project Limit"), 70, SM_CHAR_WIDTH);
            SSD1351_printf(COLOR_WHITE, small_font, "8 Project Limit");
            break;
        case CHIP_LIMIT_REACHED:
            oled_center_x(strlen("8 Chip Limit"), 70, SM_CHAR_WIDTH);
            SSD1351_printf(COLOR_WHITE, small_font, "8 Chip Limit");
            break;
        case FILE_LIMIT_REACHED:
            oled_center_x(strlen("8 File Limit"), 70, SM_CHAR_WIDTH);
            SSD1351_printf(COLOR_WHITE, small_font, "8 File Limit");
            break;
        default:
            break;
    }

    SSD1351_update();
    nrf_delay_ms(2000);
    clear_screen();
    return_home();
}
*/





