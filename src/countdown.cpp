#include "../include/utils.h"
#include <driver/timer.h>

extern TFT_eSprite Disbuff;
hw_timer_t *timer1s(uint8_t num, void (*fn)(void), bool autoreload);

Countdown::Countdown() : IsWorking(false),  \
    mins(COUNTDOWN_DEFAULT_MIN),            \
    secs(COUNTDOWN_DEFAULT_SEC) {}

void Countdown::Begin(uint8_t mins, uint8_t secs)
{
    this->IsWorking = true;
    this->mins = mins;
    this->secs = secs;

    if (!this->countdown_timer) {
        /* Create & start */
        this->countdown_timer = timer1s(0, led_heartbeat, true);   // Using Timer 0
#ifdef DEBUG_MODE
        if (!this->countdown_timer) {
            Serial.println("Starting countdown timer error!");
        }
#endif
    }
    else {
        timerRestart(this->countdown_timer);
        timerStart(this->countdown_timer);
    }
}

/*
    @brief 
        Display countdown when setting
*/
void Countdown::StaticDisplay(uint8_t mins, uint8_t secs)
{
    String _time = "%02d:%02d";

    Disbuff.setTextSize(4);
    Disbuff.setCursor(Disbuff.width()/2 - Disbuff.textWidth("99:99")/2, Disbuff.height()/2 - Disbuff.fontHeight()/2);
    Disbuff.setTextColor(TFT_RED);

    Disbuff.printf(_time.c_str(), mins, secs);

    Disbuff.pushSprite(0, 0);
}

void Countdown::PageChangeDisplay()
{
    if (this->IsWorking) {
        this->StaticDisplay(this->mins, this->secs);
    }
    else {
        this->StaticDisplay(COUNTDOWN_DEFAULT_MIN, COUNTDOWN_DEFAULT_SEC);
    }
}

void Countdown::Update()
{
    this->mins--;
    this->secs--;

    this->StaticDisplay(mins, secs);

    if (this->mins == 0 and this->secs == 0) {
        this->Stop();
    }
}

void Countdown::Stop(bool IsShutdown)
{
    this->IsWorking = false;

    /* Show "Time up" */
    Disbuff.setCursor(10, 10);
    Disbuff.setTextSize(2);
    Disbuff.fillRect(10, 10, Disbuff.height(), Disbuff.fontHeight(), TFT_BLACK);
    Disbuff.setTextColor(TFT_WHITE);

    if (IsShutdown) {
        Disbuff.printf("Reset");
    }
    else {
        Disbuff.printf("Time up!");
    }

    Disbuff.pushSprite(0, 0);

    timerStop(this->countdown_timer);
}

hw_timer_t *timer1s(uint8_t num, void (*fn)(void), bool autoreload)
{
    hw_timer_t* timer = timerBegin(num, (TIMER_BASE_CLK / 1000000), true);
    timerStop(timer);
    timerAttachInterrupt(timer, fn, false);
    timerAlarmWrite(timer, 1000000, autoreload);
    timerAlarmEnable(timer); 
    timerRestart(timer);
    timerStart(timer);

    return timer;
}

Countdown user_countdown;