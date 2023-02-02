#ifndef __COUNTDOWN_H__
#define __COUNTDOWN_H__

#include "esp32-hal-timer.h"

#define COUNTDOWN_DEFAULT_MIN 0x0
#define COUNTDOWN_DEFAULT_SEC 0x0

class Countdown {
    public:
    Countdown();

    void Begin(uint8_t mins, uint8_t secs);
    void StaticDisplay(uint8_t mins, uint8_t secs);
    void PageChangeDisplay();
    void Update();
    void SetCoundown();
    void Stop(bool IsShutdown = false);

    bool inline IsWorking() {return this->isWorking;};

    uint8_t set_min, set_sec;

    private:
    void SetMinuteUpdate();

    bool isWorking;
    bool needRefresh;
    uint8_t cur_min, cur_sec;
    
    hw_timer_t *countdown_timer = NULL;
};

extern Countdown user_countdown;

#endif /* __COUNTDOWN_H__ */