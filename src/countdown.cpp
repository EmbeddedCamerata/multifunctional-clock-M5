#include "../include/utils.h"

extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;
TaskHandle_t xhandle_countdown_update_task = NULL;

void CountdownUpdateTask(void *arg);

Countdown::Countdown() :                \
    isWorking(0),                       \
    isOnMyPage(0),                      \
    set_min(COUNTDOWN_DEFAULT_MIN),     \
    set_sec(COUNTDOWN_DEFAULT_SEC),     \
    cur_min(COUNTDOWN_DEFAULT_MIN),     \
    cur_sec(COUNTDOWN_DEFAULT_SEC) {}

void Countdown::Init(System_TypeDef *SysAttr)
{
    if (SysAttr->SysPage == PAGE_COUNTDOWN) {
        Disbuff.createSprite(TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT);
        this->OnMyPage();
    }
}

void Countdown::Begin(uint8_t mins, uint8_t secs)
{
    this->isWorking = true;
    this->set_min = this->cur_min = mins;
    this->set_sec = this->cur_sec = secs;

    xTaskCreate(CountdownUpdateTask, "CountdownUpdateTask", 1024*2, \
        (void*)0, 6, &xhandle_countdown_update_task);
}

/*
    @brief 
        Display countdown when setting without xSemaphoreTake
*/
void Countdown::StaticDisplay(uint8_t mins, uint8_t secs)
{
    String _time = "%02d:%02d";

    Disbuff.fillRect(TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2,  \
                        TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2,    \
                        Disbuff.textWidth("99:99"), Disbuff.fontHeight(),   \
                        TFT_BLACK);
    
    Disbuff.setTextSize(4);
    Disbuff.setCursor(TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2, \
                        TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2);
    Disbuff.setTextColor(TFT_RED);
    Disbuff.printf(_time.c_str(), mins, secs);
    Disbuff.pushSprite(0, 0);
}

/*
    @brief
        When system page changes to PAGE_COUNTDOWN, display.
        Idle -> Idle
        Idle -> Idle but changed
        Working -> Working
        Working -> Idle
*/
void Countdown::UpdateDisplay()
{
    xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
    if (this->isWorking) {
		/* Working */
        this->StaticDisplay(this->cur_min, this->cur_sec);
    }
    else if (this->set_min != COUNTDOWN_DEFAULT_MIN or \
            this->set_sec != COUNTDOWN_DEFAULT_SEC) {
		/* Idle but changed */
        this->StaticDisplay(this->set_min, this->set_sec);
    }
    else {
		/* Idle */
        this->StaticDisplay(COUNTDOWN_DEFAULT_MIN, COUNTDOWN_DEFAULT_SEC);
    }

    xSemaphoreGive(lcd_draw_sem);
}

void Countdown::Update()
{
    this->cur_min = (this->cur_sec == 0) ? this->cur_min - 1 : this->cur_min;
    this->cur_sec = (this->cur_sec == 0) ? 59 : this->cur_sec - 1;

#ifdef DEBUG_MODE
    Serial.printf("%02d:%02d left\n", this->cur_min, this->cur_sec);
#endif

    if (this->isOnMyPage) {
        xSemaphoreTake(lcd_draw_sem, 0);
        this->StaticDisplay(this->cur_min, this->cur_sec);
        xSemaphoreGive(lcd_draw_sem);
    }

    if (this->cur_min == 0 and this->cur_sec == 0) {
        this->Stop(false);
    }
}

void Countdown::Stop(bool isShutdown)
{
    /* Turn off the LED */
    digitalWrite(M5_LED, HIGH);

    this->isWorking = false;
    this->set_min = this->cur_min = COUNTDOWN_DEFAULT_MIN;
    this->set_sec = this->cur_sec = COUNTDOWN_DEFAULT_SEC;

    /* Show "Time up" */
    if (this->isOnMyPage) {
        xSemaphoreTake(lcd_draw_sem, (TickType_t)10);
        Disbuff.setCursor(10, 10);
        Disbuff.setTextSize(2);
        Disbuff.fillRect(10, 10, TFT_LANDSCAPE_WIDTH, Disbuff.fontHeight(), TFT_BLACK);
        Disbuff.setTextColor(TFT_WHITE);

        if (isShutdown) {
            Disbuff.printf("Reset");
        }
        else {
            Disbuff.printf("Time up!");
        }

        Disbuff.pushSprite(0, 0);
        xSemaphoreGive(lcd_draw_sem);
    }
}

void Countdown::OnMyPage()
{
    this->isOnMyPage = true;
    M5.Lcd.setRotation(PAGE_COUNTDOWN);

    if (TFT_LANDSCAPE_WIDTH != TFT_LANDSCAPE_WIDTH) {
        Disbuff.deleteSprite();
		Disbuff.createSprite(TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT);
    }

    Disbuff.fillRect(0, 0, TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT, TFT_BLACK);

	this->UpdateDisplay();
}

void Countdown::Leave()
{
    this->isOnMyPage = false;
}

void Countdown::SetCoundown()
{
    bool isChanged = true;

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
        this->Begin(this->set_min, this->set_sec);
		isChanged = false;
    }
    else {
        isChanged = false;
    }
    
    if (isChanged) {
        this->UpdateDisplay();
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

void CountdownUpdateTask(void *arg)
{
    TickType_t last_tick = xTaskGetTickCount();

    do {
        xTaskDelayUntil(&last_tick, 1000 / portTICK_RATE_MS);
        digitalWrite(M5_LED, 1 - digitalRead(M5_LED));
        user_countdown.Update();

		if (!user_countdown.IsWorking()) {
			break;
		}
    } while (1);
}

Countdown user_countdown;