/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */

#include "driver_examples.h"
#include "driver_init.h"
#include "utils.h"
#include <hal_i2c_m_sync.h>
#include <hpl_i2c_m_sync.h>
#include <utils.h>
#include <utils_assert.h>
#include <stdint.h>
#include <hpl_gclk_config.h>
#include <hpl_gclk_base.h>

static struct timer_task TIMER_0_task1, TIMER_0_task2;
/**
 * Example of using TIMER_0.
 */
static void TIMER_0_task1_cb(const struct timer_task *const timer_task)
{
}

static void TIMER_0_task2_cb(const struct timer_task *const timer_task)
{
}

void TIMER_0_example(void)
{
	TIMER_0_task1.interval = 100;
	TIMER_0_task1.cb       = TIMER_0_task1_cb;
	TIMER_0_task1.mode     = TIMER_TASK_REPEAT;
	TIMER_0_task2.interval = 200;
	TIMER_0_task2.cb       = TIMER_0_task2_cb;
	TIMER_0_task2.mode     = TIMER_TASK_REPEAT;

	timer_add_task(&TIMER_0, &TIMER_0_task1);
	timer_add_task(&TIMER_0, &TIMER_0_task2);
	timer_start(&TIMER_0);
}

void I2C_Write(uint8_t periph, uint16_t addr, uint8_t* ptrBuffer, uint16_t length) // third param was int ptrBuffer
{
	if(!periph){
		struct io_descriptor *I2C_0_io;
		
		i2c_m_sync_get_io_descriptor(&I2C_0, &I2C_0_io);
		i2c_m_sync_enable(&I2C_0);
		i2c_m_sync_set_slaveaddr(&I2C_0, addr, I2C_M_SEVEN);
		
		io_write(I2C_0_io, (uint8_t *)ptrBuffer, length);
	}
	
// 	else if(periph == 1){
// 		struct io_descriptor *I2C_1_io;
// 
// 		i2c_m_sync_get_io_descriptor(&I2C_1, &I2C_1_io);
// 		i2c_m_sync_enable(&I2C_1);
// 		i2c_m_sync_set_slaveaddr(&I2C_1, addr, I2C_M_SEVEN);
// 		
// 		io_write(I2C_1_io, (uint8_t *)ptrBuffer, length);
// 	}
}


void I2C_Read(uint8_t periph, uint16_t addr, uint8_t* ptrBuffer, uint16_t length) // third param was int ptrBuffer
{
	if(!periph){
		struct io_descriptor *I2C_0_io;
		
		i2c_m_sync_get_io_descriptor(&I2C_0, &I2C_0_io);
		i2c_m_sync_enable(&I2C_0);
		i2c_m_sync_set_slaveaddr(&I2C_0, addr, I2C_M_SEVEN);
		
		io_read(I2C_0_io, (uint8_t *)ptrBuffer, length);
	}
	
// 	else if(periph == 1){
// 		struct io_descriptor *I2C_1_io;
// 
// 		i2c_m_sync_get_io_descriptor(&I2C_1, &I2C_1_io);
// 		i2c_m_sync_enable(&I2C_1);
// 		i2c_m_sync_set_slaveaddr(&I2C_1, addr, I2C_M_SEVEN);
// 		
// 		io_read(I2C_1_io, (uint8_t *)ptrBuffer, length);
// 	}
	
}


void USART0_TX(uint8_t* ptrBuffer, uint16_t length) // first param was int ptrBuffer
{
	struct io_descriptor *io;
	
	usart_sync_get_io_descriptor(&USART_0, &io);
	usart_sync_enable(&USART_0);

	io_write(io, (uint8_t *)ptrBuffer, length);

}


void USART0_RX(uint8_t* ptrBuffer, uint16_t length) // first param was int ptrBuffer
{
	struct io_descriptor *io;
	
	usart_sync_get_io_descriptor(&USART_0, &io);
	usart_sync_enable(&USART_0);

	io_read(io, (uint8_t *)ptrBuffer, length);
	
}
