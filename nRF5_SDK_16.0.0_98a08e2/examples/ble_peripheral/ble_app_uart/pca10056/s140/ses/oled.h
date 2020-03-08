
// OLED96
// Library for accessing the 0.96" SSD1306 128x64 OLED display
// Written by Larry Bank (bitbank@pobox.com)
// Copyright (c) 2017 BitBank Software, Inc.
// Project started 1/15/2017
//

#ifndef OLED_H_
#define OLED_H_

//#define CS_PIN 38 //P1.06
#define RST_PIN 36 //P1.04
#define DC_PIN 37 //P1.05
#define FET_PIN 33 //P1.05

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

typedef enum
{
    project = 0,
    chip,
    file
} list_type;

typedef struct list list_struct;    //data that is actually displayed on oled, MAX 10 items, increase in future
typedef struct list 
{   
    list_type currentList; 
    char* header;
    char* item0;
    char* item1;
    char* item2;
    char* item3;
    char* item4;
    bool boxPresent;
    bool headerPresent;
} list_struct;

typedef struct file file_struct;
typedef struct file 
{
    char* fileName; 
    uint8_t* filePtr; // pointer to file 

}file_struct; 

typedef struct chip chip_struct;
typedef struct chip 
{   
    char* chipName; 
    file_struct* file1; // pointer to file 
    file_struct* file2;

}chip_struct; 

typedef struct project project_struct;
typedef struct project
{
    char* projectName; 
    chip_struct* chip1;
    chip_struct* chip2;
     
}project_struct;

typedef struct system system_struct;
typedef struct system
{
    char* systemName; 
    project_struct* project1;
    project_struct* project2;
    project_struct* project3;
    project_struct* project4;
    project_struct* project5;

}system_struct; 


// Initialize the OLED96 library for a specific I2C address
// Optionally enable inverted or flipped mode
// returns 0 for success, 1 for failure
//

void gpio_init(void);

void twi_init(void);

void oled_init(void); 

void oled_fill_black(void);

void oled_draw_target(uint16_t start_radius, uint16_t start_color);

void oled_shoot_holes(uint8_t radius);

void oled_draw_logo(void);



// OLD //

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

list_struct* list_new(void);

void list_init(void);

void clear_list(void);

void draw_initial_screen(void);

void rerender_screen(int8_t, int8_t , uint8_t);

void rerender_list(int8_t);

system_struct* system_new(void);

void system_init(void);

char* firmware_version_fetch(void);
char* project_name_fetch(void);
char* chip_name_fetch(void);
char* file_name_fetch(void);

file_struct* file_new(void);
chip_struct* chip_new(void);
project_struct* project_new(void);


#endif /* OLED96_H_ */