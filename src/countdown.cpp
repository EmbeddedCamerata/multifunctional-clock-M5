#include "../include/utils.h"

extern SysTypeDef UserSystem;
extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;

TaskHandle_t xhandle_countdown_timer_update = NULL;

static void CountdownTimerUpdateTask(void *arg);

CountdownTimer::CountdownTimer() :          \
	isActivated(0),						    \
    isWorking(0),                           \
    isOnMyPage(0),                          \
    set_min(COUNTDOWN_TIMER_DEFAULT_MIN),   \
    set_sec(COUNTDOWN_TIMER_DEFAULT_SEC),   \
    cur_min(COUNTDOWN_TIMER_DEFAULT_MIN),   \
    cur_sec(COUNTDOWN_TIMER_DEFAULT_SEC) {}

void CountdownTimer::Init(SysPageType_e Page)
{
    if (Page == PAGE_TIMER) {
        this->isOnMyPage = true;
        this->TFTRecreate();
    }
}

void CountdownTimer::Begin(uint8_t mins, uint8_t secs)
{
	this->isActivated = this->isWorking = true;
    this->set_min = this->cur_min = mins;
    this->set_sec = this->cur_sec = secs;

    xTaskCreate(CountdownTimerUpdateTask, "CountdownTimerUpdateTask", 1024*2, \
        (void*)0, 6, &xhandle_countdown_timer_update);
}

void CountdownTimer::Pause()
{
	this->isWorking = false;
	
	if (xhandle_countdown_timer_update != NULL) {
		vTaskSuspend(xhandle_countdown_timer_update);
#ifdef DEBUG_MODE
        Serial.println("Pause!");
#endif
	}
	
	this->StatusPromptDisplay("Pause");
}

void CountdownTimer::Resume()
{
	this->isWorking = true;
	
	if (xhandle_countdown_timer_update != NULL) {
		vTaskResume(xhandle_countdown_timer_update);
#ifdef DEBUG_MODE
        Serial.println("Resume!");
#endif
	}

	this->StatusPromptDisplay("Resume");
}

void CountdownTimer::Stop(bool isReset)
{
    /* Turn off the LED */
    digitalWrite(M5_LED, HIGH);

	this->isActivated = this->isWorking = false;

#ifdef RESET_TO_THE_BEGINNING
    this->cur_min = this->set_min;
    this->cur_sec = this->set_sec;
#else
    this->set_min = this->cur_min = COUNTDOWN_TIMER_DEFAULT_MIN;
    this->set_sec = this->cur_sec = COUNTDOWN_TIMER_DEFAULT_SEC;
#endif

	xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
	this->StaticDisplay(this->set_min, this->set_sec);
	xSemaphoreGive(lcd_draw_sem);

	if (xhandle_countdown_timer_update != xTaskGetCurrentTaskHandle()) {
		/* Entering into Stop() from ButtonsUpdate */
		if (xhandle_countdown_timer_update != NULL) {
			vTaskDelete(xhandle_countdown_timer_update);
		}

		if (isReset) {
			this->StatusPromptDisplay("Reset");
		}
		else {
			this->StatusPromptDisplay("Time up!");
		}
	}
	else {
		/* Entering into Stop() from CountdownUpdateTask */
		if (this->isOnMyPage) {
			if (isReset) {
				this->StatusPromptDisplay("Reset");
			}
			else {
				this->StatusPromptDisplay("Time up!");
			}
		}

		if (xhandle_countdown_timer_update != NULL) {
			vTaskDelete(xhandle_countdown_timer_update);
		}
	}
}

void CountdownTimer::OnMyPage()
{
    this->isOnMyPage = true;
    this->TFTRecreate();
	this->UpdateDisplay();
}

void CountdownTimer::Leave()
{
    this->isOnMyPage = false;
}

void CountdownTimer::TimerUpdate()
{
    this->cur_min = (this->cur_sec == 0) ? this->cur_min - 1 : this->cur_min;
    this->cur_sec = (this->cur_sec == 0) ? 59 : this->cur_sec - 1;

    if (this->isOnMyPage) {
        xSemaphoreTake(lcd_draw_sem, (TickType_t)10);
        this->StaticDisplay(this->cur_min, this->cur_sec);
        xSemaphoreGive(lcd_draw_sem);
    }

    if (this->cur_min == 0 and this->cur_sec == 0) {
        this->Stop();
    }
}

