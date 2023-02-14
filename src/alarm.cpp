#include "../include/utils.h"
#include <stdlib.h>

extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;

Alarm::Alarm() :		\
	isInited(0),		\
	isOnMyPage(0) {};

void Alarm::Init(SysTypeDef *SysAttr)
{
	if (this->isInited) {
        return;
    }

	this->CurPointingLoc = MINUTE_HIGH;
	this->CurAlarmTime = {.Hours = 0, .Minutes = 0};
	
	this->AlarmData.WorkingAlarmNum = 0;
	for (int i = 0; i < MAX_ALARM_NUM; i++) {
		this->AlarmData.AlarmList[i] = NULL;
	}
}

void Alarm::OnMyPage()
{
    this->isOnMyPage = true;
}

void Alarm::Leave()
{
    this->isOnMyPage = false;
}

void Alarm::AddNewAlarm()
{
	AlarmTimeTypeDef *NewAlarmTime = new AlarmTimeTypeDef;

	if (NewAlarmTime != nullptr) {
		NewAlarmTime->Hours = this->CurAlarmTime.Hours;
		NewAlarmTime->Minutes = this->CurAlarmTime.Minutes;

		this->AlarmData.AlarmList[this->AlarmData.WorkingAlarmNum] = NewAlarmTime;
#ifdef DEBUG_MODE
		Serial.printf("Alarm %d:%d\n", 											\
			this->AlarmData.AlarmList[this->AlarmData.WorkingAlarmNum]->Hours, 	\
			this->AlarmData.AlarmList[this->AlarmData.WorkingAlarmNum]->Minutes
		);
#endif

		this->AlarmData.WorkingAlarmNum++;
	}

	delete NewAlarmTime;
}

void Alarm::TFTRecreate()
{
    M5.Lcd.setRotation(PAGE_SET_ALARM);

    xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
    if (Disbuff.width() != TFT_VERTICAL_WIDTH) {
        Disbuff.deleteSprite();
		Disbuff.createSprite(TFT_VERTICAL_WIDTH, TFT_VERTICAL_HEIGHT);
    }

    Disbuff.fillRect(0, 0, TFT_VERTICAL_WIDTH, TFT_VERTICAL_HEIGHT, TFT_BLACK);
    Disbuff.pushSprite(0, 0);
    xSemaphoreGive(lcd_draw_sem);
}

/**
 * @brief Display current alarm time when setting without xSemaphoreTake
 */
void Alarm::DisplayAlarmTime()
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
    Disbuff.printf("%02d:%02d", this->CurAlarmTime.Hours, this->CurAlarmTime.Minutes);
    Disbuff.pushSprite(0, 0);
}

Alarm User_Alarm;