#ifndef __USER_SYSTEM_H__
#define __USER_SYSTEM_H__

#include <M5StickCPlus.h>
#include "utils.h"

typedef enum SystemPage {
    /* Left, Rotation(1) */
    PAGE_CLOCK, // RTC clock and calibre from WIFI
    /* Top, Rotation(2) */
    PAGE_TEMPERATURE,
    /* Right, Rotation(3) */
    PAGE_COUNTDOWN,
    /* Down, Rotation(4) */
    PAGE_SET_ALARM
} SystemPage_e;

typedef struct System_Attr {
    SystemPage_e SysPage;
} System_TypeDef;

void SystemInit(System_TypeDef *SysAttr);
void PageUpdate(void *arg);
void ButtonsUpdate(void *arg);

#endif /* __USER_SYSTEM_H__ */