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

	this->CurPointingLoc = HOUR_HIGH;
	this->CurAlarmData.AlarmTime = {
		.Hours = 0, .Minutes = 0 	// Don't care "isUsed"
	};
	this->CurAlarmData.Index = 0;

	this->AlarmData.UsedAlarmNum = 0;
	for (int i = 0; i < ALARM_MAX_NUM; i++) {
		this->AlarmData.AlarmList[i] = nullptr;
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
		Short press for BtnA to increse the value of AlarmTime of CurPointingLoc
		Long press for BtnB to check the next alarm time and can change it(optional)
		Short press for BtnB...
	*/
	if (M5.BtnA.wasReleased()) {
		this->ChangeAlarmTime();
	}
	else if (M5.BtnA.wasReleasefor(500)) {
		this->SwitchCurPointingLoc();
	}
	else if (M5.BtnB.wasReleased()) {

	}
	else if (M5.BtnB.wasReleasefor(500)) {
		
	}
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

void Alarm::AddAlarm()
{
	AlarmInfoTypeDef *NewAlarmTime = new AlarmInfoTypeDef;

	assert(this->AlarmData.UsedAlarmNum < ALARM_MAX_NUM);

	if (NewAlarmTime != nullptr) {
		NewAlarmTime->Hours = this->CurAlarmData.AlarmTime.Hours;
		NewAlarmTime->Minutes = this->CurAlarmData.AlarmTime.Minutes;
#ifdef ALARM_STARTS_WHEN_CREATED
		NewAlarmTime->isWorking = true;
#else
		NewAlarmTime->isWorking = false;
#endif

		this->AlarmData.AlarmList[this->CurAlarmData.Index] = NewAlarmTime;
#ifdef DEBUG_MODE
		Serial.printf(
			"Alarm %d:%d\n",											\
			this->AlarmData.AlarmList[this->CurAlarmData.Index]->Hours,	\
			this->AlarmData.AlarmList[this->CurAlarmData.Index]->Minutes
		);
#endif

		this->AlarmData.UsedAlarmNum++;
	}

	delete NewAlarmTime;
}

void Alarm::RemoveAlarm()
{
	if (this->AlarmData.AlarmList[this->CurAlarmData.Index] == nullptr) {
		return;
	}
}

void Alarm::ReadAlarmData()
{
	if (1) {

	}
}

void Alarm::ChangeAlarmTime()
{
	switch (this->CurPointingLoc) {
		case HOUR_HIGH:
			if (this->CurAlarmData.AlarmTime.Hours >= 20) {
				/* >=20 */
				this->CurAlarmData.AlarmTime.Hours -= 20;
			}
			else if (this->CurAlarmData.AlarmTime.Hours > 13) {
				/* >13 */
				this->CurAlarmData.AlarmTime.Hours -= 10;
			}
			else {
				/* <13, add 10 */
				this->CurAlarmData.AlarmTime.Hours += 10;
			}
			break;
		
		case HOUR_LOW:
			if (this->CurAlarmData.AlarmTime.Hours == 23) {
				this->CurAlarmData.AlarmTime.Hours = 0;
			}
			else if (this->CurAlarmData.AlarmTime.Hours % 10 == 9) {
				/* 09 or 19, subtract 9 */
				this->CurAlarmData.AlarmTime.Hours -= 9;
			}
			else {
				this->CurAlarmData.AlarmTime.Hours++;
			}
			break;
		
		case MINUTE_HIGH:
			if (this->CurAlarmData.AlarmTime.Minutes >= 50) {
				this->CurAlarmData.AlarmTime.Minutes -= 50;
			}
			else {
				this->CurAlarmData.AlarmTime.Minutes += 10;
			}
			break;
		
		case MINUTE_LOW:
			if (this->CurAlarmData.AlarmTime.Minutes == 59) {
				this->CurAlarmData.AlarmTime.Minutes = 0;
			}
			else {
				this->CurAlarmData.AlarmTime.Minutes++;
			}
			break;

		default: break;
	}
}

void Alarm::SwitchCurPointingLoc()
{
	this->CurPointingLoc = (this->CurPointingLoc == HOUR_LOW) ? \
		MINUTE_HIGH : (CurPointingLocType_e)(this->CurPointingLoc + 1);
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
    Disbuff.printf("%02d", this->CurAlarmData.AlarmTime.Hours);

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
    Disbuff.printf("%02d", this->CurAlarmData.AlarmTime.Minutes);

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
 * If none of alarms in used, display 00:00. Otherwise, display the nearest one on the screen.
 */
void Alarm::DisplayAlarmStatus()
{
	// TODO Show alarms' status
	int i;

	/* 1. Display circles top left */
	for (i = 0; i < ALARM_MAX_NUM; i++) {
		if (this->AlarmData.AlarmList[i] == nullptr) {
			Disbuff.drawCircle(
				ALARM_STATUS_CIRCLES_X_MARGIN + ALARM_STATUS_CIRCLES_INTERVAL + 10*i, \
				ALARM_STATUS_CIRCLES_Y_MARGIN, 	\
				ALARM_STATUS_CIRCLES_RADIUS, 	\
				TFT_RED
			);
		}
		else {
			Disbuff.fillCircle(
				ALARM_STATUS_CIRCLES_X_MARGIN + ALARM_STATUS_CIRCLES_INTERVAL + 10*i, \
				ALARM_STATUS_CIRCLES_Y_MARGIN, 	\
				ALARM_STATUS_CIRCLES_RADIUS, 	\
				TFT_RED
			);
		}
	}
	
	Disbuff.pushSprite(0, 0);
}

int Alarm::GetWorkingAlarmNum()
{
	int i;

	for (i = 0; i < ALARM_MAX_NUM; i++) {
		if (this->AlarmData.AlarmList[i] != nullptr and \
			this->AlarmData.AlarmList[i]->isWorking == true) {
			i++;
		}
	}

	return i;
}

int Alarm::GetCreatedAlarmNum()
{
	int i;

	for (i = 0; i < ALARM_MAX_NUM; i++) {
		if (this->AlarmData.AlarmList[i] != nullptr) {
			i++;
		}
	}

	return i;
}

void AlarmInitTask(void *arg)
{
    SysPageType_e page = ((SysTypeDef*)arg)->SysPage;
    
    User_Alarm.Init(page);
    vTaskDelete(NULL);
}

Alarm User_Alarm;