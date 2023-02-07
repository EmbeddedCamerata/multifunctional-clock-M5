#ifndef __NTP_CLOCK_H__
#define __NTP_CLOCK_H__

#include "user_system.h"
#include <time.h>

class NTPClock
{
public:
    NTPClock(bool DateOnStartup = false);

    void Init(SysPage_e Page);

    void LocalTimeUpdate();
    void ButtonsUpdate();

    void DisplayFromNTP(struct tm *TimeInfo, TickType_t Tick = portMAX_DELAY);
    void DisplayFromRTC(bool ChimeEnable, TickType_t Tick = portMAX_DELAY);

    void OnMyPage();
    void Leave();

    void ErrorHook();

    inline void Inited() { this->isInited = true; };
    inline bool IsOnMyPage() { return this->isOnMyPage; };
    inline bool IsDisplayingDate() { return this->isDisplayingDate; };

private:
    bool SyncLocalTime(struct tm *TimeInfo);
    void TFTRecreate();
    void SetRTC(struct tm *TimeInfo);
    void TimeDisplay(int hour, int minute);
    void DateDisplay(uint8_t Month, uint8_t Weekday, uint8_t Date);

private:
    bool isInited;
    bool isOnMyPage;
    bool isDisplayingDate;
    time_t LastSyncTime;

    const char *ntpServer = "time1.aliyun.com";
    const long gmtOffset_sec = 8 * 3600;
    const int daylightOffset_sec = 3600;
    const char *Weekdays[7] = {"Sun", "Mon", "Tues", "Wed", "Thu", "Wed", "Sat"};
    const char *Months[12] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
};

void NTPClockInitTask(void *arg);

extern NTPClock User_NTPClock;

#endif /* __NTP_CLOCK_H__ */