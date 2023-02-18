#ifndef __ALARM_H__
#define __ALARM_H__

#include "sys_main.h"

#define ALARM_MAX_NUM					2
#define ALARM_STATUS_CIRCLES_X_MARGIN	8
#define ALARM_STATUS_CIRCLES_Y_MARGIN	8
#define ALARM_STATUS_CIRCLES_RADIUS		3
#define ALARM_STATUS_CIRCLES_INTERVAL	3

#define ALARM_STARTS_WHEN_CREATED

typedef enum
{
	MINUTE_HIGH = 0U,
	MINUTE_LOW,
	HOUR_HIGH,
	HOUR_LOW
} CurPointingLocType_e;

typedef struct
{
	uint8_t Hours;
    uint8_t Minutes;
	bool isWorking;
} AlarmTimeTypeDef;

typedef struct
{
	AlarmTimeTypeDef *AlarmList[ALARM_MAX_NUM];
	int UsedAlarmNum;
} AlarmDataTypeDef;

class Alarm
{
public:
	Alarm();

	void Init(SysPageType_e Page);
	void ButtonsUpdate(SysTypeDef *SysAttr);
	void OnMyPage();
    void Leave();

	void AddAlarm();
	void RemoveAlarm();
	void ChangeAlarmTime();

	inline void Inited() { this->isInited = true; };
	inline bool IsOnMyPage() { return this->isOnMyPage; };

private:
	void TFTRecreate();
	void DisplayCurAlarmTime();
	void DisplayAlarmStatus();

	int GetWorkingAlarmNum();
	int GetCreatedAlarmNum();

	bool isInited;
	bool isOnMyPage;
	AlarmTimeTypeDef CurAlarmTime;
	CurPointingLocType_e CurPointingLoc;

	AlarmDataTypeDef AlarmData;
};

void AlarmInitTask(void *arg);

extern Alarm User_Alarm;

#endif /* __ALARM_H__ */