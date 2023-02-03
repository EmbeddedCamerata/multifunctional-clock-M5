#include "../include/utils.h"
#include <driver/timer.h>

hw_timer_t *timer1s(uint8_t num, void (*fn)(void), bool autoreload)
{
    hw_timer_t *timer = timerBegin(num, (TIMER_BASE_CLK / 1000000), true);
    timerStop(timer);
    timerAttachInterrupt(timer, fn, false);
    timerAlarmWrite(timer, 1000000, autoreload);
    timerAlarmEnable(timer); 
    timerRestart(timer);
    timerStart(timer);

    return timer;
}

hw_timer_t *timer1m(uint8_t num, void (*fn)(void), bool autoreload)
{
    hw_timer_t *timer = timerBegin(num, (TIMER_BASE_CLK / 1000000), true);
    timerStop(timer);
    timerAttachInterrupt(timer, fn, false);
    timerAlarmWrite(timer, 60000000, autoreload);
    timerAlarmEnable(timer); 
    timerRestart(timer);
    timerStart(timer);

    return timer;
}