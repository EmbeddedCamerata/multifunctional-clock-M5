#ifndef __USER_SYSTEM_H__
#define __USER_SYSTEM_H__

#include <time.h>
// TODO A single task to handle all the errors or failures
#define EVENT_WIFI_CONNECTED_FLAG   BIT0
#define EVENT_NTP_INITIAL_OK_FLAG   BIT1

typedef enum SysPage
{
    /* Vertical, Rotation(0) */
    PAGE_WEATHER = 0U,  /* W */
    /* Left landscape, Rotation(1) */
    PAGE_NTPCLOCK,      /* C */
    /* Vertical reversed, Rotation(2) */
    PAGE_SET_ALARM,     /* A */
    /* Right landscape, Rotation(3) */
    PAGE_TIMER,         /* T */
    PAGE_UNKNOWN
} SysPageType;

typedef struct SysAttr
{
    /* System page */
    SysPageType SysPage;
    /* To check the update interval */
    struct tm LastNTPTime;

    EventGroupHandle_t SysEvents;
} SysTypeDef;

void SystemInit(SysTypeDef *SysAttr);
void PageUpdate(void *arg);
void ButtonsUpdate(void *arg);
void WiFiConnectTask(void *arg);

#endif /* __USER_SYSTEM_H__ */