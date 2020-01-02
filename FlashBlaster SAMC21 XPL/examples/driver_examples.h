/*
 * Code generated from Atmel Start.
 *
 * This file will be overwritten when reconfiguring your Atmel Start project.
 * Please copy examples or other code you want to keep to a separate file
 * to avoid losing it when reconfiguring.
 */
#ifndef DRIVER_EXAMPLES_H_INCLUDED
#define DRIVER_EXAMPLES_H_INCLUDED

#include <hal_i2c_m_sync.h>
#include <hpl_i2c_m_sync.h>
#include <utils.h>
#include <utils_assert.h>
#include <stdint.h>
#include <hpl_gclk_config.h>
#include <hpl_gclk_base.h>

#ifdef __cplusplus
extern "C" {
#endif

void TIMER_0_example(void);

void I2C_Write(uint8_t periph, uint16_t addr, uint8_t* ptrBuffer, uint16_t length); // (address 1, pointer to buffer, length in bytes)
void I2C_Read(uint8_t periph, uint16_t addr, uint8_t* ptrBuffer, uint16_t length);

void USART0_TX(uint8_t* ptrBuffer, uint16_t length);
void USART0_RX(uint8_t* ptrBuffer, uint16_t length);

#ifdef __cplusplus
}
#endif
#endif // DRIVER_EXAMPLES_H_INCLUDED
