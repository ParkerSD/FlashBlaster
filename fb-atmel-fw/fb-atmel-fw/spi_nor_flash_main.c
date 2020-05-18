/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file or main.c
 * to avoid loosing it when reconfiguring.
 */
#include "atmel_start.h"
#include "spi_nor_flash_main.h"
#include "conf_spi_nor_flash.h"

static struct s25fl116k SPI_NOR_FLASH_0_descr;

struct spi_nor_flash *SPI_NOR_FLASH_0;

/**
 * \brief Initialize Temperature Sensors
 */
void spi_nor_flash_init(void)
{

	qspi_sync_enable(&QUAD_SPI_0);
	SPI_NOR_FLASH_0 = s25fl116k_construct(&SPI_NOR_FLASH_0_descr.parent, &QUAD_SPI_0, CONF_SPI_NOR_FLASH_0_QUAD_MODE);
}