void CountdownTimer::ButtonsUpdate()
{
	/* Buttons judgement in working */
	if (this->isActivated) {
		if (M5.BtnA.wasReleased()) {
			if (this->isWorking) {
				this->Pause();
			}
			else {
				this->Resume();
			}
		}
        else if (M5.BtnA.wasReleasefor(500)) {
            /* Reset */
            this->Stop(true);
        }
	}
	/* Buttons judgement in idle */
	else {
		this->SetCoundown();
	}
}

void CountdownTimer::TFTRecreate()
{
    M5.Lcd.setRotation(PAGE_TIMER);

    xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
    if (Disbuff.width() != TFT_LANDSCAPE_WIDTH) {
        Disbuff.deleteSprite();
		Disbuff.createSprite(TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT);
    }

    Disbuff.fillRect(0, 0, TFT_LANDSCAPE_WIDTH, TFT_LANDSCAPE_HEIGHT, TFT_BLACK);
    Disbuff.pushSprite(0, 0);
    xSemaphoreGive(lcd_draw_sem);
}

/*
    @brief 
        Display countdown when setting without xSemaphoreTake
*/
void CountdownTimer::StaticDisplay(uint8_t mins, uint8_t secs)
{
	Disbuff.setTextSize(4);
	Disbuff.setTextColor(TFT_RED);
    Disbuff.fillRect(
		TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2,	\
		TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2,    	\
		Disbuff.textWidth("99:99"), Disbuff.fontHeight(),   	\
		TFT_BLACK
	);
    
    Disbuff.setCursor(
		TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99:99")/2, 	\
        TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2
	);
    Disbuff.printf("%02d:%02d", mins, secs);
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
void CountdownTimer::UpdateDisplay()
{
    xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
    
	if (this->isActivated) {
        this->StaticDisplay(this->cur_min, this->cur_sec);
    }
    else if (this->set_min != COUNTDOWN_TIMER_DEFAULT_MIN or \
            this->set_sec != COUNTDOWN_TIMER_DEFAULT_SEC) {
		/* Idle but changed */
        this->StaticDisplay(this->set_min, this->set_sec);
    }
    else {
		/* Idle */
        this->StaticDisplay(COUNTDOWN_TIMER_DEFAULT_MIN, COUNTDOWN_TIMER_DEFAULT_SEC);
    }

    xSemaphoreGive(lcd_draw_sem);
}

void CountdownTimer::StatusPromptDisplay(const char *StrToShow)
{
	xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
	
	Disbuff.setCursor(10, 10);
	Disbuff.setTextSize(2);
	Disbuff.setTextColor(TFT_WHITE);
	Disbuff.fillRect(10, 10, TFT_LANDSCAPE_WIDTH/2, Disbuff.fontHeight(), TFT_BLACK);
	Disbuff.print(StrToShow);
	Disbuff.pushSprite(0, 0);
	
	xSemaphoreGive(lcd_draw_sem);
	
	vTaskDelay(800 / portTICK_RATE_MS);
	
	xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
	
	Disbuff.fillRect(10, 10, TFT_LANDSCAPE_WIDTH/2, Disbuff.fontHeight(), TFT_BLACK);
	Disbuff.pushSprite(0, 0);

	xSemaphoreGive(lcd_draw_sem);
}

void CountdownTimer::SetCoundown()
{
    bool isChanged = true;

    if (M5.BtnA.wasReleased()) {
        /* Short press of BtnA for +1 second */
        if (this->set_sec == 59) {
            this->set_sec = 0;
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

void CountdownTimerInitTask(void *arg)
{
    SysPageType_e page = ((SysTypeDef*)arg)->SysPage;
    
    User_CountdownTimer.Init(page);
    vTaskDelete(NULL);
}

static void CountdownTimerUpdateTask(void *arg)
{
    // TickType_t last_tick = xTaskGetTickCount();

    while(1) {
        // xTaskDelayUntil(&last_tick, 1000 / portTICK_RATE_MS);
		vTaskDelay(1000 / portTICK_RATE_MS);
        digitalWrite(M5_LED, 1 - digitalRead(M5_LED));
        User_CountdownTimer.TimerUpdate();
    };
}

CountdownTimer User_CountdownTimer;