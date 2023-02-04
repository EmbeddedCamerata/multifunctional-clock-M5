#include "include/utils.h"

System_TypeDef UserSystem = {
	.SysPage = PAGE_UNKNOWN
};
TaskHandle_t xhandle_page_update = NULL;
TaskHandle_t xhandle_buttons_update = NULL;

void setup() {
    SystemInit(&UserSystem);

	xTaskCreate(PageUpdate, "PageUpdate", 1024*2, (void*)0, 4, &xhandle_page_update);
	xTaskCreate(ButtonsUpdate, "ButtonsUpdate", 1024*2, (void*)0, 5, &xhandle_buttons_update);
}

void loop() {

}