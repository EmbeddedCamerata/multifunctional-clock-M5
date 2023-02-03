#include "../include/utils.h"
#include <WiFi.h>

#ifdef DEBUG_MODE
const char *PageStr[4] = {
    "PAGE_TEMPERATURE",
	"PAGE_CLOCK",
	"PAGE_SET_ALARM",
    "PAGE_COUNTDOWN",
};
#endif

extern System_TypeDef UserSystem;
extern TaskHandle_t xhandle_clock_display_update;
TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
SemaphoreHandle_t lcd_draw_sem = NULL;

/* Functions statement */
void PageChangRefresh(SysPage_e new_page);

void SystemInit(System_TypeDef *SysAttr)
{
	/*
		Initialization
	*/
	// 1. M5
	M5.begin();

	// 2. LED
	pinMode(M5_LED, OUTPUT);
	digitalWrite(M5_LED, HIGH);

	// 3. IMU
    int rc = M5.IMU.Init(); /* return 0 is ok, return -1 is unknow */
	if (rc < 0) {
		Serial.printf("IMU init error: %d\n", rc);
		return;
	}

	// 4. Sytem page. Self-adaption rotation based on IMU
#ifdef INITIAL_PAGE_SELF_ADAPTION
	SysPage_e page;
	float accX, accY, accZ;

	if ((page = IMUJudge(accX, accY, accZ)) != PAGE_UNKNOWN) {
		SysAttr->SysPage = page;
	}
	else {
		SysAttr->SysPage = INITIAL_DEFAULT_PAGE;
	}
#else
	SysAttr->SysPage = INITIAL_DEFAULT_PAGE;
#endif

	// 5. WiFi
	Serial.printf("Connecting to %s", _SSID);
	WiFi.begin(_SSID, _PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {  // If the wifi connection fails.  若wifi未连接成功
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");

	// 6. Create semaphores
	lcd_draw_sem = xSemaphoreCreateMutex();
	if (lcd_draw_sem == NULL) {
		Serial.println("Semaphore lcd_draw_sem error!");
		return;
	}

	Serial.println("System init OK");
	delay(500);

	/*
		Display the first data based on initial page
	*/
	user_NTPclock.Init(SysAttr);
	user_countdown.Init(SysAttr);
}

void PageUpdate(void *arg)
{
	SysPage_e new_page;
	float accX, accY, accZ;
	
	while(1) {
		M5.IMU.getAccelData(&accX, &accY, &accZ);

		if ((new_page = IMUJudge(accX, accY, accZ)) != PAGE_UNKNOWN) {
			if (new_page != UserSystem.SysPage) {
				PageChangRefresh(new_page);
				UserSystem.SysPage = new_page;
			}
		}
		
		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

void PageChangRefresh(SysPage_e new_page)
{
	switch (new_page) {
		case PAGE_TEMPERATURE:
			/* Leave */
			user_countdown.Leave();
			user_NTPclock.Leave();

			/* Suspend Clock display task */
			if (xhandle_clock_display_update != NULL) {
				vTaskSuspend(xhandle_clock_display_update);
			}
			
			xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
			M5.Lcd.setRotation(0);
			Disbuff.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_BLACK);

#ifdef DEBUG_MODE
			Disbuff.setCursor(Disbuff.height()-10, 10);
			Disbuff.print(PageStr[new_page]);
#endif
			Disbuff.pushSprite(0, 0);
			xSemaphoreGive(lcd_draw_sem);

			break;

		case PAGE_CLOCK:
			/* Leave */
			user_countdown.Leave();

			user_NTPclock.OnMyPage();

			if (xhandle_clock_display_update != NULL) {
				vTaskResume(xhandle_clock_display_update);
			}

			break;
		
		case PAGE_SET_ALARM:
			/* Leave */
			user_countdown.Leave();
			user_NTPclock.Leave();
			/* Suspend Clock display task */
			if (xhandle_clock_display_update != NULL) {
				vTaskSuspend(xhandle_clock_display_update);
			}

			M5.Lcd.setRotation(2);
			Disbuff.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_BLACK);
#ifdef DEBUG_MODE
			Disbuff.setCursor(120, 0);
			Disbuff.print(PageStr[new_page]);
#endif
			Disbuff.pushSprite(0, 0);
			xSemaphoreGive(lcd_draw_sem);

			break;
		
		case PAGE_COUNTDOWN:
			/* Leave */
			user_NTPclock.Leave();
			/*
				Suspend tasks of other pages
			*/
			// 1. Clock display task
			if (xhandle_clock_display_update != NULL) {
				vTaskSuspend(xhandle_clock_display_update);
			}

			/* Tell user_coundown that it is on my page */
			user_countdown.OnMyPage();
			break;
		
		default: break;
	}
	
#ifdef DEBUG_MODE
	Serial.println(PageStr[new_page]);
#endif
}

void ButtonsUpdate(void *arg)
{
    while (1) {
        M5.update();

        switch (UserSystem.SysPage) {
			case PAGE_TEMPERATURE:
				break;

			case PAGE_CLOCK:
				if (M5.BtnA.wasReleased()) {
				/* Short press of BtnA for update NTP time immediately */
					user_NTPclock.UpdateNow();
#ifdef DEBUG_MODE
        			Serial.println("Button A released");
#endif
    			}
				break;
			
			case PAGE_SET_ALARM:
				break;

			case PAGE_COUNTDOWN:
				/* Buttons judgement in working */
				if (user_countdown.IsWorking()) {
					// TODO
				}
				/* Buttons judgement in idle */
				else {
					user_countdown.SetCoundown();
				}
				
				break;
			
			default: break;
		}

		vTaskDelay(50 / portTICK_RATE_MS);
    }
}