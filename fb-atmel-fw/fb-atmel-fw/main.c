#include <atmel_start.h>
#include "driver_examples.h"
#include "hal_gpio.h"

int main(void)
{
	/* Initializes MCU, drivers and middleware */
	atmel_start_init();
	
	//spi_nor_flash_read(const struct spi_nor_flash *const me, uint8_t *buf, uint32_t address, uint32_t length);

	uint8_t buff[4];
	
	//QUAD_SPI_0_example();
	gpio_set_pin_level(PB11,true); // nordic holding pin low? 
	while (1) 
	{   
		//gpio_set_pin_level(PB11,false);
		spi_nor_flash_read(SPI_NOR_FLASH_0, buff, 0, 4);
		//spi_nor_flash_write
		//gpio_set_pin_level(PB11,true);
		//I2C_0_example();
	}
}
