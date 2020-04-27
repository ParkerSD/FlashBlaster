
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "nrf_drv_qspi.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "app_error.h"
#include "flash.h" 
#include "sdk_config.h" 
#include "oled.h"
#include "nrf_gpio.h"
#include "system.h"

#define QSPI_TEST 0 //set true to enable qspi test 

#define QSPI_STD_CMD_WRSR   0x01
#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99
#define QSPI_STD_CMD_REMS   0x90 // REMS - 2 bytes
#define QSPI_STD_CMD_RES    0xAB // RES Read Electronic Signature - 1 byte
#define QSPI_STD_CMD_RDID   0x9F // RDID

#define QSPI_TEST_DATA_SIZE 32

#define WAIT_FOR_PERIPH() do { \
        while (!m_finished) {} \
        m_finished = false;    \
    } while (0)


static volatile bool m_finished = false;
static uint8_t m_buffer_tx[QSPI_TEST_DATA_SIZE];
static uint8_t m_buffer_rx[QSPI_TEST_DATA_SIZE];


static void qspi_handler(nrf_drv_qspi_evt_t event, void * p_context)
{
    if(event == NRF_DRV_QSPI_EVENT_DONE)
    {
        m_finished = true;
    }
}


static void configure_memory()
{
    uint8_t temporary = 0x40; //NOTE was 0x40

    uint32_t err_code;
    nrf_qspi_cinstr_conf_t cinstr_cfg = {
        .opcode    = QSPI_STD_CMD_RSTEN,
        .length    = NRF_QSPI_CINSTR_LEN_1B,
        .io2_level = true,
        .io3_level = true,
        .wipwait   = true,
        .wren      = true
    };

    // Send reset enable
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Send reset command
    cinstr_cfg.opcode = QSPI_STD_CMD_RST;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    // Switch to qspi mode
//    cinstr_cfg.opcode = QSPI_STD_CMD_WRSR; //TODO: NOT CONVERTING TO QUAD MODE
//    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
//    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, &temporary, NULL);
//    APP_ERROR_CHECK(err_code);
}


