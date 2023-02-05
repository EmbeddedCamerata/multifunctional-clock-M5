#ifndef __NTP_CLOCK_H__
#define __NTP_CLOCK_H__

#include "user_system.h"
#include <time.h>

#define NTP_CALIBRATION_INTERVAL    (60) // Minutes

class NTPClock
{
public:
    NTPClock(bool DateOnStartup = false);

    void Init(System_TypeDef *SysAttr);

    void LocalTimeUpdate();
    void ButtonsUpdate();

    void DisplayFromNTP(struct tm *TimeInfo, TickType_t Tick = portMAX_DELAY);
    void DisplayFromRTC(bool ChimeEnable, TickType_t Tick = portMAX_DELAY);

    void OnMyPage();
    void Leave();

    void ErrorHook();

    inline bool IsOnMyPage() { return this->isOnMyPage; };
    inline bool IsDisplayingDate() { return this->isDisplayingDate; };

private:
    bool SyncLocalTime(struct tm *TimeInfo);
    void SetRTC(struct tm *TimeInfo);
    void TimeDisplay(int hour, int minute);
    void DateDisplay(uint8_t Month, uint8_t Weekday, uint8_t Date);

private:
    bool isInited;
    bool isOnMyPage;
    bool isDisplayingDate;
    time_t LastSyncTime;
};

extern NTPClock User_NTPClock;

#endif /* __NTP_CLOCK_H__ */