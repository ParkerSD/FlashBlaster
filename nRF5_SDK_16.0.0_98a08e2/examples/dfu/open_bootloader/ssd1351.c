
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"

#include "app_timer.h"

#include "nrf_drv_spi.h"
#include "oled.h"
#include "ssd1351.h"

/* Buffer to hold the Display RAM Data */
STATIC DRAM displayRAM;


//#define DRAM_16 displayRAM.halfw
//#define DRAM_8 displayRAM.byte


#define SPI_INSTANCE  0 /**< SPI instance index. */
static const nrf_drv_spi_t spi = NRF_DRV_SPI_INSTANCE(SPI_INSTANCE); 


/*_________________ SPI START________________ */ 

static volatile bool spi_xfer_done;  /**< Flag used to indicate that SPI instance completed the transfer. */


void spi_event_handler(nrf_drv_spi_evt_t const * p_event,
                       void *                    p_context)
{
    spi_xfer_done = true;
}

void spi_init(void)
{
    nrf_drv_spi_config_t spi_config = NRF_DRV_SPI_DEFAULT_CONFIG;
    spi_config.ss_pin   = SPI_SS_PIN;
    spi_config.miso_pin = SPI_MISO_PIN;
    spi_config.mosi_pin = SPI_MOSI_PIN;
    spi_config.sck_pin  = SPI_SCK_PIN;
    APP_ERROR_CHECK(nrf_drv_spi_init(&spi, &spi_config, NULL, NULL)); //arg3 spi_event_handler casuing errors
}

//rudimentary tx / rx fucntions, using global buffers and lengths

void spi_tx (uint8_t *spi_tx_buff, uint32_t length)
{
    // Reset transfer done flag
    spi_xfer_done = false;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, spi_tx_buff, length, NULL, NULL));
}

void spi_rx (uint8_t *spi_rx_buff, uint32_t length)
{
    // Reset transfer done flag
    spi_xfer_done = false;

    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, NULL, NULL, spi_rx_buff, length));
}

void spi_tx_byte (uint8_t data)
{  
 // Reset transfer done flag
    spi_xfer_done = false;
  
    APP_ERROR_CHECK(nrf_drv_spi_transfer(&spi, &data, 1, NULL, NULL));
}

/*____________________ SPI END_____________________ */ 

/* Screen cursor for printing */
struct cursor{
  uint8_t x;
  uint8_t y;
}SSD1351_cursor;

/**
  * @brief  Writes command to the SSD1351 OLED Display
  * @param  cmd: command to send
  * @retval None
  */
void SSD1351_write_command(uint8_t cmd){
  
  nrf_gpio_pin_clear(DC_PIN);
  nrf_delay_ms(1);
  spi_tx_byte (cmd);
}

/**
  * @brief  Writes single byte data to the SSD1351 OLED Display
  * @param  data: data byte to send
  * @retval None
  */
void SSD1351_write_data(uint8_t data){
  nrf_gpio_pin_set(DC_PIN); 
  nrf_delay_ms(1);
  spi_tx_byte (data);
}

/**
  * @brief  Writes a data buffer of bytes to SSD1351 display
  * @param  data: pointer to data buffer to send
  * @param  len: integer with length of buffer to send
  * @retval None
  */
void SSD1351_write_data_buffer(uint8_t *data, uint32_t len){
  nrf_gpio_pin_set(DC_PIN);
  nrf_delay_ms(1);

  if(len > 255)
  {
    uint32_t times = (len/255);
    for(int x = 0; x < times; x++)
    { 
      spi_tx (data + (255 * x), 255);   // chunk up data into 255 byte chunks 
    }
    uint32_t rem = (len % 255);         // transfer remainder
    spi_tx (data, rem);
  }
  else                                  // if buffer is less than 255 bytes transfer all at once
  {
    spi_tx (data, len); 
  }

}

/*
 * @brief Converts from RGB to a single 16bit value
 * @param r: starting x coordinate
 * @para g: starting y coordinate
 * @param b: width of the rectangle
 * @reval 16bit value with the rgb color for display
 */
