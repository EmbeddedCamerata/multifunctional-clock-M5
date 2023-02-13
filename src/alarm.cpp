#include "../include/utils.h"
#include <stdlib.h>

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

Alarm User_Alarm;