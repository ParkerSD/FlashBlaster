
// OLED96
// Library for accessing the 0.96" SSD1306 128x64 OLED display
// Written by Larry Bank (bitbank@pobox.com)
// Copyright (c) 2017 BitBank Software, Inc.
// Project started 1/15/2017
//

#ifndef OLED_H_
#define OLED_H_


// OLED type for init function
enum {
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


typedef struct list list_t; 
typedef struct list 
{
    char* header;
    char* item0;
    char* item1;
    char* item2;
    bool boxPresent;
    bool headerPresent;
} list_t;





// Initialize the OLED96 library for a specific I2C address
// Optionally enable inverted or flipped mode
// returns 0 for success, 1 for failure
//

void twi_init(void);

void oled_init(void); 

void oled_test(void);

int oledInit(int iChannel, int iAddress, int iType, int bFlip, int bInvert);

// Turns off the display and closes the I2C handle
void oledShutdown(void);

// Fills the display with the byte pattern
int clear_display(unsigned char ucPattern);

// Write a text string to the display at x (column 0-127) and y (row 0-7)
// bLarge = 0 - 8x8 font, bLarge = 1 - 16x24 font
int oledWriteString(int x, int y, char *szText, int bLarge);

// Sets a pixel to On (1) or Off (0)
// Coordinate system is pixels, not text rows (0-127, 0-63)
int oledSetPixel(int x, int y, unsigned char ucPixel);

// Sets the contrast (brightness) level of the display
// Valid values are 0-255 where 0=off and 255=max brightness
int oledSetContrast(unsigned char ucContrast);


void draw_box(int y);

void draw_text(int y, char* text); //0 < y < 7 

static list_t* new_list(void);

void init_list(void);

void clear_list(void);

void draw_screen(void);

void draw_initial_screen(void);

void rerender_screen(int8_t, uint8_t);

void rerender_list(int8_t);


#endif /* OLED96_H_ */