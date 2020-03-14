#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_button.h"
#include "nrf_gpio.h"


#define BTN_UP 8  
#define BTN_DOWN 34 // P1.02
#define BTN_ENTER 5

#define BUTTON_COUNT 3
#define BUTTON_DEBOUNCE_DELAY 30

#define LONG_PRESS_THRESHOLD 1000


void button_init(void);

void get_button_state(void);

void timer_start(void);
void timer_stop(void); 
void timer_init(void); 

void hibernate(void);

#endif /* BUTTON_H_ */ 
