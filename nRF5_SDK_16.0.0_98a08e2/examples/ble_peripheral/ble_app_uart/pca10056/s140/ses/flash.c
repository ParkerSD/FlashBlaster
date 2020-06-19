
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
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

    nrf_gpio_cfg_output(19); // QSPI Pins
    nrf_gpio_cfg_output(17); 
    nrf_gpio_cfg_output(32); 
    nrf_gpio_cfg_output(21); 
    nrf_gpio_cfg_output(22);
    nrf_gpio_cfg_output(23);

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
       nrf_gpio_pin_set(LED_ORANGE); //"Data Consistent"
    }
#endif
}

void qspi_deinit(void)
{

    nrf_drv_qspi_uninit();

    nrf_gpio_cfg_default(19); // QSPI Pins
    nrf_gpio_cfg_default(17); 
    nrf_gpio_cfg_default(32); 
    nrf_gpio_cfg_default(21); 
    nrf_gpio_cfg_default(22);
    nrf_gpio_cfg_default(23);

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
    uint32_t current_addr = PROJECTS_START_ADDR;
 
    flash_read(proj_num, DIRECTORY_START_ADDR, WORD_SIZE); // read global project total from directory
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


uint32_t fetch_bytes_prog(void)
{
    uint32_t bytes_prog; 
    uint8_t bytes_prog_buff[WORD_SIZE]; 
    flash_read(bytes_prog_buff, FILE_BYTES_PROG_ADDR, WORD_SIZE);// calc beginning of file data address
    bytes_prog = bytes_prog_buff[0] << 24 | bytes_prog_buff[1] << 16 | bytes_prog_buff[2] << 8 | bytes_prog_buff[3];
  
    return bytes_prog; 
}





void flash_file_num_inc(uint32_t chip_addr) //increment file_num in chip parent in flash
{   
    //NOTE cannot use chip_addr, must seek to beginning of containing sector
    // Chip section starts at address 8192 and is 12288 bytes long,  
    // 3 x 4KB sectors --- 1st addr: 8192 (0x2000), 2nd addr: 12288 (0x3000), 3rd addr: 16384 (0x4000)
    uint32_t sector_addr; 
    uint32_t chip_addr_offset;
    uint32_t total_offset;
    uint8_t data_buff[FLASH_SECTOR_SIZE];

    if(chip_addr >= CHIP_3RD_SECTOR)
    {
        sector_addr = CHIP_3RD_SECTOR;
    }
    else if(chip_addr < CHIP_3RD_SECTOR && chip_addr >= CHIP_2ND_SECTOR)
    {
        sector_addr = CHIP_2ND_SECTOR;
    }
    else if(chip_addr < CHIP_2ND_SECTOR)
    {
        sector_addr = CHIP_1ST_SECTOR;
    }

    chip_addr_offset = chip_addr - sector_addr;
    total_offset = chip_addr_offset + FILE_NUM_OFFSET;
    
    flash_read(data_buff, sector_addr, FLASH_SECTOR_SIZE); 
    flash_erase(sector_addr, NRF_QSPI_ERASE_LEN_4KB);
    
    uint32_t file_num_chip; 
    file_num_chip = data_buff[total_offset] << 24 | data_buff[total_offset+1] << 16 | data_buff[total_offset+2] << 8 | data_buff[total_offset+3];
    file_num_chip++;
    data_buff[total_offset] = (file_num_chip >> 24) & 0xFF; //bit shift 32bit int into 8bit array 
    data_buff[total_offset+1] = (file_num_chip >> 16) & 0xFF;
    data_buff[total_offset+2] = (file_num_chip >> 8) & 0xFF;
    data_buff[total_offset+3] = file_num_chip & 0xFF;

    flash_write(data_buff, sector_addr, FLASH_SECTOR_SIZE);
}


void flash_file_dir_update(int file_data_length) // increment file count global and update file bytes programmed in directory
{   
    float a = file_data_length;
    float b = BLE_PACKET_SIZE;
    double div_result = a/b; 
    int file_bytes_prog = ceil(div_result) * BLE_PACKET_SIZE; //file data programmed in 244 chunks, may not be exact data length of file
    
    uint8_t data_buff[FLASH_SECTOR_SIZE]; 
    flash_read(data_buff, DIRECTORY_START_ADDR, FLASH_SECTOR_SIZE);
    flash_erase(DIRECTORY_START_ADDR, NRF_QSPI_ERASE_LEN_4KB); 

    uint32_t file_count;
    file_count = data_buff[FILE_COUNT_GLOBAL_ADDR] << 24 | data_buff[FILE_COUNT_GLOBAL_ADDR+1] << 16 | data_buff[FILE_COUNT_GLOBAL_ADDR+2] << 8 | data_buff[FILE_COUNT_GLOBAL_ADDR+3]; 
    file_count++; 
    data_buff[FILE_COUNT_GLOBAL_ADDR] = (file_count >> 24) & 0xFF; //bit shift 32bit int into 8bit array 
    data_buff[FILE_COUNT_GLOBAL_ADDR+1] = (file_count >> 16) & 0xFF;
    data_buff[FILE_COUNT_GLOBAL_ADDR+2] = (file_count >> 8) & 0xFF;
    data_buff[FILE_COUNT_GLOBAL_ADDR+3] = file_count & 0xFF;

    uint32_t total_bytes_prog; 
    total_bytes_prog = data_buff[FILE_BYTES_PROG_ADDR] << 24 | data_buff[FILE_BYTES_PROG_ADDR+1] << 16 | data_buff[FILE_BYTES_PROG_ADDR+2] << 8 | data_buff[FILE_BYTES_PROG_ADDR+3]; 
    total_bytes_prog += file_bytes_prog;
    data_buff[FILE_BYTES_PROG_ADDR] = (total_bytes_prog >> 24) & 0xFF; //bit shift 32bit int into 8bit array 
    data_buff[FILE_BYTES_PROG_ADDR+1] = (total_bytes_prog >> 16) & 0xFF;
    data_buff[FILE_BYTES_PROG_ADDR+2] = (total_bytes_prog >> 8) & 0xFF;
    data_buff[FILE_BYTES_PROG_ADDR+3] = total_bytes_prog & 0xFF;

    flash_write(data_buff, DIRECTORY_START_ADDR, FLASH_SECTOR_SIZE);
}


uint32_t flash_add_project(char* project_name)
{
    uint32_t num_projects;
    uint8_t num_projects_buff[WORD_SIZE];
    uint8_t project_addr_buff[WORD_SIZE]; 
    uint8_t sector_buff[FLASH_SECTOR_SIZE]; //buffer directory for modification

    flash_read(num_projects_buff, ADDR_NUM_PROJECTS, WORD_SIZE); //read project_num total
    num_projects = num_projects_buff[0] << 24 | num_projects_buff[1] << 16 | num_projects_buff[2] << 8 | num_projects_buff[3];
    
    //update project sector
    flash_read(sector_buff, PROJECTS_START_ADDR, FLASH_SECTOR_SIZE); //buffer project sector for modification
    flash_erase(PROJECTS_START_ADDR, NRF_QSPI_ERASE_LEN_4KB);
    uint8_t chip_num_init[WORD_SIZE] = {0, 0, 0, 0};
    uint32_t project_addr_sector = num_projects * MAX_PROJECT_SIZE;
    memcpy(&sector_buff[project_addr_sector], project_name, MAX_STRING_SIZE); // copy project name string to buffer
    memcpy(&sector_buff[project_addr_sector + MAX_STRING_SIZE], chip_num_init, WORD_SIZE); // copy chip_num to buffer
    flash_write(sector_buff, PROJECTS_START_ADDR, FLASH_SECTOR_SIZE); //rewrite modified sector buffer

    //update directory
    uint32_t new_project_addr = (num_projects * MAX_PROJECT_SIZE) + PROJECTS_START_ADDR; //calc new project start address 
    uint32_t new_project_dir_addr = (num_projects * WORD_SIZE) + ADDR_PROJECT_PTR_FIRST; //project addr in directory
    num_projects++; //increment num projects
    flash_read(sector_buff, DIRECTORY_START_ADDR, FLASH_SECTOR_SIZE);
    flash_erase(DIRECTORY_START_ADDR, NRF_QSPI_ERASE_LEN_4KB); 

    sector_buff[0] = (num_projects >> 24) & 0xFF;  
    sector_buff[1] = (num_projects >> 16) & 0xFF;
    sector_buff[2] = (num_projects >> 8) & 0xFF;
    sector_buff[3] = num_projects & 0xFF;

    sector_buff[new_project_dir_addr] = (new_project_addr >> 24) & 0xFF;  
    sector_buff[new_project_dir_addr+1] = (new_project_addr >> 16) & 0xFF;
    sector_buff[new_project_dir_addr+2] = (new_project_addr >> 8) & 0xFF;
    sector_buff[new_project_dir_addr+3] = new_project_addr & 0xFF;

    flash_write(sector_buff, DIRECTORY_START_ADDR, FLASH_SECTOR_SIZE); //rewrite directory

    return new_project_addr; 
}


uint32_t flash_add_chip(uint32_t project_addr, char* chip_name, uint8_t* chip_id, bool add_all_cmd)
{
    uint32_t num_chips_global;
    uint8_t num_chips_buff[WORD_SIZE];
    uint8_t chip_addr_buff[WORD_SIZE]; 
    uint8_t sector_buff[CHIP_SECTOR_SIZE];
    uint8_t file_num_init[WORD_SIZE] = {0, 0, 0, 0}; 

    flash_read(num_chips_buff, CHIP_COUNT_GLOBAL_ADDR, WORD_SIZE); //read chip_num_global total
    num_chips_global = num_chips_buff[0] << 24 | num_chips_buff[1] << 16 | num_chips_buff[2] << 8 | num_chips_buff[3];
    
    if(add_all_cmd)
    {
      file_num_init[3] = 1; //increment file_num if add_all_cmd used
    }

    //update chip sector with new chip 
    uint32_t new_chip_sector_addr = num_chips_global * MAX_CHIP_SIZE;
    flash_read(sector_buff, CHIPS_START_ADDR, CHIP_SECTOR_SIZE);
    flash_erase(CHIPS_START_ADDR, NRF_QSPI_ERASE_LEN_4KB); //erase first sector 
    flash_erase(CHIPS_START_ADDR + FLASH_SECTOR_SIZE, NRF_QSPI_ERASE_LEN_4KB); //erase second sector 
    flash_erase(CHIPS_START_ADDR + (2*FLASH_SECTOR_SIZE), NRF_QSPI_ERASE_LEN_4KB); //erase third sector 
    memcpy(&sector_buff[new_chip_sector_addr], chip_name, MAX_STRING_SIZE); //copy new chip name into sector
    memcpy(&sector_buff[new_chip_sector_addr + MAX_STRING_SIZE], chip_id, WORD_SIZE); // chip_id
    memcpy(&sector_buff[new_chip_sector_addr + FILE_NUM_OFFSET], file_num_init, WORD_SIZE); 
    flash_write(sector_buff, CHIPS_START_ADDR, CHIP_SECTOR_SIZE); // rewrite sector 
    
    //update directory
    flash_read(sector_buff, DIRECTORY_START_ADDR, FLASH_SECTOR_SIZE); 
    flash_erase(DIRECTORY_START_ADDR, NRF_QSPI_ERASE_LEN_4KB);
    num_chips_global++;
    sector_buff[CHIP_COUNT_GLOBAL_ADDR] = (num_chips_global >> 24) & 0xFF;  
    sector_buff[CHIP_COUNT_GLOBAL_ADDR+1] = (num_chips_global >> 16) & 0xFF;
    sector_buff[CHIP_COUNT_GLOBAL_ADDR+2] = (num_chips_global >> 8) & 0xFF;
    sector_buff[CHIP_COUNT_GLOBAL_ADDR+3] = num_chips_global & 0xFF;
    flash_write(sector_buff, DIRECTORY_START_ADDR, FLASH_SECTOR_SIZE);

    //update parent project
    uint32_t new_chip_addr = new_chip_sector_addr + CHIPS_START_ADDR;
    flash_read(sector_buff, PROJECTS_START_ADDR, FLASH_SECTOR_SIZE); 
    flash_erase(PROJECTS_START_ADDR, NRF_QSPI_ERASE_LEN_4KB);
    //add chip_addr_ptr to project
    uint32_t project_sector_addr = project_addr - PROJECTS_START_ADDR; 
    uint32_t num_chips_project = sector_buff[project_sector_addr + CHIP_NUM_OFFSET] << 24 | sector_buff[project_sector_addr + CHIP_NUM_OFFSET + 1] << 16 | sector_buff[project_sector_addr + CHIP_NUM_OFFSET + 2] << 8 | sector_buff[project_sector_addr + CHIP_NUM_OFFSET + 3];
    uint32_t chip_ptr_addr = PROJECT_HEADER_SIZE + (num_chips_project * WORD_SIZE);
    sector_buff[project_sector_addr + chip_ptr_addr] = (new_chip_addr >> 24) & 0xFF;  
    sector_buff[project_sector_addr + chip_ptr_addr+1] = (new_chip_addr >> 16) & 0xFF;
    sector_buff[project_sector_addr + chip_ptr_addr+2] = (new_chip_addr >> 8) & 0xFF;
    sector_buff[project_sector_addr + chip_ptr_addr+3] = new_chip_addr & 0xFF;
    //increment chip_num in project
    num_chips_project++;
    sector_buff[project_sector_addr + CHIP_NUM_OFFSET] = (num_chips_project >> 24) & 0xFF;  
    sector_buff[project_sector_addr + CHIP_NUM_OFFSET+1] = (num_chips_project >> 16) & 0xFF;
    sector_buff[project_sector_addr + CHIP_NUM_OFFSET+2] = (num_chips_project >> 8) & 0xFF;
    sector_buff[project_sector_addr + CHIP_NUM_OFFSET+3] = num_chips_project & 0xFF;
    flash_write(sector_buff, PROJECTS_START_ADDR, FLASH_SECTOR_SIZE);

    return new_chip_addr;
}


void file_header_write(uint32_t chip_addr, char* file_name, uint8_t* timestamp, uint32_t file_data_length, bool add_all_cmd)
{   
    uint32_t file_count; 
    uint8_t file_count_buff[WORD_SIZE]; 

    // read global file count to detemine where to place in file sector 
    flash_read(file_count_buff, FILE_COUNT_GLOBAL_ADDR, WORD_SIZE);
    file_count = file_count_buff[0] << 24 | file_count_buff[1] << 16 | file_count_buff[2] << 8 | file_count_buff[3];

    //write file name string in file flash section
    uint32_t curr_file_addr = DIRECTORY_OFFSET + PROJECT_SECTOR_OFFSET + CHIP_SECTOR_OFFSET + (FILE_HEADER_SIZE * file_count); // determine address in file section
    flash_write(file_name, curr_file_addr, MAX_STRING_SIZE);    
    //flash_write(timestamp, curr_file_addr + MAX_STRING_SIZE, WORD_SIZE); //TODO fetch and write timestamp
    
    //write data length to file
    uint8_t file_data_length_buff[WORD_SIZE];
    file_data_length_buff[0] = (file_data_length >> 24) & 0xFF; //bit shift 32bit int into 8bit array 
    file_data_length_buff[1] = (file_data_length >> 16) & 0xFF;
    file_data_length_buff[2] = (file_data_length >> 8) & 0xFF;
    file_data_length_buff[3] = file_data_length & 0xFF;
    flash_write(file_data_length_buff, curr_file_addr + MAX_STRING_SIZE + WORD_SIZE, WORD_SIZE); 
    
    //write data address into file
    uint32_t bytes_prog = fetch_bytes_prog(); 
    uint32_t file_data_addr = bytes_prog + DATA_SECTOR_START; 
    uint8_t file_data_addr_buff[WORD_SIZE];
    file_data_addr_buff[0] = (file_data_addr >> 24) & 0xFF; //bit shift 32bit int into 8bit array 
    file_data_addr_buff[1] = (file_data_addr >> 16) & 0xFF;
    file_data_addr_buff[2] = (file_data_addr >> 8) & 0xFF;
    file_data_addr_buff[3] = file_data_addr & 0xFF;
    flash_write(file_data_addr_buff, curr_file_addr + FILE_DATA_ADDR_OFFSET, WORD_SIZE);   
    
    //add file address pointer to chip
    uint32_t file_num_chip;
    uint8_t file_num_chip_buff[WORD_SIZE]; 
    flash_read(file_num_chip_buff, chip_addr + FILE_NUM_OFFSET, WORD_SIZE); //read number of files in chip
    file_num_chip = file_num_chip_buff[0] << 24 | file_num_chip_buff[1] << 16 | file_num_chip_buff[2] << 8 | file_num_chip_buff[3];

    if(add_all_cmd)
    {
        file_num_chip--; //chip was initted with 1 file and casues offset in file_addr_chip if add_all_cmd was used
    }

    uint32_t file_addr_chip = chip_addr + CHIP_HEADER_SIZE + (file_num_chip * WORD_SIZE);
    uint8_t curr_file_addr_buff[WORD_SIZE];
    curr_file_addr_buff[0] = (curr_file_addr >> 24) & 0xFF; //bit shift 32bit int into 8bit array 
    curr_file_addr_buff[1] = (curr_file_addr >> 16) & 0xFF;
    curr_file_addr_buff[2] = (curr_file_addr >> 8) & 0xFF;
    curr_file_addr_buff[3] = curr_file_addr & 0xFF;
    flash_write(curr_file_addr_buff, file_addr_chip, WORD_SIZE); //write file address in chip

  //NOTE FOR TEST 
//  uint8_t data_buff[500];
//  flash_read(data_buff, chip_addr, 500);
//  m_finished = true;
                
                
 
    // write file header data (name, timestamp, datalength, data_addr)
}