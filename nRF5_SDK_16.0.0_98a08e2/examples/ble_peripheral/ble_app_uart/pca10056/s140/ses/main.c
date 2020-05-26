/**
 * Copyright (c) 2014 - 2019, Nordic Semiconductor ASA
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 *    list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form, except as embedded into a Nordic
 *    Semiconductor ASA integrated circuit in a product or a software update for
 *    such product, must reproduce the above copyright notice, this list of
 *    conditions and the following disclaimer in the documentation and/or other
 *    materials provided with the distribution.
 *
 * 3. Neither the name of Nordic Semiconductor ASA nor the names of its
 *    contributors may be used to endorse or promote products derived from this
 *    software without specific prior written permission.
 *
 * 4. This software, with or without modification, must only be used with a
 *    Nordic Semiconductor ASA integrated circuit.
 *
 * 5. Any software provided in binary form under this license must not be reverse
 *    engineered, decompiled, modified and/or disassembled.
 *
 * THIS SOFTWARE IS PROVIDED BY NORDIC SEMICONDUCTOR ASA "AS IS" AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY, NONINFRINGEMENT, AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL NORDIC SEMICONDUCTOR ASA OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
 * OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */
/** @file
 *
 * @defgroup ble_sdk_uart_over_ble_main main.c
 * @{
 * @ingroup  ble_sdk_app_nus_eval
 * @brief    UART over BLE application main file.
 *
 * This file contains the source code for a sample application that uses the Nordic UART service.
 * This application uses the @ref srvlib_conn_params module.
 */


#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "ble_hci.h"
#include "ble_advdata.h"
#include "ble_advertising.h"
#include "ble_conn_params.h"
#include "nrf_sdh.h"
#include "nrf_sdh_soc.h"
#include "nrf_sdh_ble.h"
#include "nrf_ble_gatt.h"
#include "nrf_ble_qwr.h"
#include "app_timer.h"
#include "ble_nus.h"
#include "app_uart.h"
#include "app_util_platform.h"
#include "bsp_btn_ble.h"
#include "nrf_pwr_mgmt.h"
#include "nrf_drv_twi.h"
#include "nrfx_twi.h"
#include "nrfx_twim.h"
#include "nrf_drv_spi.h"
#include "oled.h"
#include "app_button.h"
#include "nrf_gpio.h"
#include "button.h"
#include "ssd1351.h"
#include "usb.h"
#include "app_scheduler.h"
#include "nrf_power.h"
#include "system.h"
#include "flash.h"
#include "battery.h"
#include "ble.h"
#include "nrfx_wdt.h"
#include "nrf_wdt.h"
#include "nrf_drv_wdt.h"
#include "nrf_drv_twi.h"
#include "nrfx_twi.h"
#include "nrfx_twim.h"


#if defined (UART_PRESENT)
#include "nrf_uart.h"
#endif
#if defined (UARTE_PRESENT)
#include "nrf_uarte.h"
#endif


#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"

#define SCHED_MAX_EVENT_DATA_SIZE       APP_TIMER_SCHED_EVENT_DATA_SIZE     // Maximum size of scheduler events.  
#ifdef SVCALL_AS_NORMAL_FUNCTION 
#define SCHED_QUEUE_SIZE                20                                  // Maximum number of events in the scheduler queue. More is needed in case of Serialization.  
#else 
#define SCHED_QUEUE_SIZE                10                                  // Maximum number of events in the scheduler queue.  
#endif 


#define SCL_PIN 16
#define SDA_PIN 14
#define TWI_INSTANCE_ID 1 
static const nrf_drv_twi_t m_twi = NRF_DRV_TWI_INSTANCE(TWI_INSTANCE_ID);


static nrf_drv_wdt_channel_id wdt_channel_id; 


/**@brief Function for initializing the timer module.
 */
