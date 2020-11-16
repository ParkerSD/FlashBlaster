#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_drv_twi.h"
#include "nrfx_twi.h"
#include "nrfx_twim.h"
#include "twi.h"
#include "nrf_gpio.h"
#include "nrf_delay.h"
#include "oled.h"



static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


void twi_init(void)
{
    ret_code_t err_code;

    const nrf_drv_twi_config_t twi_config = {
       .scl                = SCL_PIN,
       .sda                = SDA_PIN,
       .frequency          = NRF_DRV_TWI_FREQ_400K,
       .interrupt_priority = APP_IRQ_PRIORITY_HIGH,
       .clear_bus_init     = false
    };

    err_code = nrf_drv_twi_init(&m_twi, &twi_config, NULL, NULL);
    APP_ERROR_CHECK(err_code);

    nrf_drv_twi_enable(&m_twi);
}


void twi_cmd_tx(uint8_t cmd, uint8_t* data, uint8_t data_length)
{
    /*
    nrf_drv_twi_tx(nrf_drv_twi_t const * p_instance,
                          uint8_t               address,
                          uint8_t const *       p_data,
                          uint8_t               length,
                          bool                  no_stop);
    */

    uint8_t packet_len = 1; //uint8_t packet_len = 2 + data_length;
    uint8_t packet_buff[packet_len];
    packet_buff[0] = start_byte;
    //packet_buff[1] = cmd;

    //for(int i = 0; i < data_length; i++)
    //{
    //    packet_buff[2+i] = *(data + i);
    //}

    nrf_drv_twi_tx(&m_twi, ATMEL_ADDRESS, packet_buff, packet_len, false);
}

void twi_cmd_rx(uint8_t* rx_buffer, uint8_t length)
{
    /*
    nrf_drv_twi_rx(nrf_drv_twi_t const * p_instance,
                              uint8_t               address,
                              uint8_t *             p_data,
                              uint8_t               length)
    {
    */
    nrf_drv_twi_rx(&m_twi, ATMEL_ADDRESS, rx_buffer, length);

}

