
#include "nrf_drv_qspi.h"

#define CHIP_1ST_SECTOR 8192
#define CHIP_2ND_SECTOR 12288
#define CHIP_3RD_SECTOR 16384

void flash_erase(uint32_t addr, nrf_qspi_erase_len_t length); 

void flash_write(uint8_t* buffer_tx, uint32_t start_addr, size_t DATA_SIZE_BYTES);

void flash_read(uint8_t* buffer_tx, uint32_t start_addr, size_t DATA_SIZE_BYTES);

void qspi_init(void);

uint32_t seek_to_project(char* project_name, uint8_t name_length);

uint32_t seek_to_chip(uint32_t project_addr, char* chip_name, uint8_t name_length);

void file_header_write(uint32_t chip_addr, char* file_name, uint8_t* timestamp, uint32_t file_data_length, bool add_all_cmd);

uint32_t fetch_bytes_prog(void);

void flash_file_num_inc(uint32_t chip_addr_global);

void flash_file_dir_update(int file_data_length);

uint32_t flash_add_project(char*);

uint32_t flash_add_chip(uint32_t project_addr, char* chip_name, uint8_t* chip_id, bool include_file);
