#include "../include/utils.h"
#include <time.h>

void SetRTC(struct tm *TimeInfo);
void NTPTimeTask(void *arg);

void SetRTC(struct tm *TimeInfo)
{
    RTC_TimeTypeDef TimeStruct;
    TimeStruct.Hours   = TimeInfo->tm_hour;  // Set the time.
    TimeStruct.Minutes = TimeInfo->tm_min;
    TimeStruct.Seconds = TimeInfo->tm_sec;
    M5.Rtc.SetTime(&TimeStruct);  // writes the set time to the real time clock.

    RTC_DateTypeDef DateStruct;
    DateStruct.WeekDay = TimeInfo->tm_wday;  // Set the date.
    DateStruct.Month   = TimeInfo->tm_mon;
    DateStruct.Date    = TimeInfo->tm_mday;
    DateStruct.Year    = TimeInfo->tm_year;
    M5.Rtc.SetData(&DateStruct);  // writes the set date to the real time clock.
}

void UpdateLocalTime()
{
    struct tm timeinfo;
    if (!getLocalTime(&timeinfo)) {  // Return 1 when the time is successfully obtained.
        Serial.println("Failed to obtain time");
        return;
    }
    else {
		SetRTC(&timeinfo);
#ifdef DEBUG_MODE
        Serial.println(&timeinfo, "%A, %B %d \n%Y %H:%M:%S");  // Screen prints date and time.
#endif
    }
}