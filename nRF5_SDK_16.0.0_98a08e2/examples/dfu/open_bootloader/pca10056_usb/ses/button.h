#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_button.h"
#include "nrf_gpio.h"




#define BUTTON_COUNT 3
#define BUTTON_DEBOUNCE_DELAY 30

#define LONG_PRESS_THRESHOLD 500



void button_init(void);

void get_button_state(void);

void timer_start(void);
void timer_stop(void); 
void timer_init(void); 

void hibernate(void);

void device_shutdown(void);

#endif /* BUTTON_H_ */ 