uint16_t SSD1351_get_rgb(uint8_t r, uint8_t g, uint8_t b){
  uint16_t rgb_color = 0;
  rgb_color |= ((r/8) << 8);
  rgb_color |= ((g/4) >> 3);
  rgb_color |= (((g/4) % 0x07) << 13);
  rgb_color |= ((b/8) << 3);
  return rgb_color;
}

/**
  * @brief  Initializes the SSD1351 OLED Display
  * @retval None
  */
void SSD1351_init(void){
  nrf_gpio_pin_set(RST_PIN);
  nrf_delay_ms(10); //was 200
  nrf_gpio_pin_clear(RST_PIN);
  nrf_delay_ms(10); //was 200
  nrf_gpio_pin_set(RST_PIN);
  nrf_delay_ms(10); //was 200

  SSD1351_write_command(SSD1351_CMD_COMMANDLOCK);
  SSD1351_write_data(0x12);
  SSD1351_write_command(SSD1351_CMD_COMMANDLOCK);
  SSD1351_write_data(0xB1);
  
  SSD1351_write_command(SSD1351_CMD_DISPLAYOFF);
  SSD1351_write_command(SSD1351_CMD_CLOCKDIV);
  SSD1351_write_data(0xF1);
  SSD1351_write_command(SSD1351_CMD_MUXRATIO);
  SSD1351_write_data(127);
  nrf_delay_ms(10); //was 300

  SSD1351_write_command(SSD1351_CMD_SETREMAP);
  SSD1351_write_data(0b00110010);// was x20

  SSD1351_write_command(SSD1351_CMD_SETCOLUMN);
  SSD1351_write_data(0x00);
  SSD1351_write_data(0x7F);

  SSD1351_write_command(SSD1351_CMD_SETROW);
  SSD1351_write_data(0x00);
  SSD1351_write_data(0x7F);

  SSD1351_write_command(SSD1351_CMD_STARTLINE);
  SSD1351_write_data(0x00); // was x00

  SSD1351_write_command(SSD1351_CMD_DISPLAYOFFSET);
  SSD1351_write_data(0x00);

  SSD1351_write_command(SSD1351_CMD_SETGPIO);
  SSD1351_write_data(0x00);

  SSD1351_write_command(SSD1351_CMD_FUNCTIONSELECT);
  SSD1351_write_data(0x01);

  SSD1351_write_command(SSD1351_CMD_PRECHARGE);
  SSD1351_write_data(0x32);

  SSD1351_write_command(SSD1351_CMD_VCOMH);
  SSD1351_write_data(0x05);

  SSD1351_write_command(SSD1351_CMD_NORMALDISPLAY);

  SSD1351_write_command(SSD1351_CMD_CONTRASTABC);
  SSD1351_write_data(0x8A);                          // Color A: Blue
  SSD1351_write_data(0x51);                          // Color B: Green
  SSD1351_write_data(0x8A);                          // Color C: Red

  SSD1351_write_command(SSD1351_CMD_CONTRASTMASTER);
  SSD1351_write_data(0x0F);

  SSD1351_write_command(SSD1351_CMD_SETVSL);
  SSD1351_write_data(0xA0);
  SSD1351_write_data(0xB5);
  SSD1351_write_data(0x55);

  SSD1351_write_command(SSD1351_CMD_PRECHARGE2);
  SSD1351_write_data(0x01);

  SSD1351_write_command(SSD1351_CMD_DISPLAYON);
  nrf_delay_ms(10);
}

/**
  * @brief  Turns off the SSD1351 OLED Display
  * @retval None
  */
void SSD1351_stop(void){
  SSD1351_write_command(SSD1351_CMD_DISPLAYOFF);
}

void SSD1351_start(void){
  SSD1351_write_command(SSD1351_CMD_DISPLAYON);
}
/**
  * @brief  Turns off the SSD1351 OLED Display
  * @param  color: Unsigned int16 containing color code
  * @retval None
  */
