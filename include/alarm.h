#ifndef __ALARM_H__
#define __ALARM_H__

#include "sys_main.h"

#define ALARM_MAX_NUM					3
#define ALARM_STATUS_CIRCLES_X_MARGIN	8
#define ALARM_STATUS_CIRCLES_Y_MARGIN	8
#define ALARM_STATUS_CIRCLES_RADIUS		3
#define ALARM_STATUS_CIRCLES_INTERVAL	3

#define ALARM_STARTS_WHEN_CREATED

#define ALARM_TIME_HOUR_Y_OFFSET		(-(TFT_VERTICAL_HEIGHT/6))
#define ALARM_TIME_MINUTE_Y_OFFSET		(-ALARM_TIME_HOUR_Y_OFFSET)

/* Indicate which number is ready to change */
typedef enum
{
    HOUR_HIGH = 0U,
    HOUR_LOW,
    MINUTE_HIGH,
    MINUTE_LOW,
} CurPointingLocType_e;

typedef enum
{
    ALARM_NOT_CREATED = 0U,
    ALARM_WORKING,
    ALARM_SUSPENDED
} AlarmStatusType_e;

typedef enum
{
    ALARM_MINUTE_UPDATE = 0U,
    ALARM_HOUR_UPDATE
} AlarmModeType_e;

/* Save the alarm info */
typedef struct
{
    uint8_t Hours;
    uint8_t Minutes;
    AlarmStatusType_e Status;
} AlarmInfoTypeDef;

typedef struct
{
    AlarmInfoTypeDef AlarmTime;
    int Index;
} CurAlarmDataTypedef;

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
    void ReadAlarmData();
    void ChangeAlarmTime();
    void NextCurPointingLoc();
    void NextAlarm();

    inline void Inited() { this->isInited = true; };
    inline bool IsOnMyPage() { return this->isOnMyPage; };

private:
    void TFTRecreate();

    void DisplayCurAlarmTime();
    void DisplayAlarmStatus();

    int GetWorkingAlarmNum();

    bool isInited;
    bool isOnMyPage;
    bool CurAlarmReload;
    CurPointingLocType_e CurPointingLoc;

    CurAlarmDataTypedef CurAlarmData;
    AlarmInfoTypeDef *AlarmList[ALARM_MAX_NUM];
};

void AlarmInitTask(void *arg);

extern Alarm User_Alarm;

#endif /* __ALARM_H__ */