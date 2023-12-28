#include "include/utils.h"

SysTypeDef UserSystem = {
    .SysPage = PAGE_UNKNOWN,
    .SysEvents = NULL,
};

TaskHandle_t xhandle_page_update = NULL;
TaskHandle_t xhandle_buttons_update = NULL;
SemaphoreHandle_t lcd_draw_sem = NULL;

void setup()
{
    M5.begin();

    // 1. Create semaphores
    if ((lcd_draw_sem = xSemaphoreCreateMutex()) == NULL)
    {
        Serial.println("Semaphore created failed!");
        return;
    }

    if ((UserSystem.SysEvents = xEventGroupCreate()) == NULL)
    {
        Serial.println("Event group created failed!");
        return;
    }

    SystemInit(&UserSystem);

    xTaskCreate(PageUpdate, "PageUpdate", 1024 * 2, (void *)&UserSystem, 4, &xhandle_page_update);
    xTaskCreate(ButtonsUpdate, "ButtonsUpdate", 1024 * 2, (void *)&UserSystem, 5, &xhandle_buttons_update);
}

void loop()
{
}