void SSD1351_fill(uint16_t color){
  for (int i = 0; i < DRAM_SIZE_16; i++){
    displayRAM.halfw[i] = color;
  }
  //SSD1351_write_command(SSD1351_CMD_STOPSCROLL);
}

/**
  * @brief  Updates the screen RAM
  * @retval None
  */
void SSD1351_update(void){
  
  //nrf_delay_ms(1);
  SSD1351_write_command(SSD1351_CMD_WRITERAM);
  SSD1351_write_data_buffer(displayRAM.byte, DRAM_SIZE_8);
  
}

/**
  * @brief  Updates a specific area within the display
  * @retval None
  */
/*void SSD1351_update_area(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1){
  SSD1351_write_command(SSD1351_CMD_SETCOLUMN);
  SSD1351_write_data(x0);
  SSD1351_write_data(y0);

  SSD1351_write_command(SSD1351_CMD_SETROW);
  SSD1351_write_data(x1);
  SSD1351_write_data(x0);

  int a0 = x0 + (y0 * 128);
  int a1 = x1 + (y1 * 128);

  SSD1351_write_command(SSD1351_CMD_WRITERAM);
  for (int i = a0; i < a1 * 2; i++){
    SSD1351_write_data(DRAM_8[i]);
  }

  // Back to default settings

  SSD1351_write_command(SSD1351_CMD_SETCOLUMN);
  SSD1351_write_data(x0);
  SSD1351_write_data(y0);

  SSD1351_write_command(SSD1351_CMD_SETROW);
  SSD1351_write_data(x1);
  SSD1351_write_data(x0);
}*/

/**
 * @brief Writes a pixel data to the screen RAM buffer
 * @param color: Unsigned int16 containing color code
 * @param x: Pixel's horizontal position
 * @param y: Pixel's vertical position
 * @retval None
 */
void SSD1351_write_pixel(int16_t x, int16_t y, uint16_t color){
  if ( x > 127 || y > 127 || x < 0 || y < 0){
    return;
  }
  int a = (127 - x) + (y * 128);
  displayRAM.halfw[a] = color;
}


/*  LINE DRAWING FUNCTIONS */

STATIC void SSD1351_draw_line_low(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
  int16_t dx = x1 - x0;
  int16_t dy = y1 - y0;
  int16_t yi = 1;
  if (dy < 0){
    yi = -1;
    dy = -dy;
  }
  int16_t D = 2*dy - dx;
  int16_t y = y0;

  if (x0 < x1){
    for (int16_t x = x0; x <= x1; x++){
      SSD1351_write_pixel(x, y, color);
      if (D > 0){
        y = y + yi;
        D = D - 2*dx;
      }
      D = D + 2*dy;
    }
  }
  else{
    for (int16_t x = x0; x >= x1; x--){
      SSD1351_write_pixel(x, y, color);
      if (D > 0){
        y = y + yi;
        D = D - 2*dx;
      }
      D = D + 2*dy;
    }
  }

}

STATIC void SSD1351_draw_line_high(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
  int16_t dx = x1 - x0;
  int16_t dy = y1 - y0;
  int16_t xi = 1;
  if (dx < 0){
    xi = -1;
    dx = -dx;
  }
  int16_t D = 2*dx - dy;
  int16_t x = x0;

  if (y0 < y1){
    for (int16_t y = y0; y <= y1; y++){
      SSD1351_write_pixel(x, y, color);
      if (D > 0){
        x = x + xi;
        D = D - 2*dy;
      }
      D = D + 2*dx;
    }
  }
  else{
    for (int16_t y = y0; y >= y1; y--){
      SSD1351_write_pixel(x, y, color);
      if (D > 0){
        x = x + xi;
        D = D - 2*dy;
      }
      D = D + 2*dx;
    }
  }
}

/*
 * @brief Draws a line from specified parameters into display RAM
 * @param x0: starting x coordinate
 * @para y0: starting y coordinate
 * @param x1: ending x coordinate
 * @param y1: ending y coordinate
 * @color: color to use to draw the line
 * @reval None
 */
