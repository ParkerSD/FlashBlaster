
// OLED96
// Library for accessing the 0.96" SSD1306 128x64 OLED display
// Written by Larry Bank (bitbank@pobox.com)
// Copyright (c) 2017 BitBank Software, Inc.
// Project started 1/15/2017
//

#ifndef OLED_H_
#define OLED_H_

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 128
#define SM_CHAR_WIDTH 7

//#define CS_PIN 38 //P1.06
#define LDO_EN 4  //NOTE used for detecting usb on boot
#define BB_EN 26 //NOTE not used 
#define RST_PIN 36 //P1.04
#define DC_PIN 37 //P1.05
#define iRST 11 //not used, moved to atmel
#define FET_PIN 33 //not used

//atmel pins
#define ATMEL_RESET_PIN 30 
#define BOOT_PIN 45 //hold high before reset to boot atmel 
#define I2CS_INT 43 // indication that atmel has i2c slave tx data waiting for read

//#define LED_BLUE 27 
#define LED_RED 42 //P1.10
#define LED_ORANGE 43 //P1.11

#define SWCLK 16 

// OLED type for init function
enum 
{
	OLED_128x32 = 1,
	OLED_128x64,
	OLED_132x64,
	OLED_64x32
};

typedef enum
{
	FONT_NORMAL=0,	// 8x8
	FONT_BIG,		// 16x24
	FONT_SMALL		// 6x8
} FONTSIZE;




// Initialize the OLED96 library for a specific I2C address
// Optionally enable inverted or flipped mode
// returns 0 for success, 1 for failure
//

void gpio_init(void);

void oled_init(void); 

void oled_draw_target(uint16_t start_radius, uint16_t start_color);

void oled_shoot_holes(uint8_t radius);

void oled_draw_logo(void);

void clear_leds(void);

void draw_text(int y, char* text); //0 < y < 7 

void oled_draw_progress_bar(void);

void oled_draw_err(uint8_t err_id);

void oled_advertising_indicate(uint32_t ad_duration);

void oled_stop_ad_timer(void);

void oled_draw_transfer_progress(void);

void oled_draw_transfer_complete(void);

void oled_center_small_x(uint8_t num_chars, uint8_t y_point);









#endif /* OLED96_H_ */