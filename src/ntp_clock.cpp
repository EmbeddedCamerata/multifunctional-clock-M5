#include "../include/utils.h"
#include <WiFi.h>

extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;
const char *ntpServer = "time1.aliyun.com";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 3600;
const char *Weekdays[7] =
    {"Mon", "Tues", "Wed", "Thu", "Wed", "Sat", "Sun"};
const char *Months[12] =
    {"Jan", "Feb", "Mar", "Apr", "May", "Jun",
     "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"};

TaskHandle_t xhandle_clock_display_update = NULL;

void ClockDisplayTask(void *arg);

NTPClock::NTPClock() :  \
    isInited(0),        \
    isOnMyPage(0) {}

void NTPClock::Init(System_TypeDef *SysAttr)
{
    if (SysAttr->SysPage == PAGE_CLOCK) {
        Disbuff.createSprite(TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT);
        this->OnMyPage();
    }

    this->isInited = true;
}

void NTPClock::OnMyPage()
{
    int _try_time = 0;

    this->isOnMyPage = true;
    M5.Lcd.setRotation(PAGE_CLOCK);

    if (Disbuff.width() != TFT_LANDSCAPE_WIDTH) {
        Disbuff.deleteSprite();
		Disbuff.createSprite(TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT);
    }

    Disbuff.fillRect(0, 0, TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT, TFT_BLACK);

    if (!this->isInited) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // init and get the time.
        
        if (!this->SyncLocalTime()) {
            do {
                _try_time++;
#ifdef DEBUG_MODE
                Serial.printf("Retries: %d/%d\n", _try_time, NTP_UPDATE_RETRY_TIMES);
#endif

				if (_try_time == NTP_UPDATE_RETRY_TIMES) {
					// TODO Error handle here
					return;
				}
            } while (!this->SyncLocalTime());
        }

        WiFi.setSleep(true);
        // WiFi.disconnect(true);
        // WiFi.mode(WIFI_OFF);

        xTaskCreate(ClockDisplayTask, "ClockDisplayTask", 1024*2, (void*)0, 6, &xhandle_clock_display_update);

        this->ClockDisplay(this->LastSyncNTPTime.tm_hour, this->LastSyncNTPTime.tm_min);
    }
    else {
        this->ClockDisplayFromRTC();
    }
}

void NTPClock::Leave()
{
    this->isOnMyPage = false;
}

/*
    @brief
        Get NTP time and update LastNTPTime
*/
bool NTPClock::SyncLocalTime()
{
    struct tm timeinfo;

    if (!getLocalTime(&timeinfo)) {  // Return 1 when the time is successfully obtained.
        Serial.println("Failed to obtain time");
        return false;
    }

    SetRTC(&timeinfo);
    this->LastSyncNTPTime = timeinfo;
    
#ifdef DEBUG_MODE
    Serial.println(&timeinfo, "%A, %B %d \n%Y %H:%M:%S");  // Screen prints date and time.
#endif

    return true;
}

void NTPClock::ClockDisplayFromRTC()
{
	String _time = "%02d:%02d";
	RTC_TimeTypeDef TimeStruct;
    
	M5.Rtc.GetTime(&TimeStruct);

	xSemaphoreTake(lcd_draw_sem, (TickType_t)10);
	Disbuff.fillRect(TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2,  \
						TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2,    \
						Disbuff.textWidth("99:99"), Disbuff.fontHeight(),   \
						TFT_BLACK);
	
	Disbuff.setTextSize(4);
	Disbuff.setCursor(TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2, \
						TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2);
	Disbuff.setTextColor(TFT_RED);
	Disbuff.printf(_time.c_str(), TimeStruct.Hours, TimeStruct.Minutes);
	Disbuff.pushSprite(0, 0);

	xSemaphoreGive(lcd_draw_sem);
}

void NTPClock::ClockDisplay(int hour, int minute)
{
	String _time = "%02d:%02d";

	xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
	Disbuff.fillRect(TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2,  \
						TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2,    \
						Disbuff.textWidth("99:99"), Disbuff.fontHeight(),   \
						TFT_BLACK);
	
	Disbuff.setTextSize(4);
	Disbuff.setCursor(TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2, \
						TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2);
	Disbuff.setTextColor(TFT_RED);
	Disbuff.printf(_time.c_str(), hour, minute);
	Disbuff.pushSprite(0, 0);

	xSemaphoreGive(lcd_draw_sem);
}

void NTPClock::UpdateNow()
{
    
    if (!this->SyncLocalTime()) {
        return;
    }
    
    xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
    Disbuff.setTextSize(1);
	Disbuff.fillRect(10, 10, Disbuff.textWidth("OK"), Disbuff.fontHeight(), TFT_BLACK);
	Disbuff.setCursor(10, 10);
	Disbuff.print("OK");
	Disbuff.pushSprite(0, 0);
	xSemaphoreGive(lcd_draw_sem);

	this->ClockDisplay(this->LastSyncNTPTime.tm_hour, this->LastSyncNTPTime.tm_min);
}

void NTPClock::SetRTC(struct tm *TimeInfo)
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

void ClockDisplayTask(void *arg)
{
    String _time = "%02d:%02d";
    struct tm CurTimeInfo;
    RTC_DateTypeDef DateStruct;
    RTC_TimeTypeDef TimeStruct;

    while (1) {
        M5.Rtc.GetTime(&TimeStruct);
        
        if (user_NTPclock.IsOnMyPage() and TimeStruct.Seconds == 0) {
            user_NTPclock.ClockDisplay(TimeStruct.Hours, TimeStruct.Minutes);
#ifdef DEBUG_MODE
			Serial.printf("Now %02d:%02d\n", TimeStruct.Hours, TimeStruct.Minutes);
#endif
        }

        vTaskDelay(1000 / portTICK_RATE_MS);
    }
}

NTPClock user_NTPclock;