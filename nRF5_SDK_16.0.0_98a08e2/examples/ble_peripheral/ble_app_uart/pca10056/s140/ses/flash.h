
#include "nrf_drv_qspi.h"

void flash_erase(uint32_t addr, nrf_qspi_erase_len_t length); 

void flash_write(uint8_t* buffer_tx, uint32_t start_addr, size_t DATA_SIZE_BYTES);

void flash_read(uint8_t* buffer_tx, uint32_t start_addr, size_t DATA_SIZE_BYTES);

void qspi_init(void);

uint32_t seek_to_project(char* project_name, uint8_t length);

uint32_t seek_to_chip(uint32_t project_addr, char* chip_name, uint8_t length);

void file_header_write(uint32_t chip_addr, char* file_name, uint8_t* timestamp, uint32_t file_data_length);

void file_data_write(uint32_t file_addr);