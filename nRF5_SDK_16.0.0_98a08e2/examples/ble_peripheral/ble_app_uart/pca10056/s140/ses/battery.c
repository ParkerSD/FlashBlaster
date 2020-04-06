#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "nrf_drv_saadc.h"
#include "nrf_drv_ppi.h"
#include "nrf_drv_timer.h"
#include "boards.h"
#include "app_error.h"
#include "nrf_delay.h"
#include "app_util_platform.h"
#include "nrf_pwr_mgmt.h"
#include "oled.h"
#include "app_button.h"
#include "nrf_gpio.h"
#include "nrf_log.h"
#include "nrf_log_ctrl.h"
#include "nrf_log_default_backends.h"
#include "button.h" 
#include "ssd1351.h" 
#include "app_timer.h" 
#include "app_error.h" 
#include "system.h" 
#include "battery.h" 


#define SAMPLES_IN_BUFFER 5
volatile uint8_t state = 1;

static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1); //timer0 used for softdevice
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;

static nrf_saadc_value_t adc_buffer[5];
static uint16_t sum;
static uint16_t avg;

battery_struct* battery; 


void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    // average samples and update battery icon ? 
}


void avg_buffer_samples(void)
{   
    sum = 0; //clear sum 
    for(int i = 0; i < SAMPLES_IN_BUFFER; i++)
    {
        sum += adc_buffer[i]; 
    }
    avg = sum/SAMPLES_IN_BUFFER;
}


void saadc_sampling_event_init(void)
{
    ret_code_t err_code;

    err_code = nrf_drv_ppi_init();
    APP_ERROR_CHECK(err_code);

    nrf_drv_timer_config_t timer_cfg = NRF_DRV_TIMER_DEFAULT_CONFIG;
    timer_cfg.bit_width = NRF_TIMER_BIT_WIDTH_32;
    err_code = nrf_drv_timer_init(&m_timer, &timer_cfg, timer_handler);
    APP_ERROR_CHECK(err_code);

    /* setup m_timer for compare event every 2000ms */
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, 1000);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr   = nrf_drv_saadc_sample_task_get();

    /* setup ppi channel so that timer compare event is triggering sample task in SAADC */
    err_code = nrf_drv_ppi_channel_alloc(&m_ppi_channel);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_ppi_channel_assign(m_ppi_channel,
                                          timer_compare_event_addr,
                                          saadc_sample_task_addr);
    APP_ERROR_CHECK(err_code);
}


void saadc_sampling_event_enable(void)
{
    ret_code_t err_code = nrf_drv_ppi_channel_enable(m_ppi_channel);

    APP_ERROR_CHECK(err_code);
}


void saadc_callback(nrf_drv_saadc_evt_t const * p_event)
{
    if (p_event->type == NRF_DRV_SAADC_EVT_DONE)
    {
        ret_code_t err_code;
        err_code = nrf_drv_saadc_buffer_convert(p_event->data.done.p_buffer, SAMPLES_IN_BUFFER);
        APP_ERROR_CHECK(err_code);

        for (int i = 0; i < SAMPLES_IN_BUFFER; i++)
        {
            adc_buffer[i] = p_event->data.done.p_buffer[i];
        }
        m_adc_evt_counter++;

        avg_buffer_samples();
        battery_draw_percent(avg);

    }
}


void saadc_init(void)
{
    ret_code_t err_code;
    nrf_saadc_channel_config_t channel_config = NRF_DRV_SAADC_DEFAULT_CHANNEL_CONFIG_SE(NRF_SAADC_INPUT_AIN0);

    err_code = nrf_drv_saadc_init(NULL, saadc_callback);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_channel_init(0, &channel_config);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[0], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);

    err_code = nrf_drv_saadc_buffer_convert(m_buffer_pool[1], SAMPLES_IN_BUFFER);
    APP_ERROR_CHECK(err_code);
}


void battery_draw_percent(uint16_t avg) //bar is 5x10, percent is 1-10 / 10%-100% 
{   
    uint8_t percent = avg / 100; //NOTE not linear use Raw ADC value, denominator scale value
    if(percent > 10)
    {
        percent = 10; 
    }   
    //SSD1351_draw_filled_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    if(percent >= 5)
    {   
        battery_draw_outline(COLOR_GREEN);
        SSD1351_draw_filled_rect(112, 12, 10, 4, COLOR_BLACK); // erase 
        SSD1351_draw_filled_rect(112, 12, percent, 4, COLOR_GREEN);
        SSD1351_update();
    }
    else if(percent <= 5 && percent >= 2)
    {   
        battery_draw_outline(COLOR_YELLOW);
        SSD1351_draw_filled_rect(112, 12, 10, 4, COLOR_BLACK); // erase 
        SSD1351_draw_filled_rect(112, 12, percent, 4, COLOR_YELLOW);
        SSD1351_update();
    }
    else if(percent < 2 && percent >= 1)
    {   
        battery_draw_outline(COLOR_RED);
        SSD1351_draw_filled_rect(112, 12, 10, 4, COLOR_BLACK); // erase 
        SSD1351_draw_filled_rect(112, 12, percent, 4, COLOR_RED);
        SSD1351_update();
    }
    else if(percent < 1)
    {
        hibernate(); //shutdown amp if low voltage condition 
    }
}

void battery_draw_charging(void) //yellow lightning bolt
{

}

void battery_draw_outline(uint16_t color)
{
    //SSD1351_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    SSD1351_draw_rect(110, 10, 12, 7, color);
    SSD1351_draw_rect(123, 12, 0, 3, color); //battery end
    SSD1351_update();
}

void adc_init(void)
{
    saadc_init();
    saadc_sampling_event_init();
    saadc_sampling_event_enable();
}

void battery_init(void)
{
    // create battery struct
    // draw battery icon
    // poll ADC
    // compare voltage to table RSOC table
    // draw battery percent 
    battery = malloc(sizeof(battery_struct)); 

    adc_init();

}