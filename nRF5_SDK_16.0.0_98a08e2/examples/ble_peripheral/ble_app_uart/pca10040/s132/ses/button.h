#ifndef BUTTON_H_
#define BUTTON_H_

#include <stdint.h>
#include <string.h>
#include "nordic_common.h"
#include "nrf.h"
#include "app_button.h"
#include "nrf_gpio.h"


#define BTN_UP 5  
#define BTN_DOWN 7 
#define BTN_ENTER 11 
#define BUTTON_COUNT 3
#define BUTTON_DEBOUNCE_DELAY 10

void button_init(void);

void get_button_state(void);




#endif /* BUTTON_H_ */ 
