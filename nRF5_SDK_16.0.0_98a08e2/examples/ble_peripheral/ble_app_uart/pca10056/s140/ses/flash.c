
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

#define QSPI_STD_CMD_WRSR   0x01
#define QSPI_STD_CMD_RSTEN  0x66
#define QSPI_STD_CMD_RST    0x99

#define QSPI_TEST_DATA_SIZE 256

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
    cinstr_cfg.opcode = QSPI_STD_CMD_WRSR;
    cinstr_cfg.length = NRF_QSPI_CINSTR_LEN_2B;
    err_code = nrf_drv_qspi_cinstr_xfer(&cinstr_cfg, &temporary, NULL);
    APP_ERROR_CHECK(err_code);
}


//set configuration registers
//check status registers (2)
//make continous read
//write function, writing can only drop bits so erase by sector and write
void qspi_init(void)
{
    uint32_t i;
    uint32_t err_code;

    for (i = 0; i < QSPI_TEST_DATA_SIZE; ++i)
    {
        m_buffer_tx[i] = 1;
    }

    nrf_drv_qspi_config_t config = NRF_DRV_QSPI_DEFAULT_CONFIG;

    err_code = nrf_drv_qspi_init(&config, qspi_handler, NULL); // used qspi_handler for arg 2
    APP_ERROR_CHECK(err_code);

    configure_memory();

    m_finished = false;
    err_code = nrf_drv_qspi_erase(NRF_QSPI_ERASE_LEN_64KB, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();
   // while(nrf_drv_qspi_mem_busy_check() == NRF_ERROR_BUSY); 

    err_code = nrf_drv_qspi_write(m_buffer_tx, QSPI_TEST_DATA_SIZE, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();

    err_code = nrf_drv_qspi_read(m_buffer_rx, QSPI_TEST_DATA_SIZE, 0);
    APP_ERROR_CHECK(err_code);
    WAIT_FOR_PERIPH();


    //    NRF_LOG_INFO("Compare...");
    if (memcmp(m_buffer_tx, m_buffer_rx, QSPI_TEST_DATA_SIZE) == 0)
    {
       nrf_gpio_pin_set(LED_GREEN);
       while(1); 
    }
    else
    {
    //   while(1); //"Data inconsistent"
    }


}