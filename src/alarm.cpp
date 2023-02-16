#include "../include/utils.h"
#include <stdlib.h>

extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;

Alarm::Alarm() :		\
	isInited(0),		\
	isOnMyPage(0) {};

void Alarm::Init(SysPageType_e Page)
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

	if (Page == PAGE_SET_ALARM) {
		this->OnMyPage();
	}
}

void Alarm::ButtonsUpdate(SysTypeDef *SysAttr)
{

}

void Alarm::OnMyPage()
{
    this->isOnMyPage = true;
	this->TFTRecreate();

	xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
	this->DisplayAlarmStatus();
	this->DisplayCurAlarmTime();
	xSemaphoreGive(lcd_draw_sem);
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
void Alarm::DisplayCurAlarmTime()
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
    Disbuff.printf(
		"%02d:%02d", this->CurAlarmTime.Hours, this->CurAlarmTime.Minutes
	);
    Disbuff.pushSprite(0, 0);
}

/**
 * @brief Display all alarm status at the top right. Filled circle is IN USED, otherwise AVAILABLE.
 */
void Alarm::DisplayAlarmStatus()
{
	Disbuff.setCursor(8, 8);
	Disbuff.drawCircle(8+3, 8, 3, TFT_RED);
	Disbuff.drawCircle(8+13, 8, 3, TFT_RED);
	Disbuff.fillCircle(8+23, 8, 3, TFT_RED);
	Disbuff.pushSprite(0, 0);
}

void AlarmInitTask(void *arg)
{
    SysPageType_e page = ((SysTypeDef*)arg)->SysPage;
    
    User_Alarm.Init(page);
    vTaskDelete(NULL);
}

Alarm User_Alarm;