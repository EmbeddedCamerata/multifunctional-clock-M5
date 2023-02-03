#ifndef _BSP_TIMER_H_
#define _BSP_TIMER_H_

#include "esp32-hal-timer.h"

hw_timer_t *timer1s(uint8_t num, void (*fn)(void), bool autoreload = false);
hw_timer_t *timer1m(uint8_t num, void (*fn)(void), bool autoreload = false);

#endif