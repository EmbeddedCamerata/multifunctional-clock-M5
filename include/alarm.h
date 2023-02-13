#ifndef __ALARM_H__
#define __ALARM_H__

#include "sys_main.h"

#define MAX_ALARM_NUM	2

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
} AlarmTimeTypeDef;

typedef struct
{
	AlarmTimeTypeDef *AlarmList[MAX_ALARM_NUM];
	int WorkingAlarmNum;
} AlarmDataTypeDef;

class Alarm
{
public:
	Alarm();

	void Init(SysTypeDef *SysAttr);
	void ButtonsUpdate(SysTypeDef *SysAttr);
	void OnMyPage();
    void Leave();

	void AddNewAlarm();

	inline void Inited() { this->isInited = true; };
	inline bool IsOnMyPage() { return this->isOnMyPage; };

private:
	bool isInited;
	bool isOnMyPage;
	AlarmTimeTypeDef CurAlarmTime;
	CurPointingLocType_e CurPointingLoc;

	AlarmDataTypeDef AlarmData;
};

extern Alarm User_Alarm;

#endif /* __ALARM_H__ */