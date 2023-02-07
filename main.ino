#include "include/utils.h"

System_TypeDef UserSystem = {
	.SysPage = PAGE_UNKNOWN,
	.SysEvents = NULL
};

TaskHandle_t xhandle_wifi_connect = NULL;
TaskHandle_t xhandle_system_init = NULL;
TaskHandle_t xhandle_page_update = NULL;
TaskHandle_t xhandle_buttons_update = NULL;
SemaphoreHandle_t lcd_draw_sem = NULL;

void setup()
{
	M5.begin();

	// 1. Create semaphores
	lcd_draw_sem = xSemaphoreCreateMutex();
	if (lcd_draw_sem == NULL) {
		Serial.println("Semaphore created failed!");
		return;
	}

	UserSystem.SysEvents = xEventGroupCreate();
	if (UserSystem.SysEvents == NULL) {
		Serial.println("Event group created failed!");
		return;
	}

	xTaskCreate(WiFiConnectTask, "WiFiConnectTask", 1024*2, (void*)0, 3, &xhandle_wifi_connect);
	
	SystemInit(&UserSystem);

	xTaskCreate(PageUpdate, "PageUpdate", 1024*2, (void*)0, 4, &xhandle_page_update);
	xTaskCreate(ButtonsUpdate, "ButtonsUpdate", 1024, (void*)0, 5, &xhandle_buttons_update);
}

void loop()
{

}