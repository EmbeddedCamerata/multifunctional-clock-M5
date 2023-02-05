#include "../include/utils.h"
#include <WiFi.h>

extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;
const char *ntpServer = "time1.aliyun.com";
const long gmtOffset_sec = 8 * 3600;
const int daylightOffset_sec = 3600;
const char *Weekdays[7] = {
    "Sun", "Mon", "Tues", "Wed", "Thu", "Wed", "Sat"
};
const char *Months[12] = {
    "Jan", "Feb", "Mar", "Apr", "May", "Jun",
    "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
};

TaskHandle_t xhandle_clock_display = NULL;

void ClockDisplayTask(void *arg);
void ClockRegularUpdateTask(void *arg);

NTPClock::NTPClock(bool DateOnStartup) :   \
    isInited(0),                           \
    isOnMyPage(0),                         \
    isDisplayingDate(DateOnStartup) {}

void NTPClock::Init(System_TypeDef *SysAttr)
{
    if (SysAttr->SysPage == PAGE_CLOCK) {
        Disbuff.createSprite(TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT);
        this->OnMyPage();
    }

    this->isInited = true;
}

void NTPClock::LocalTimeUpdate()
{
    struct tm timeinfo;
    
    if (!this->SyncLocalTime(&timeinfo)) {
        return;
    }
    
    if (this->isOnMyPage) {
        this->DisplayFromNTP(&timeinfo, (TickType_t)10);
    }
}

void NTPClock::ButtonsUpdate()
{
    if (M5.BtnA.wasReleased()) {
        /* Short press of BtnA for update NTP time immediately */
        this->LocalTimeUpdate();
    }
    if (M5.BtnB.wasReleased()) {
        
    }
}

void NTPClock::OnMyPage()
{
    int _try_time = 0;
    struct tm timeinfo;

    this->isOnMyPage = true;
    M5.Lcd.setRotation(PAGE_CLOCK);

    if (Disbuff.width() != TFT_LANDSCAPE_WIDTH) {
        Disbuff.deleteSprite();
		Disbuff.createSprite(TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT);
    }

    Disbuff.fillRect(0, 0, TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT, TFT_BLACK);

    if (!this->isInited) {
        configTime(gmtOffset_sec, daylightOffset_sec, ntpServer);  // init and get the time.
        
        if (!this->SyncLocalTime(&timeinfo)) {
            do {
                _try_time++;
#ifdef DEBUG_MODE
                Serial.printf("Retries: %d/%d\n", _try_time, NTP_UPDATE_RETRY_TIMES);
#endif
				if (_try_time == NTP_UPDATE_RETRY_TIMES) {
					// TODO Error handle here
					return;
				}
            } while (!this->SyncLocalTime(&timeinfo));
        }

        WiFi.setSleep(true);
        // WiFi.disconnect(true);
        // WiFi.mode(WIFI_OFF);

        xTaskCreate(ClockDisplayTask, "ClockDisplayTask", \
            1024*2, (void*)0, 6, &xhandle_clock_display);

        this->DisplayFromNTP(&timeinfo);
    }
    else {
        this->DisplayFromRTC(false, (TickType_t)10);
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
bool NTPClock::SyncLocalTime(struct tm *TimeInfo)
{
    if (!getLocalTime(TimeInfo)) {  // Return 1 when the time is successfully obtained.
        Serial.println("Failed to obtain time");
        return false;
    }

    SetRTC(TimeInfo);
    this->LastSyncTime = mktime(TimeInfo);
    
#ifdef DEBUG_MODE
    Serial.println(TimeInfo, "%A, %B %d \n%Y %H:%M:%S");  // Screen prints date and time.
#endif

    return true;
}

void NTPClock::DisplayFromNTP(struct tm *TimeInfo, TickType_t Tick)
{
    xSemaphoreTake(lcd_draw_sem, Tick);
    
    this->TimeDisplay(TimeInfo->tm_hour, TimeInfo->tm_min);
    if (this->isDisplayingDate) {
        this->DateDisplay(TimeInfo->tm_mon, TimeInfo->tm_wday, TimeInfo->tm_mday);
    }
	
    xSemaphoreGive(lcd_draw_sem);
}

void NTPClock::DisplayFromRTC(bool ChimeEnable, TickType_t Tick)
{
    RTC_DateTypeDef DateStruct;
	RTC_TimeTypeDef TimeStruct;

    M5.Rtc.GetTime(&TimeStruct);

	xSemaphoreTake(lcd_draw_sem, Tick);
    
    if (ChimeEnable) {
        if (TimeStruct.Seconds == 0) {
            this->TimeDisplay(TimeStruct.Hours, TimeStruct.Minutes);
        }
    }
    else {
        this->TimeDisplay(TimeStruct.Hours, TimeStruct.Minutes);
    }

    if (this->isDisplayingDate) {
        M5.Rtc.GetData(&DateStruct);
        this->DateDisplay(DateStruct.Month, DateStruct.WeekDay, DateStruct.Date);
    }
	
    xSemaphoreGive(lcd_draw_sem);
}

/*
    @brief
        Display time & date(NTPCLOCK_DISPLAY_DATE).
        TODO AM/PM
*/
void NTPClock::TimeDisplay(int hour, int minute)
{
    Disbuff.setTextSize(4);
    Disbuff.fillRect(
        TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2,   \
        TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2,        \
        Disbuff.textWidth("99:99"), Disbuff.fontHeight(),       \
        TFT_BLACK
    );
	
	Disbuff.setCursor(
        TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2,   \
		TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2
    );
	
    Disbuff.setTextColor(TFT_RED);
	Disbuff.printf("%02d:%02d", hour, minute);
	Disbuff.pushSprite(0, 0);
}

void NTPClock::DateDisplay(uint8_t Month, uint8_t Weekday, uint8_t Date)
{
    Disbuff.setTextSize(2);
    Disbuff.fillRect(10, 10, Disbuff.textWidth("Jan. 12 / Tues"), \
        Disbuff.fontHeight(), TFT_BLACK
    );
    Disbuff.setCursor(10, 10);
    Disbuff.setTextColor(TFT_WHITE);
    Disbuff.printf("%s. %d / %s", Months[Month], Date, Weekdays[Weekday]);
    Disbuff.pushSprite(0, 0);
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
    int _interval = 0;

    while (1) {
        vTaskDelay(1000 / portTICK_RATE_MS);
        _interval++;

        if (_interval == 60 * NTP_CALIBRATION_INTERVAL) {
            _interval = 0;
            User_NTPClock.LocalTimeUpdate();
        }
        
        if (User_NTPClock.IsOnMyPage()) {
            User_NTPClock.DisplayFromRTC(true, portMAX_DELAY);
        }
    }
}

NTPClock User_NTPClock(true);