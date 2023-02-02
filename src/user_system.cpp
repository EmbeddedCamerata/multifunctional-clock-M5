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
TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
SemaphoreHandle_t lcd_draw_sem = NULL;

/* Functions statement */
void PageChangRefresh(SystemPage_e new_page, bool recreate_sprite=false);

void SystemInit(System_TypeDef *SysAttr)
{
	M5.begin();

	/* Initialization */
	// 1. Set the initial page
	SysAttr->SysPage = PAGE_CLOCK;

	// 2. LED
	pinMode(M5_LED, OUTPUT);

	// 3. LCD
	M5.Lcd.setRotation(1); /* Default, clock */
	Disbuff.createSprite(TFT_HEIGHT, TFT_WIDTH);

	// 4. IMU
    int rc = M5.IMU.Init(); /* return 0 is ok, return -1 is unknow */
	if (rc < 0) {
		Serial.print("IMU init error:");Serial.println(rc);
	}

	// 5. WiFi
	Serial.printf("Connecting to %s", _SSID);
	WiFi.begin(_SSID, _PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {  // If the wifi connection fails.  若wifi未连接成功
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");

	lcd_draw_sem = xSemaphoreCreateMutex();
	if (lcd_draw_sem == NULL) {
		Serial.println("Semaphore lcd_draw_sem error!");
	}

	Serial.println("System init OK");

	delay(500);

	/* Prepare the data */
	// 1. Get the time and update RTC
	configTime(8*3600, 3600, ntpServer);  // init and get the time.  初始化并设置NTP
    UpdateLocalTime();
    WiFi.disconnect(true);  // Disconnect wifi.  断开wifi连接
    WiFi.mode(WIFI_OFF);
}

void PageUpdate(void *arg)
{
	float accX, accY, accZ;
	SystemPage_e new_page = *(SystemPage_e*)arg;
	
	while(1) {
		M5.IMU.getAccelData(&accX, &accY, &accZ);

		if (1 - accX < 0.1) {
			/* accX approx 1 */
			new_page = PAGE_CLOCK;
		}
		else if (1 + accX < 0.1) {
			/* accX approx -1 */
			new_page = PAGE_COUNTDOWN;
		}
		else if (1 - accY < 0.1) {
			/* accY approx 1 */
			new_page = PAGE_TEMPERATURE;
		}
		else if (1 + accY < 0.1) {
			/* accY approx -1 */
			new_page = PAGE_SET_ALARM;
		}

		if (new_page != UserSystem.SysPage) {
			if (UserSystem.SysPage + new_page == 2 or \
				UserSystem.SysPage + new_page == 4) {
				/* No need for recreate sprite */
				PageChangRefresh(new_page, false);
			}
			else {
				/* Need recreate sprite */
				PageChangRefresh(new_page, true);
			}
			UserSystem.SysPage = new_page;
		}

		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

void PageChangRefresh(SystemPage_e new_page, bool recreate_sprite)
{
	// if (new_page == PAGE_TEMPERATURE) {
	// 	M5.Lcd.setRotation(0);
	// }
	// else if (new_page == PAGE_CLOCK) {
	// 	M5.Lcd.setRotation(1);
	// }
	// else if (new_page == PAGE_SET_ALARM) {
	// 	M5.Lcd.setRotation(2);
	// }
	// else if (new_page == PAGE_COUNTDOWN) {
	// 	M5.Lcd.setRotation(3);
	// }

	xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);

	if (recreate_sprite) {
		/* Recreate sprite */
		if (Disbuff.width() == TFT_HEIGHT) {
			/* Landscape to vertical */
			Disbuff.deleteSprite();
			Disbuff.createSprite(TFT_WIDTH, TFT_HEIGHT);
		}
		else {
			/* Vertical to landscape */
			Disbuff.deleteSprite();
			Disbuff.createSprite(TFT_HEIGHT, TFT_WIDTH);
		}
	}
	Disbuff.setTextColor(TFT_WHITE);
	Disbuff.setTextSize(1);

	switch (new_page) {
		case PAGE_TEMPERATURE:
			M5.Lcd.setRotation(0);
			Disbuff.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_BLACK);

#ifdef DEBUG_MODE
			Disbuff.setCursor(100, 120);
			Disbuff.print(PageStr[new_page]);
#endif
			break;

		case PAGE_CLOCK:
			M5.Lcd.setRotation(1);
			Disbuff.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_BLACK);
#ifdef DEBUG_MODE
			Disbuff.setCursor(80, 120);
			Disbuff.print(PageStr[new_page]);
#endif
			break;
		
		case PAGE_SET_ALARM:
			M5.Lcd.setRotation(2);
			Disbuff.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_BLACK);
#ifdef DEBUG_MODE
			Disbuff.setCursor(120, 0);
			Disbuff.print(PageStr[new_page]);
#endif
			break;
		
		case PAGE_COUNTDOWN:
			M5.Lcd.setRotation(3);
			Disbuff.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_BLACK);
#ifdef DEBUG_MODE
			Disbuff.setCursor(120, 0);
			Disbuff.print(PageStr[new_page]);
#endif
			user_countdown.PageChangeDisplay();
			break;
		
		default: break;
	}
	
	Disbuff.pushSprite(0, 0);
	xSemaphoreGive(lcd_draw_sem);
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