static void timers_init(void)
{
    ret_code_t err_code = app_timer_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for initializing the nrf log module.
 */
void log_init(void)
{
    ret_code_t err_code = NRF_LOG_INIT(NULL);
    APP_ERROR_CHECK(err_code);

    NRF_LOG_DEFAULT_BACKENDS_INIT();
}


/**@brief Function for initializing power management.
 */
void power_management_init(void)
{
    ret_code_t err_code;
    err_code = nrf_pwr_mgmt_init();
    APP_ERROR_CHECK(err_code);
}


/**@brief Function for handling the idle state (main loop).
 *
 * @details If there is no pending log operation, then sleep until next the next event occurs.
 */
void idle_state_handle(void)
{
    if (NRF_LOG_PROCESS() == false)
    {
        nrf_pwr_mgmt_run();
        NRF_WDT->RR[0] = WDT_RR_RR_Reload; // reset watchdog timer
    }
}

 
static void scheduler_init(void) 
{ 
    APP_SCHED_INIT(SCHED_MAX_EVENT_DATA_SIZE, SCHED_QUEUE_SIZE); 
} 

void wdt_error_handler(void)
{
    //throw error
}

void watchdog_init(void)
{
    uint32_t op_status = NRF_SUCCESS;
    nrf_drv_wdt_config_t watchdogConfig; 
    watchdogConfig.behaviour = NRF_WDT_BEHAVIOUR_PAUSE_SLEEP_HALT;
    watchdogConfig.reload_value = 15000; // increase to extend watchdog timeout 
    watchdogConfig.interrupt_priority = APP_IRQ_PRIORITY_LOW;
    op_status = nrf_drv_wdt_init(&watchdogConfig, wdt_error_handler);
    APP_ERROR_CHECK(op_status);
    op_status = nrf_drv_wdt_channel_alloc(&wdt_channel_id);
    APP_ERROR_CHECK(op_status);
    nrf_drv_wdt_enable();

/*  //--DST implementation--//
    NRF_WDT->CONFIG = (WDT_CONFIG_HALT_Pause << WDT_CONFIG_HALT_Pos) | ( WDT_CONFIG_SLEEP_Run << WDT_CONFIG_SLEEP_Pos);   //Configure Watchdog. a) Pause watchdog while the CPU is halted by the debugger.  b) Keep the watchdog running while the CPU is sleeping.
    NRF_WDT->CRV = 3*32768;             //ca 3 sek. timout
    NRF_WDT->RREN |= WDT_RREN_RR0_Msk;  //Enable reload register 0
    NRF_WDT->TASKS_START = 1;           //Start the Watchdog timer
*/
}

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

void twi_tx(void) //uint8_t* data, uint16_t length
{
    /*
    nrf_drv_twi_tx(nrf_drv_twi_t const * p_instance,
                          uint8_t               address,
                          uint8_t const *       p_data,
                          uint8_t               length,
                          bool                  no_stop);
    */
  
     char test_data[] = {'t','h','i','s',' ','i','s',' ','a',' ','t','e','s','t'};
     nrf_drv_twi_tx(&m_twi, 0x10, test_data, 14, false);   
}

void flashblaster_init(void)
{
    // bool erase_bonds;
    // Initialize.

    power_clock_init();
    log_init();
    timers_init();
    //buttons_leds_init(&erase_bonds);
    power_management_init();

    #if FIRST_BOOT == false // defined in system.h
    watchdog_init();
    #endif 

    ble_stack_init();
    scheduler_init(); 
    gap_params_init();
    gatt_init();
    services_init();
    advertising_init();
    conn_params_init();
    advertising_start();

    //usb_init(); // not needed until usb data needed, should test before next rev 
    usb_pwr_init();
    button_init();
    
    twi_init();
    spi_init(); //SPI in blocking mode(no handler inited), may cause issues with BLE later
    qspi_init();
    
    oled_init(); 
    system_init();
    list_init();
    draw_initial_screen();
    battery_init();
    
}


void hibernate(void)
{
    //nrf_delay_ms(500);
    clear_leds();
    nrf_gpio_pin_clear(RST_PIN); // turn off display
    nrf_gpio_pin_clear(FET_PIN);
    nrf_delay_ms(1000); // delay to avoid reboot after turn off
    nrf_gpio_cfg_sense_input(BTN_ENTER, NRF_GPIO_PIN_PULLUP, NRF_GPIO_PIN_SENSE_LOW);
    //NRF_WDT->TASKS_START = 0; // deactivate watchdog if needed
    NRF_POWER->SYSTEMOFF = 1;
}


int main(void)
{   
    gpio_init();
    nrf_power_dcdcen_set(true);
    if(!nrf_gpio_pin_read(BTN_ENTER)) // && !nrf_gpio_pin_read(BTN_UP)
    {
        flashblaster_init();
    }
    else
    {
        hibernate(); 
    }

    //TODO: should be able to render strings based on presence of data in flash, should not be initing entire filesystem in RAM
    // only store one file hierarchy in RAM, render and pop fucntions, set_current_project(), set_current_chip() , set_current_file()
    // file directory section in flash which is read at boot and can keep tracka of all current projects and their dependencies
    
    // SWD bitbang protocol ref: black magic probe github, and silicon labs swd app note
    // either APP-side or Device-side controls
    // BT5 file transfer from phone app, DFU firmware updates 
    // maximize utiliy of display, maximize ergonoics, a developer and production line tool 

    // Enter main loop.

    for (;;)
    {   
        idle_state_handle();
    }
}


/**
 * @}
 */
