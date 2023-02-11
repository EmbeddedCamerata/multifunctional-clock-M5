#ifndef __SYS_MAIN_H__
#define __SYS_MAIN_H__

#include <time.h>

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
} SysPageType_e;

typedef struct SysAttr
{
    SysPageType_e SysPage;            /* System page */
    EventGroupHandle_t SysEvents;
} SysTypeDef;

void SystemInit(SysTypeDef *SysAttr);
void PageUpdate(void *arg);
void ButtonsUpdate(void *arg);

#endif /* __SYS_MAIN_H__ */