void SSD1351_draw_line(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color){
  if (abs(y1 - y0) < abs(x1 - x0)){
    if (x0 > x1){
      SSD1351_draw_line_low(x1, y1, x0, y0, color);
    }
    else{
      SSD1351_draw_line_low(x0, y0, x1, y1, color);
    }
  }
  else{
    if (y0 > y1){
      SSD1351_draw_line_high(x1, y1, x0, y0, color);
    }
    else{
      SSD1351_draw_line_high(x0, y0, x1, y1, color);
    }
  }
  return;
}

/*
 * @brief Draws a rectangle with specified dimensions into display RAM
 * @param x0: starting x coordinate
 * @para y0: starting y coordinate
 * @param w: width of the rectangle
 * @oaram h: height of the rectangle
 * @color: color for the border
 * @reval None
 */
void SSD1351_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
  SSD1351_draw_line(x, y, x + w, y, color);
  SSD1351_draw_line(x + w, y, x + w, y + h, color);
  SSD1351_draw_line(x + w, y + h, x, y + h, color);
  SSD1351_draw_line(x, y + h, x, y, color);
}

/*
 * @brief Draws a filled rectangle with specified dimensions into display RAM
 * @param x0: starting x coordinate
 * @para y0: starting y coordinate
 * @param w: width of the rectangle
 * @oaram h: height of the rectangle
 * @oaram color: color for the rectangle
 * @reval None
 */
void SSD1351_draw_filled_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color){
  for (int i = x; i < x + w; i++){
    for (int j = y; j < y + h; j++){
      SSD1351_write_pixel(i, j, color);
    }
  }
}

/*
 * @brief Draws a rotated filled rectangle with specified dimensions into display RAM
 * @param xc: center x coordinate
 * @para yc: center y coordinate
 * @param w: width of the rectangle
 * @param h: height of the rectangle
 * @param r: rotation in degrees
 * @oaram color: color for the rectangle
 * @reval None
 */
/*
void SSD1351_draw_rotated_rect(int16_t xc, int16_t yc, int16_t w, int16_t h, int16_t r, uint16_t color){
  r = r % 90;
  int16_t hyp = sqrt((h * h)/4 + (w * w)/4);
  float wr = 45  + r;
  float rad = (wr/360)*M_PI;
  int16_t x0 = xc - (hyp * sin(rad));
  int16_t y0 = yc + (hyp * cos(rad));
}*/

STATIC void draw_circle(int16_t xc, int16_t yc, int16_t x, int16_t y, uint16_t color){
  SSD1351_write_pixel(xc + x, yc + y, color);
  SSD1351_write_pixel(xc - x, yc + y, color);
  SSD1351_write_pixel(xc + x, yc - y, color);
  SSD1351_write_pixel(xc - x, yc - y, color);
  SSD1351_write_pixel(xc + y, yc + x, color);
  SSD1351_write_pixel(xc - y, yc + x, color);
  SSD1351_write_pixel(xc + y, yc - x, color);
  SSD1351_write_pixel(xc - y, yc - x, color);
}

STATIC void draw_filled_circle(int16_t xc, int16_t yc, int16_t x, int16_t y, uint16_t color){
  SSD1351_draw_line(xc - x, yc + y, xc + x, yc + y, color);
  SSD1351_draw_line(xc - x, yc - y, xc + x, yc - y, color);
  SSD1351_draw_line(xc -y, yc + x, xc + y, yc + x, color);
  SSD1351_draw_line(xc - y, yc - x, xc + y, yc - x, color);
}

/*
 * @brief Draws a cicle with specified origin and radius into display RAM
 * @param xc: integer for the x origin coordinate
 * @param yc: integer for the y origin coordinate
 * @param color: color for the border
 * @reval None
 */
void SSD1351_draw_circle(int16_t xc, int16_t yc, uint16_t r, uint16_t color){
  int x = 0, y = r;
  int d = 3 - 2 * r;
  draw_circle(xc, yc, x, y, color);
  while (y >= x){
    x++;
    if (d > 0){
      y--;
      d = d + 4 * (x - y) + 10;
    }
    else{
      d = d + 4 * x + 6;
    }
    draw_circle(xc, yc, x, y, color);
  }
}

