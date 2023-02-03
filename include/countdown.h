#ifndef __COUNTDOWN_H__
#define __COUNTDOWN_H__

#include "user_system.h"

#define COUNTDOWN_DEFAULT_MIN 0x0
#define COUNTDOWN_DEFAULT_SEC 0x0

class Countdown
{
public:
    Countdown();

    void Init(System_TypeDef *SysAttr);
    void Begin(uint8_t mins, uint8_t secs);
    void Update();
    void SetCoundown();
    void Stop(bool IsShutdown = false);

    void OnMyPage();
    void Leave();

    inline bool IsOnMyPage() { return this->isOnMyPage; };
    inline bool IsWorking() { return this->isWorking; };

    uint8_t set_min, set_sec;

private:
    void SetMinuteUpdate();
    void StaticDisplay(uint8_t mins, uint8_t secs);
    void UpdateDisplay();

    bool isWorking;
    bool isOnMyPage;
    uint8_t cur_min, cur_sec;
};

extern Countdown user_countdown;

#endif /* __COUNTDOWN_H__ */