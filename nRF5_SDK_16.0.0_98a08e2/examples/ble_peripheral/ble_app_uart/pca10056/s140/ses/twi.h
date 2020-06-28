

#define SCL_PIN 16
#define SDA_PIN 14
#define TWI_INSTANCE_ID 1 
#define ATMEL_ADDRESS 0x10

// TWI CMDS //
#define start_byte 0xCC
#define target_cmd 0xBB // payload = data address, data length, target flash address, chip type
#define progress_cmd 0xAA // payload = one byte of flash write progress from atmel
#define error_cmd 0xEE // followed by error type

#define prog_complete 124 // equals number of vertical lines in progress bar drawing
#define prog_error 0xFD // used to exit proggramming loop 

//error type defines 
#define error_no_target 0x00
#define error_no_dbg_pwr 0x01
#define error_dbg_locked 0x02


void twi_init(void);

void twi_cmd_tx(uint8_t cmd, uint8_t* data, uint8_t data_length);

void twi_cmd_rx(uint8_t* rx_buffer, uint8_t length);

void atmel_reset(void);