/*
 * @brief Draws a cicle with specified origin and radius into display RAM
 * @param xc: integer for the x origin coordinate
 * @param yc: integer for the y origin coordinate
 * @param color: color for the circle
 * @reval None
 */
void SSD1351_draw_filled_circle(int16_t xc, int16_t yc, uint16_t r, uint16_t color){
  int x = 0, y = r;
  int d = 3 - 2 * r;
  draw_filled_circle(xc, yc, x, y, color);
  while (y >= x){
    x++;
    if (d > 0){
      y--;
      d = d + 4 * (x - y) + 10;
    }
    else{
      d = d + 4 * x + 6;
    }
    draw_filled_circle(xc, yc, x, y, color);
  }
}

STATIC void SSD1351_write_char(uint16_t color, font_t font, char c){
  uint16_t fd;
  if ((COLUMNS <= SSD1351_cursor.x + font.width) || (ROWS <= SSD1351_cursor.y + font.height)){
    return;
  }
  if (c == '\n'){
    SSD1351_cursor.x = 127;
  }
  else{
    for (int i = 0; i < font.height; i++){
        fd = font.data[(c - 32) * font.height + i];
        for (int j = 0; j < font.width; j++){
          if ((fd << j) & 0x8000){
            SSD1351_write_pixel(SSD1351_cursor.x + j, SSD1351_cursor.y + i, color);
          }
        }
      }
  }
  SSD1351_cursor.x += font.width;
  if ((SSD1351_cursor.x + font.width >= 127) & (SSD1351_cursor.y + font.height <= 127)){
    SSD1351_cursor.y = SSD1351_cursor.y + font.height + 2;
    SSD1351_cursor.x = 0;
  }
  return;
}

STATIC void SSD1351_write_string(uint16_t color, font_t font, char *line){
  if (line == NULL){
    return;
  }
  while (*line != 0){
    SSD1351_write_char(color, font, *line);
    line++;
  }
}

STATIC void SSD1351_write_int(uint16_t color, font_t font, int8_t n){
  char number[5];
  sprintf(number, "%i", n);
  SSD1351_write_string(color, font, number);
}

/*
 * @brief Prints a formatted string to the display
 * @param color: unsigned integer for the color of the string
 * @param font: structure holding the type of font
 * @param format: formatted string
 */
void SSD1351_printf(uint16_t color, font_t font, const char *format, ...){
  if (format == NULL){
    return;
  }
  va_list valist;
  va_start(valist, format);
  while (*format != 0){
    if (*format != '%'){
      SSD1351_write_char(color, font, *format);
      format++;
    }
    else{
      format++;
      switch(*format){
        case 's':
          SSD1351_write_string(color, font, va_arg(valist, char *));
          break;
        case 'c':
          SSD1351_write_char(color, font, va_arg(valist, int)); //?
          break;
        case 'i':
          SSD1351_write_int(color, font, (int8_t)va_arg(valist, int));
          break;
        default:
          break;
      }
      format++;
    }
  }
}

/*
 * @brief Sets the printing cursor to a positioin
 * @param x: integer for the x position for the cursor
 * @param y: integer for the y position for the cursor
 */

void SSD1351_set_cursor(uint8_t x, uint8_t y){
  SSD1351_cursor.x = x;
  SSD1351_cursor.y = y;
}

/*
 * @brief Draws a sprite
 * @param sp: pointer to struct holding sprite data
 */
/*
void SSD1351_draw_sprite(int16_t x, int16_t y, sprite *sp){
  for (int i = 0; i < sp->width; i++){
    for (int j = 0; j < sp->height; j++){
      uint16_t color = color_palette[sp->content[i + (j * sp->width)]];
      if (color != 0){
        SSD1351_write_pixel(x + i, y + j, color);
      }
    }
  }
}
*/
