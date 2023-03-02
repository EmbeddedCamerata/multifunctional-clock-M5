#include "../include/utils.h"
#include <stdlib.h>

extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;
TaskHandle_t xhandle_alarm_update_tasks[ALARM_MAX_NUM] = {NULL};

void AlarmUpdateTask(void *arg);

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
		.Hours = 0, .Minutes = 0
	};
	this->CurAlarmData.Index = 0;

	for (int i = 0; i < ALARM_MAX_NUM; i++) {
		this->AlarmList[i] = nullptr;
	}

	if (Page == PAGE_SET_ALARM) {
		this->OnMyPage();
	}
}

void Alarm::ButtonsUpdate(SysTypeDef *SysAttr)
{
	/*
		Short press for BtnA to increse the value of AlarmTime of CurPointingLoc
		Long press for BtnA to switch the CurPointingLoc
		Short press for BtnB to check the next alarm time and can change it
		Long press for BtnB to start/suspend the selected alarm
		The number is blink when the selected alarm IS NOT working.
	*/
	if (M5.BtnA.wasReleased()) {
		this->ChangeAlarmTime();
		this->DisplayCurAlarmTime();
	}
	else if (M5.BtnA.wasReleasefor(500)) {
		this->NextCurPointingLoc();
	}
	else if (M5.BtnB.wasReleased()) {
		this->NextAlarm();
		this->DisplayCurAlarmTime();
	}
	else if (M5.BtnB.wasReleasefor(500)) {
		/* Check the alarm status */
		switch (this->AlarmList[this->CurAlarmData.Index]->Status) {
			case ALARM_NOT_CREATED:
				/* Create the alarm task and update the status */
				xTaskCreate(AlarmUpdateTask, "AlarmUpdateTask", 1024, \
					(void*)0, 7, &xhandle_alarm_update_tasks[this->CurAlarmData.Index]
				);
				this->AlarmList[this->CurAlarmData.Index]->Status = ALARM_WORKING;
				break;
			
			case ALARM_WORKING:
				vTaskSuspend(&xhandle_alarm_update_tasks[this->CurAlarmData.Index]);
				this->AlarmList[this->CurAlarmData.Index]->Status = ALARM_SUSPENDED;
				break;
			
			case ALARM_SUSPENDED:
				vTaskResume(&xhandle_alarm_update_tasks[this->CurAlarmData.Index]);
				this->AlarmList[this->CurAlarmData.Index]->Status = ALARM_WORKING;
				break; 
			
			default: break;
		}
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

	if (NewAlarmTime != nullptr) {
		NewAlarmTime->Hours = this->CurAlarmData.AlarmTime.Hours;
		NewAlarmTime->Minutes = this->CurAlarmData.AlarmTime.Minutes;
#ifdef ALARM_STARTS_WHEN_CREATED
		NewAlarmTime->Status = ALARM_WORKING;
#else
		NewAlarmTime->Status = ALARM_SUSPENDED;
#endif

		this->AlarmList[this->CurAlarmData.Index] = NewAlarmTime;
#ifdef DEBUG_MODE
		Serial.printf(
			"Alarm %d:%d\n",									\
			this->AlarmList[this->CurAlarmData.Index]->Hours,	\
			this->AlarmList[this->CurAlarmData.Index]->Minutes
		);
#endif
	}

	delete NewAlarmTime;
}

void Alarm::RemoveAlarm()
{
	if (this->AlarmList[this->CurAlarmData.Index] == nullptr) {
		return;
	}
}

void Alarm::ReadAlarmData()
{
	if (this->AlarmList[this->CurAlarmData.Index] != nullptr) {
		this->CurAlarmData.AlarmTime.Hours = \
			this->AlarmList[this->CurAlarmData.Index]->Hours;
		this->CurAlarmData.AlarmTime.Minutes = \
			this->AlarmList[this->CurAlarmData.Index]->Minutes;
	}
	else {
		this->CurAlarmData.AlarmTime.Hours = this->CurAlarmData.AlarmTime.Minutes = 0;
	}

}

void Alarm::ChangeAlarmTime()
{
	// TODO Improve the method
	switch (this->CurPointingLoc) {
		case HOUR_HIGH:
			if (this->CurAlarmData.AlarmTime.Hours >= 20) {
				/* >=20 */
				this->CurAlarmData.AlarmTime.Hours -= 20;
			}
			else if (this->CurAlarmData.AlarmTime.Hours > 13) {
				/* >13 */
				this->CurAlarmData.AlarmTime.Hours = 23;
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

void Alarm::NextCurPointingLoc()
{
	this->CurPointingLoc = (this->CurPointingLoc == MINUTE_LOW) ? \
		HOUR_HIGH : (CurPointingLocType_e)(this->CurPointingLoc + 1);
#ifdef DEBUG_MODE
	Serial.printf("At #%d\n", this->CurPointingLoc);
#endif
}

void Alarm::NextAlarm()
{
	this->CurAlarmData.Index = 
		(this->CurAlarmData.Index == ALARM_MAX_NUM) ? 0 : this->CurAlarmData.Index + 1;
#ifdef DEBUG_MODE
	Serial.printf("Alarm #%d\n", this->CurAlarmData.Index);
#endif
	//TODO Refresh current alarm time display. Consider hook.
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
	Disbuff.setTextColor(TFT_BLUE);
    Disbuff.fillRect(
		TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99")/2,							\
		TFT_VERTICAL_HEIGHT/2 + ALARM_TIME_HOUR_Y_OFFSET - Disbuff.fontHeight()/2,	\
		Disbuff.textWidth("99"), Disbuff.fontHeight(),   							\
		TFT_BLACK
	);
    
    Disbuff.setCursor(
		TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99")/2, 							\
        TFT_VERTICAL_HEIGHT/2 + ALARM_TIME_HOUR_Y_OFFSET - Disbuff.fontHeight()/2
	);
    Disbuff.printf("%02d", this->CurAlarmData.AlarmTime.Hours);

	Disbuff.setTextColor(TFT_RED);
	Disbuff.fillRect(
		TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99")/2,								\
		TFT_VERTICAL_HEIGHT/2 + ALARM_TIME_MINUTE_Y_OFFSET - Disbuff.fontHeight()/2,	\
		Disbuff.textWidth("99"), Disbuff.fontHeight(),   								\
		TFT_BLACK
	);
    
    Disbuff.setCursor(
		TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99")/2,	\
        TFT_VERTICAL_HEIGHT/2 + ALARM_TIME_MINUTE_Y_OFFSET - Disbuff.fontHeight()/2
	);
    Disbuff.printf("%02d", this->CurAlarmData.AlarmTime.Minutes);

	Disbuff.setTextColor(TFT_WHITE);
	Disbuff.setCursor(
		TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth(":")/2,	\
        TFT_VERTICAL_HEIGHT/2 - Disbuff.fontHeight()/2
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
		if (this->AlarmList[i] == nullptr) {
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
		if (this->AlarmList[i]->Status == ALARM_WORKING) {
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

void AlarmUpdateTask(void *arg)
{

}

Alarm User_Alarm;