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
	this->CurAlarmTime = {.Hours = 0, .Minutes = 0, .inUsed = false};

	for (int i = 0; i < ALARM_MAX_NUM; i++) {
		this->AlarmList[i] = NULL;
	}

	if (Page == PAGE_SET_ALARM) {
		this->OnMyPage();
	}
}

void Alarm::ButtonsUpdate(SysTypeDef *SysAttr)
{
	// TODO
	/*
		Long press for BtnA to switch the CurPointingLoc
		Short press for BtnA to increse the value of CurAlarmTime of CurPointingLoc
		Long press for BtnB to check the next alarm time and can change it(optional)
		Short press for BtnB...
	*/
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

		this->AlarmList[this->AlarmData.WorkingAlarmNum] = NewAlarmTime;
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

void Alarm::ChangeAlarmTime()
{
	switch (this->CurPointingLoc) {
		case HOUR_HIGH:
			if (this->CurAlarmTime.Hours >= 20) {
				/* >=20 */
				this->CurAlarmTime.Hours -= 20;
			}
			else if (this->CurAlarmTime.Hours > 13) {
				/* >13 */
				this->CurAlarmTime.Hours -= 10;
			}
			else {
				/* <13, add 10 */
				this->CurAlarmTime.Hours += 10;
			}
			break;
		
		case HOUR_LOW:
			if (this->CurAlarmTime.Hours == 23) {
				this->CurAlarmTime.Hours = 0;
			}
			else if (this->CurAlarmTime.Hours % 10 == 9) {
				/* 09 or 19, subtract 9 */
				this->CurAlarmTime.Hours -= 9;
			}
			else {
				this->CurAlarmTime.Hours++;
			}
			break;
		
		case MINUTE_HIGH:
			if (this->CurAlarmTime.Minutes >= 50) {
				this->CurAlarmTime.Minutes -= 50;
			}
			else {
				this->CurAlarmTime.Minutes += 10;
			}
			break;
		
		case MINUTE_LOW:
			if (this->CurAlarmTime.Minutes == 59) {
				this->CurAlarmTime.Minutes = 0;
			}
			else {
				this->CurAlarmTime.Minutes++;
			}
			break;

		default: break;
	}
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
 * Display this:
 * 12
 * :
 * 34
 */
void Alarm::DisplayCurAlarmTime()
{
	Disbuff.setTextSize(4);
	Disbuff.setTextColor(TFT_RED);
    Disbuff.fillRect(
		TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99")/2,		\
		TFT_VERTICAL_HEIGHT/2 - 15 - Disbuff.fontHeight()/2,	\
		Disbuff.textWidth("99"), Disbuff.fontHeight(),   		\
		TFT_BLACK
	);
    
    Disbuff.setCursor(
		TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99")/2, 		\
        TFT_LANDSCAPE_HEIGHT/2 - 15 - Disbuff.fontHeight()/2
	);
    Disbuff.printf("%02d", this->CurAlarmTime.Hours);

	Disbuff.fillRect(
		TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99")/2,		\
		TFT_VERTICAL_HEIGHT/2 + 15 + Disbuff.fontHeight()/2,	\
		Disbuff.textWidth("99"), Disbuff.fontHeight(),   		\
		TFT_BLACK
	);
    
    Disbuff.setCursor(
		TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth("99")/2, 		\
        TFT_LANDSCAPE_HEIGHT/2 + 15 + Disbuff.fontHeight()/2
	);
    Disbuff.printf("%02d", this->CurAlarmTime.Minutes);

	Disbuff.setTextColor(TFT_WHITE);
	Disbuff.setCursor(
		TFT_LANDSCAPE_WIDTH/2 - Disbuff.textWidth(":")/2, 		\
        TFT_LANDSCAPE_HEIGHT/2 - Disbuff.fontHeight()/2
	);
	Disbuff.print(":");
    Disbuff.pushSprite(0, 0);
}

/**
 * @brief Display all alarm status at the top right. Filled circle is IN USED, otherwise AVAILABLE.
 * If none of alarms in used, display 00:00. Else, display the nearest one on the screen.
 */
void Alarm::DisplayAlarmStatus()
{
	// TODO Show alarms' status
	int i;

	for (i = 0; i < ALARM_MAX_NUM; i++) {
		
	}
	
	
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