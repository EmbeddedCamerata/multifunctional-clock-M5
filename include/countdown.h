#ifndef __COUNTDOWN_H__
#define __COUNTDOWN_H__

#include <stdint.h>
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
    void Stop(bool IsShutdown = false);

    bool inline isWorking() {return this->IsWorking;};

    private:
    bool IsWorking;
    bool NeedRefresh;
    uint8_t mins, secs;
    
    hw_timer_t *countdown_timer = NULL;
};

extern Countdown user_countdown;

#endif /* __COUNTDOWN_H__ */