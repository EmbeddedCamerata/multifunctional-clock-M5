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
extern TaskHandle_t xhandle_clock_display;
TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
TaskHandle_t xhandle_wifi_connect = NULL;
SemaphoreHandle_t wifi_connected_sem = NULL;
SemaphoreHandle_t lcd_draw_sem = NULL;

/* Functions statement */
void PageChangRefresh(SysPage_e new_page);
void WiFiConnectTask(void *arg);
void PowerDisplay();

void SystemInit(System_TypeDef *SysAttr)
{
	/*
		Initialization
	*/
	// 1. M5
	M5.begin();

	// 2. Create semaphores
	lcd_draw_sem = xSemaphoreCreateMutex();
	if (lcd_draw_sem == NULL) {
		Serial.println("Semaphore lcd_draw_sem error!");
		return;
	}

	wifi_connected_sem = xSemaphoreCreateBinary();
	if (wifi_connected_sem == NULL) {
		Serial.println("Semaphore wifi_connected_sem error!");
		return;
	}

	// 3. WiFi
	xTaskCreate(WiFiConnectTask, "WiFiConnectTask", 1024*2, (void*)0, 4, &xhandle_wifi_connect);

	// 4. LED
	pinMode(M5_LED, OUTPUT);
	digitalWrite(M5_LED, HIGH);

	// 5. IMU
    int rc = M5.IMU.Init(); /* return 0 is ok, return -1 is unknow */
	if (rc < 0) {
		Serial.printf("IMU init error: %d\n", rc);
		return;
	}

	// 6. Sytem page. Self-adaption rotation based on IMU
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

	Serial.println("System init OK");
	delay(500);

	/*
		Display the first data based on initial page
	*/
	User_NTPClock.Init(SysAttr);
	User_Countdown.Init(SysAttr);
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
			User_Countdown.Leave();
			User_NTPClock.Leave();

			/* Suspend Clock display task */
			if (xhandle_clock_display != NULL) {
				vTaskSuspend(xhandle_clock_display);
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
			User_Countdown.Leave();

			User_NTPClock.OnMyPage();

			if (xhandle_clock_display != NULL) {
				vTaskResume(xhandle_clock_display);
			}

			break;
		
		case PAGE_SET_ALARM:
			/* Leave */
			User_Countdown.Leave();
			User_NTPClock.Leave();
			/* Suspend Clock display task */
			if (xhandle_clock_display != NULL) {
				vTaskSuspend(xhandle_clock_display);
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
			User_NTPClock.Leave();
			/*
				Suspend tasks of other pages
			*/
			// 1. Clock display task
			if (xhandle_clock_display != NULL) {
				vTaskSuspend(xhandle_clock_display);
			}

			/* Tell user_coundown that it is on my page */
			User_Countdown.OnMyPage();
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
				User_NTPClock.ButtonsUpdate();

				break;
			
			case PAGE_SET_ALARM:
				break;

			case PAGE_COUNTDOWN:
				User_Countdown.ButtonsUpdate();
				
				break;
			
			default: break;
		}

		vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void WiFiConnectTask(void *arg)
{
	Serial.printf("Connecting to %s", _SSID);
	WiFi.begin(_SSID, _PASSWORD);
	while (WiFi.status() != WL_CONNECTED) {  // If the wifi connection fails.  若wifi未连接成功
        delay(500);
        Serial.print(".");
    }
    Serial.println("\nConnected!");

	xSemaphoreGive(wifi_connected_sem);

	vTaskDelete(NULL);
}

void PowerDisplay()
{
	int disCharge; // Battery output current
	float vbat = 0.0;	// Battery voltage value
	float ibat = 0.0;
	float bat;	// Current Power
	int Vaps;	// Battery capacity
	float temp;	// Battery temperature
	
	disCharge = M5.Axp.GetIdischargeData() / 2;	// Need test without serial
	vbat = M5.Axp.GetBatVoltage();				// 4.19V
	ibat = M5.Axp.GetBatCurrent();				// Need test without serial
	bat = M5.Axp.GetBatPower();					// Need test without serial
	Vaps = M5.Axp.GetVapsData();				// 3567? 
	temp = M5.Axp.GetTempInAXP192(); 			// 44.00

#ifdef DEBUG_MODE
	Serial.printf("V/I: %.2f/%.2f\n", vbat, ibat);
	Serial.printf("Temp: %.2f\n", temp);
#endif

	xSemaphoreTake(lcd_draw_sem, (TickType_t)10);
	Disbuff.setTextSize(1);
	Disbuff.fillRect(10, 10, TFT_LANDSCAPE_WIDTH, Disbuff.fontHeight(), TFT_BLACK);
	Disbuff.setCursor(10, 10);
	Disbuff.setTextColor(TFT_RED);

	Disbuff.printf("V/I/B: %.2f/%.2f/%.2f\n", vbat, ibat, bat);

	Disbuff.pushSprite(0, 0);
	xSemaphoreGive(lcd_draw_sem);
}