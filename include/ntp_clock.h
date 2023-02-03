#ifndef __NTP_CLOCK_H__
#define __NTP_CLOCK_H__

#include "user_system.h"
#include <time.h>

#define NTP_CALIBRE_INTERVAL (30 * 60)

class NTPClock
{
public:
    NTPClock();

    void Init(System_TypeDef *SysAttr);
    bool SyncLocalTime();
    void ClockDisplay(int hour, int minute);
    void Update();
    void UpdateNow();
    void OnMyPage();
    void Leave();

    void ErrorHook();

    inline bool IsOnMyPage() { return this->isOnMyPage; };

public:
    struct tm LastNTPTime;

private:
    void SetRTC(struct tm *TimeInfo);
    void ClockDisplayFromRTC();

private:
    bool isInited;
    bool isOnMyPage;
    struct tm LastSyncNTPTime;
};

extern NTPClock user_NTPclock;

#endif /* __NTP_CLOCK_H__ */