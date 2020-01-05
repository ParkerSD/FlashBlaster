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
#define BUTTON_COUNT 2
#define BUTTON_DEBOUNCE_DELAY 25

void button_init(void);


#endif /* BUTTON_H_ */ 