//set configuration registers
//check status registers (2)
//make continous read
//write function, writing can only drop bits so erase by sector and write
void qspi_init(void)
{
    uint32_t err_code;

    nrf_drv_qspi_config_t config = NRF_DRV_QSPI_DEFAULT_CONFIG;

    err_code = nrf_drv_qspi_init(&config, qspi_handler, NULL); // used qspi_handler for arg 2
    APP_ERROR_CHECK(err_code);

    configure_memory();

#if QSPI_TEST
    uint32_t i;
    for (i = 0; i < QSPI_TEST_DATA_SIZE; ++i)
    {
        m_buffer_tx[i] = 0x7;
    }
    m_finished = false;
    err_code = nrf_drv_qspi_erase(NRF_QSPI_ERASE_LEN_64KB, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH(); 

    err_code = nrf_drv_qspi_write(m_buffer_tx, QSPI_TEST_DATA_SIZE, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();

    err_code = nrf_drv_qspi_read(m_buffer_rx, QSPI_TEST_DATA_SIZE, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();


    // Compare Buffers
    if (memcmp(m_buffer_tx, m_buffer_rx, QSPI_TEST_DATA_SIZE) == 0)
    {
       nrf_gpio_pin_set(LED_GREEN); //"Data Consistent"
    }
#endif
}


void flash_erase(uint32_t start_addr, nrf_qspi_erase_len_t length) // NRF_QSPI_ERASE_LEN_4KB, NRF_QSPI_ERASE_LEN_64KB, NRF_QSPI_ERASE_LEN_ALL
{
    m_finished = false;
    uint32_t err_code;
    err_code = nrf_drv_qspi_erase(length, start_addr); 
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
}


void flash_write(uint8_t* buffer_tx, uint32_t start_addr, size_t DATA_SIZE_BYTES)
{
    m_finished = false;
    uint32_t err_code;
    err_code = nrf_drv_qspi_write(buffer_tx, DATA_SIZE_BYTES, start_addr);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
}


void flash_read(uint8_t* buffer_rx, uint32_t start_addr, size_t DATA_SIZE_BYTES)
{ 
    m_finished = false;
    uint32_t err_code;
    err_code = nrf_drv_qspi_read(buffer_rx, DATA_SIZE_BYTES, start_addr);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
}


uint32_t seek_to_project(char* project_name, uint8_t name_length) // NULL return = no project found
{   
    int n = 1; 
    char name_str[MAX_STRING_SIZE]; 
    uint32_t project_addr;
    
    uint8_t proj_num[WORD_SIZE]; 
    uint32_t projects_total_num; 
    uint32_t current_addr = 4000; //projects start at flash addr 4000
 
    flash_read(proj_num, 0, WORD_SIZE); // read global project total from directory
    projects_total_num = proj_num[0] << 24 | proj_num[1] << 16 | proj_num[2] << 8 | proj_num[3];

    for(int i = 0; i < projects_total_num; i++) //for total number of projects
    {
        flash_read(name_str, current_addr + (52*i), MAX_STRING_SIZE);  //check string every 52 bytes
        n = memcmp(name_str, project_name, name_length); //read and compare 
        project_addr = current_addr + (52*i); 
        
        if(!n)//strings match
        {
            return project_addr;//return address of found project
        }
    }
    return NULL; 
}



uint32_t seek_to_chip(uint32_t project_addr, char* chip_name, uint8_t name_length)
{   
    int n = 1; 
    char name_str[MAX_STRING_SIZE]; 
    uint32_t chip_addr; 
    uint8_t chip_addr_buff[WORD_SIZE];

    uint8_t chip_num[WORD_SIZE]; 
    uint32_t chip_total_num; 

    flash_read(chip_num, project_addr + MAX_STRING_SIZE, WORD_SIZE); // read chip_num from project 
    chip_total_num = chip_num[0] << 24 | chip_num[1] << 16 | chip_num[2] << 8 | chip_num[3];

    for(int i = 0; i < chip_total_num; i++)
    {
        flash_read(chip_addr_buff, project_addr + PROJECT_HEADER_SIZE + (i*WORD_SIZE), WORD_SIZE); //get address from project 
        chip_addr = chip_addr_buff[0] << 24 | chip_addr_buff[1] << 16 | chip_addr_buff[2] << 8 | chip_addr_buff[3];

        flash_read(name_str, chip_addr, MAX_STRING_SIZE);//NOTE MUST READ IN WORDS, read address 
        n = memcmp(name_str, chip_name, name_length); //read and compare 

        if(!n)//strings match
        {
            return chip_addr;//return address of found chip
        }
    }
    return NULL;
}



//flash_write(uint8_t* buffer_tx, uint32_t start_addr, size_t DATA_SIZE_BYTES)

void file_header_write(uint32_t chip_addr, char* file_name, uint8_t* timestamp, uint32_t file_data_length)
{   
    uint32_t file_count; 
    uint8_t file_count_buff[WORD_SIZE]; 
    
    flash_read(file_count_buff, FILE_COUNT_GLOBAL_ADDR, WORD_SIZE);// read global file count 
    file_count = file_count_buff[0] << 24 | file_count_buff[1] << 16 | file_count_buff[2] << 8 | file_count_buff[3];

    uint32_t curr_file_addr = DIRECTORY_OFFSET + PROJECT_SECTOR_OFFSET + CHIP_SECTOR_OFFSET + (FILE_HEADER_SIZE * file_count); // determine address in file section
    flash_write(file_name, curr_file_addr, MAX_STRING_SIZE);    //write file name string in file flash section
    //flash_write(timestamp, curr_file_addr + MAX_STRING_SIZE, WORD_SIZE); //TODO fetch and write timestamp

    uint8_t file_data_length_buff[WORD_SIZE];
    file_data_length_buff[0] = (file_data_length >> 24) & 0xFF; //bit shift 32bit int into 8bit array 
    file_data_length_buff[1] = (file_data_length >> 16) & 0xFF;
    file_data_length_buff[2] = (file_data_length >> 8) & 0xFF;
    file_data_length_buff[3] = file_data_length & 0xFF;
    flash_write(file_data_length_buff, curr_file_addr + MAX_STRING_SIZE + WORD_SIZE, WORD_SIZE); //write data length

    uint32_t file_num_chip;
    uint8_t file_num_chip_buff[WORD_SIZE]; 
    flash_read(file_num_chip_buff, chip_addr + FILE_NUM_OFFSET, WORD_SIZE); //read number of files in chip
    file_num_chip = file_num_chip_buff[0] << 24 | file_num_chip_buff[1] << 16 | file_num_chip_buff[2] << 8 | file_num_chip_buff[3];
    
    uint32_t file_addr_chip = chip_addr + CHIP_HEADER_SIZE + (file_num_chip * WORD_SIZE);
    uint8_t curr_file_addr_buff[WORD_SIZE];
    curr_file_addr_buff[0] = (curr_file_addr >> 24) & 0xFF; //bit shift 32bit int into 8bit array 
    curr_file_addr_buff[1] = (curr_file_addr >> 16) & 0xFF;
    curr_file_addr_buff[2] = (curr_file_addr >> 8) & 0xFF;
    curr_file_addr_buff[3] = curr_file_addr & 0xFF;
    flash_write(curr_file_addr_buff, file_addr_chip, WORD_SIZE); //write file address in chip
 
    // write file header data (name, timestamp, datalength, data_addr) 
    //TODO write data address of file into file item
    //(AFTER PROGRAMMING COMPLETE)
    // increment file_num in chip parent
    // increment file_count_global
    // increment file_bytes_programmed

}


