#include "../include/utils.h"

extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;
TaskHandle_t xhandle_countdown_update = NULL;

Countdown::Countdown() : isWorking(0),  \
    set_min(COUNTDOWN_DEFAULT_MIN),     \
    set_sec(COUNTDOWN_DEFAULT_SEC),     \
    cur_min(COUNTDOWN_DEFAULT_MIN),     \
    cur_sec(COUNTDOWN_DEFAULT_SEC) {}

void Countdown::Begin(uint8_t mins, uint8_t secs)
{
    this->isWorking = true;
    this->set_min = this->cur_min = mins;
    this->set_sec = this->cur_sec = secs;

    xTaskCreate(countdown_update, "countdown_update", 1024, \
        (void*)0, 6, &xhandle_countdown_update);
}

/*
    @brief 
        Display countdown when setting
*/
void Countdown::StaticDisplay(uint8_t mins, uint8_t secs)
{
    String _time = "%02d:%02d";

    Disbuff.fillRect(Disbuff.width()/2 - Disbuff.textWidth("99:99")/2,      \
                        Disbuff.height()/2 - Disbuff.fontHeight()/2,        \
                        Disbuff.textWidth("99:99"), Disbuff.fontHeight(),   \
                        TFT_BLACK);
    
    Disbuff.setTextSize(4);
    Disbuff.setCursor(Disbuff.width()/2 - Disbuff.textWidth("99:99")/2,     \
                        Disbuff.height()/2 - Disbuff.fontHeight()/2);
    Disbuff.setTextColor(TFT_RED);
    Disbuff.printf(_time.c_str(), mins, secs);
}

void Countdown::PageChangeDisplay()
{
    if (this->isWorking) {
        this->StaticDisplay(this->cur_min, this->cur_sec);
    }
    else if (this->set_min != COUNTDOWN_DEFAULT_MIN or \
            this->set_sec != COUNTDOWN_DEFAULT_SEC) {
        this->StaticDisplay(this->set_min, this->set_sec);
    }
    else {
        this->StaticDisplay(COUNTDOWN_DEFAULT_MIN, COUNTDOWN_DEFAULT_SEC);
    }
}

void Countdown::Update()
{
    this->cur_min = (this->cur_sec == 0) ? this->cur_min - 1 : this->cur_min;
    this->cur_sec = (this->cur_sec == 0) ? 59 : this->cur_sec - 1;

#ifdef DEBUG_MODE
    Serial.printf("%02d:%02d\n", this->cur_min, this->cur_sec);
#endif

    xSemaphoreTake(lcd_draw_sem, 0);
    this->StaticDisplay(this->cur_min, this->cur_sec);
    Disbuff.pushSprite(0, 0);
    xSemaphoreGive(lcd_draw_sem);

    if (this->cur_min == 0 and this->cur_sec == 0) {
        this->Stop(false);
    }
}

void Countdown::Stop(bool isShutdown)
{
    this->isWorking = false;
    this->set_min = COUNTDOWN_DEFAULT_MIN;
    this->set_sec = COUNTDOWN_DEFAULT_SEC;
    this->cur_min = COUNTDOWN_DEFAULT_MIN;
    this->cur_sec = COUNTDOWN_DEFAULT_SEC;

    /* Show "Time up" */
    xSemaphoreTake(lcd_draw_sem, (TickType_t)10);
    Disbuff.setCursor(10, 10);
    Disbuff.setTextSize(2);
    Disbuff.fillRect(10, 10, Disbuff.height(), Disbuff.fontHeight(), TFT_BLACK);
    Disbuff.setTextColor(TFT_WHITE);

    if (isShutdown) {
        Disbuff.printf("Reset");
    }
    else {
        Disbuff.printf("Time up!");
    }

    Disbuff.pushSprite(0, 0);
    xSemaphoreGive(lcd_draw_sem);

    if (xhandle_countdown_update != NULL) {
        vTaskDelete(xhandle_countdown_update);
    }
}

void Countdown::SetCoundown()
{
    this->needRefresh = true;

    if (M5.BtnA.wasReleased()) {
        /* Short press of BtnA for +1 second */
        if (this->set_sec == 59) {
            this->set_sec = 0;
            this->SetMinuteUpdate();
        }
        else {
            this->set_sec++;
        }
#ifdef DEBUG_MODE
        Serial.println("Button A released");
#endif
    }
    else if (M5.BtnA.wasReleasefor(500)) {
        /* Long press of BtnA for +10 seconds */
        if (this->set_sec >= 50) {
            this->set_sec = 0;
            this->SetMinuteUpdate();
        }
        else {
            this->set_sec += 10;
        }
#ifdef DEBUG_MODE
        Serial.println("Button A long released");
#endif
    }
    else if (M5.BtnB.wasReleased()) {
        /* Short press of BtnB for +1 minute */
        if (this->set_min == 10) {
            this->set_min = 0;
        }
        else {
            this->set_min++;
        }
    }
    else if (M5.BtnB.wasReleasefor(500)) {
        /* Long press of BtnB for start */
        this->needRefresh = false;
        this->Begin(this->set_min, this->set_sec);
    }
    else {
        this->needRefresh = false;
    }
    
    if (this->needRefresh) {
        xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
        
        this->StaticDisplay(this->set_min, this->set_sec);
        Disbuff.pushSprite(0, 0);

        xSemaphoreGive(lcd_draw_sem);
    }
}

void Countdown::SetMinuteUpdate()
{
    if (this->set_min == 10) {
        this->set_min = 0;
    }
    else {
        this->set_min++;
    }
}

Countdown user_countdown;