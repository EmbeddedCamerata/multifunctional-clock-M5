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

void countdown_update(void *arg)
{
    TickType_t last_tick = xTaskGetTickCount();

    while (1) {
        xTaskDelayUntil(&last_tick, 1000 / portTICK_RATE_MS);
        digitalWrite(M5_LED, 1 - digitalRead(M5_LED));
        user_countdown.Update();
    }
}