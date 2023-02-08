#ifndef __COUNTDOWN_H__
#define __COUNTDOWN_H__

#include "user_system.h"

class Countdown
{
public:
    Countdown();

    void Init(SysPage_e Page);
    void Begin(uint8_t mins, uint8_t secs);
    void Pause();
    void Resume();
    void Stop(bool isReset = false);

    void OnMyPage();
    void Leave();

    void CountdownUpdate();
    void ButtonsUpdate();

    inline bool IsOnMyPage() { return this->isOnMyPage; };
    inline bool IsStopped() { return (not this->isWorking) and (not this->isActivated); };
    inline bool IsWorking() { return this->isWorking; };

    uint8_t set_min, set_sec;

private:
    void SetCoundown();

    void TFTRecreate();
    void StaticDisplay(uint8_t mins, uint8_t secs);
    void UpdateDisplay();
    void StatusPromptDisplay(const char* StrToShow);

    bool isActivated;   // Whether it starts counting down
    bool isWorking;     // Whether it is counting down
    bool isOnMyPage;
    uint8_t cur_min, cur_sec;
};

void CountdownInitTask(void *arg);

extern Countdown User_Countdown;

#endif /* __COUNTDOWN_H__ */