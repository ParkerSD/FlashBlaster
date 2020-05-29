

#define SCL_PIN 16
#define SDA_PIN 14
#define TWI_INSTANCE_ID 1 
#define ATMEL_ADDRESS 0x10

// TWI CMDS //
#define addr_cmd 0x01 
#define len_cmd 0x02 

void twi_init(void);

void twi_cmd_tx(uint8_t cmd, uint8_t* data, uint8_t data_length);