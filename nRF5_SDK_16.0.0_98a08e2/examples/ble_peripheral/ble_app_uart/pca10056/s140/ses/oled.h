
// OLED96
// Library for accessing the 0.96" SSD1306 128x64 OLED display
// Written by Larry Bank (bitbank@pobox.com)
// Copyright (c) 2017 BitBank Software, Inc.
// Project started 1/15/2017
//

#ifndef OLED_H_
#define OLED_H_

//#define CS_PIN 38 //P1.06
#define LDO_EN 0  //TODO NOW PIN 4
#define BB_EN 1 //TODO NOW PIN 26 
#define RST_PIN 36 //P1.04
#define DC_PIN 37 //P1.05
#define iRST 11 //P1.01
#define FET_PIN 33 //P1.01

#define LED_BLUE 27 //P1.01
#define LED_RED 26 //P1.01
#define LED_GREEN 30 //P1.01

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

void twi_init(void);

void oled_init(void); 

void oled_draw_target(uint16_t start_radius, uint16_t start_color);

void oled_shoot_holes(uint8_t radius);

void oled_draw_logo(void);

void clear_leds(void);


void draw_text(int y, char* text); //0 < y < 7 










#endif /* OLED96_H_ */