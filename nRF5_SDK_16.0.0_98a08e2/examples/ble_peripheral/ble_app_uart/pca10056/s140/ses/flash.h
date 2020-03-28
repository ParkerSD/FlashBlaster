
#include "nrf_drv_qspi.h"

void flash_erase(uint32_t addr, nrf_qspi_erase_len_t length); 

void flash_write(uint8_t* buffer_tx, uint32_t start_addr, size_t DATA_SIZE_BYTES);

void flash_read(uint8_t* buffer_tx, uint32_t start_addr, size_t DATA_SIZE_BYTES);

void qspi_init(void);