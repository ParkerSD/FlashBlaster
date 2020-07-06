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


#define SAMPLES_IN_BUFFER 10
volatile uint8_t state = 1;

static const nrf_drv_timer_t m_timer = NRF_DRV_TIMER_INSTANCE(1); //timer0 used for softdevice
static nrf_saadc_value_t     m_buffer_pool[2][SAMPLES_IN_BUFFER];
static nrf_ppi_channel_t     m_ppi_channel;
static uint32_t              m_adc_evt_counter;

static nrf_saadc_value_t adc_buffer[SAMPLES_IN_BUFFER];
static uint16_t sum;
static uint16_t avg;
static uint32_t adc_speed; 
static bool reinit_timer = false; 

static battery_struct* battery; 




void timer_handler(nrf_timer_event_t event_type, void * p_context)
{
    // average samples and update battery icon ? 
}


void adc_average(void)
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
    uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, adc_speed);
    nrf_drv_timer_extended_compare(&m_timer,
                                   NRF_TIMER_CC_CHANNEL0,
                                   ticks,
                                   NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                   false);
    nrf_drv_timer_enable(&m_timer);

    uint32_t timer_compare_event_addr = nrf_drv_timer_compare_event_address_get(&m_timer,
                                                                                NRF_TIMER_CC_CHANNEL0);
    uint32_t saadc_sample_task_addr = nrf_drv_saadc_sample_task_get();

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

        adc_average();
        battery_draw_icon();

        if(!reinit_timer) //reinit timer once 
        {
            adc_speed = 500; 
            nrf_drv_timer_disable(&m_timer);
            uint32_t ticks = nrf_drv_timer_ms_to_ticks(&m_timer, adc_speed);
            nrf_drv_timer_extended_compare(&m_timer,
                                           NRF_TIMER_CC_CHANNEL0,
                                           ticks,
                                           NRF_TIMER_SHORT_COMPARE0_CLEAR_MASK,
                                           false);
            nrf_drv_timer_enable(&m_timer);
            reinit_timer = true; 
        }
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


void battery_draw_icon(void) //charge bar is 5x9
{   
    uint16_t battery_val = avg; 

    uint8_t bars; 

    if(!battery->charging_state)
    {
        battery_draw_charging(COLOR_BLACK);
    }

    if(battery_val >= HALF_CHARGE)
    {   
        if(battery_val <= FIVE_BAR_MAX_VOLT)
        {
            bars = 5; 
        }
        else if(battery_val <= SIX_BAR_MAX_VOLT)
        {
            bars = 6; 
        }
        else if(battery_val <= SEVEN_BAR_MAX_VOLT)
        {
            bars = 7; 
        }
        else if(battery_val <= EIGHT_BAR_MAX_VOLT)
        {
            bars = 8; 
        }
        else if(battery_val <= FULL_CHARGE)
        {
            bars = 9; 
        }
        else if(battery_val > FULL_CHARGE)
        {
            bars = 9; 
        }
        battery_draw_outline(COLOR_GREEN);
        SSD1351_draw_filled_rect(112, 12, 10, 4, COLOR_BLACK); // erase 
        SSD1351_draw_filled_rect(112, 12, bars, 4, COLOR_GREEN);
    }
    else if(battery_val < HALF_CHARGE && battery_val >= LOW_CHARGE)
    {   
        if(battery_val <= THREE_BAR_MAX_VOLT)
        {
            bars = 3; 
        }
        else if(battery_val < HALF_CHARGE)
        {
            bars = 4; 
        }
        battery_draw_outline(COLOR_YELLOW);
        SSD1351_draw_filled_rect(112, 12, 10, 4, COLOR_BLACK); // erase 
        SSD1351_draw_filled_rect(112, 12, bars, 4, COLOR_YELLOW);
    }
    else if(battery_val < LOW_CHARGE && battery_val >= NO_CHARGE)
    {   
        if(battery_val <= ONE_BAR_MAX_VOLT)
        {
            bars = 1; 
        }
        else if(battery_val < LOW_CHARGE)
        {
            bars = 2; 
        }
        battery_draw_outline(COLOR_RED);
        SSD1351_draw_filled_rect(112, 12, 10, 4, COLOR_BLACK); // erase 
        SSD1351_draw_filled_rect(112, 12, bars, 4, COLOR_RED);
    }
    else if(battery_val < NO_CHARGE)
    {   
        //NOTE Commented out for testing 
        // hibernate(); //shutdown amp if low voltage condition, plug in USB to reboot
        
        //for test 
        battery_draw_outline(COLOR_AQUA);
        SSD1351_draw_filled_rect(112, 12, 10, 4, COLOR_BLACK); // erase 
        SSD1351_draw_filled_rect(112, 12, 9, 4, COLOR_AQUA);
    }


    if(battery->charging_state)
    {
        battery_draw_charging(COLOR_YELLOW);
    }
    
    SSD1351_update();
}


void battery_set_charging_state(bool state)
{
    battery->charging_state = state; 
}


void battery_draw_charging(uint16_t color) //vertical yellow lightning bolt
{
    //SSD1351_draw_line(x0, y0, x1, y1, uint16_t color)
//NOTE adjacent 
//    SSD1351_draw_line( 105, 11, 108, 14, color);
//    SSD1351_draw_line( 108, 14, 105, 14, color);
//    SSD1351_draw_line( 105, 14, 107, 17, color);
//    SSD1351_draw_line( 105, 11, 107, 14, color);
//    SSD1351_draw_line( 107, 14, 105, 14, color);
//    SSD1351_draw_line( 104, 14, 107, 17, color);

//NOTE super imposed 
    SSD1351_draw_line( 115, 11, 118, 14, color);
    SSD1351_draw_line( 118, 14, 115, 14, color);
    SSD1351_draw_line( 115, 14, 117, 17, color);
    SSD1351_draw_line( 115, 11, 117, 14, color);
    SSD1351_draw_line( 117, 14, 115, 14, color);
    SSD1351_draw_line( 114, 14, 117, 17, color);

    SSD1351_draw_line( 113, 14, 117, 18, COLOR_BLACK);
    SSD1351_draw_line( 115, 12, 115, 13, COLOR_BLACK);
    //SSD1351_draw_line( 119, 14, 115, 10, COLOR_BLACK); //note used: top right black line
    SSD1351_draw_line( 117, 15, 117, 16, COLOR_BLACK);
}

void battery_draw_outline(uint16_t color)
{
    //SSD1351_draw_rect(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
    SSD1351_draw_rect(110, 10, 12, 7, color);
    SSD1351_draw_rect(123, 12, 0, 3, color); //battery end
}

void adc_init(void)
{
    saadc_init();
    adc_speed = 1; // quick sample on boot 
    saadc_sampling_event_init();
    saadc_sampling_event_enable();    
}

void battery_init(void)
{
    battery = malloc(sizeof(battery_struct)); 
    
    if(nrf_gpio_pin_read(LDO_EN)) // read ldo_en pin to determine if usb is connected on boot 
    {
        battery_set_charging_state(true);
    }
    else
    {
         battery_set_charging_state(false); 
    }

    adc_init();
}