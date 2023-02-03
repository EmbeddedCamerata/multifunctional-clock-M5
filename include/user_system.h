#ifndef __USER_SYSTEM_H__
#define __USER_SYSTEM_H__

#include <time.h>

typedef enum SysPage
{
    /* Top, Rotation(0) */
    PAGE_TEMPERATURE,
    /* Left landscape, Rotation(1) */
    PAGE_CLOCK, // Using RTC and WIFI
    /* Top reversed, Rotation(2) */
    PAGE_SET_ALARM,
    /* Right landscape, Rotation(3) */
    PAGE_COUNTDOWN, // Using hw_timer for counting
    PAGE_UNKNOWN
} SysPage_e;

typedef struct System_Attr
{
    /* System page */
    SysPage_e SysPage;
    /* To check the update interval */
    struct tm LastNTPTime;
} System_TypeDef;

void SystemInit(System_TypeDef *SysAttr);
void PageUpdate(void *arg);
void ButtonsUpdate(void *arg);

#endif /* __USER_SYSTEM_H__ */