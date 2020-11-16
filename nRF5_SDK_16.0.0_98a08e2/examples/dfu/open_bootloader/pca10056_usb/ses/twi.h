

#define SCL_PIN 16
#define SDA_PIN 14
#define TWI_INSTANCE_ID 1 
#define ATMEL_ADDRESS 0x10

// TWI CMDS //
#define start_byte 0xCC
#define target_cmd 0xBB // payload = data address, data length, target flash address, chip type
#define progress_cmd 0xAA // payload = one byte of flash write progress from atmel
#define error_cmd 0xEE // followed by error type
#define shutdwn_cmd 0xF1 //shut down atmel

#define prog_complete 123 // equals number of vertical lines in progress bar drawing
#define prog_error 0xFD // used to exit proggramming loop 

//error type defines 
#define ERROR_NO_TARGET 0x00
#define ERROR_NO_DBG_PWR 0x01
#define ERROR_DBG_LOCKED 0x02
#define ERROR_CLIENT_TIMEOUT 0x03
#define ERROR_SERVER_TIMEOUT 0x04
#define PROJECT_LIMIT_REACHED 0x05
#define CHIP_LIMIT_REACHED 0x06
#define FILE_LIMIT_REACHED 0x07



void twi_init(void);

void twi_cmd_tx(uint8_t cmd, uint8_t* data, uint8_t data_length);

void twi_cmd_rx(uint8_t* rx_buffer, uint8_t length);
