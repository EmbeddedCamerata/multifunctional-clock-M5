#include "../include/utils.h"
#include "../include/sys_err.h"
#include <WiFi.h>

#ifdef DEBUG_MODE
const char *PageStr[4] = {
    "PAGE_WEATHER",
	"PAGE_NTPCLOCK",
	"PAGE_SET_ALARM",
    "PAGE_TIMER",
};
#endif

extern TaskHandle_t xhandle_clock_display;

TFT_eSprite Disbuff = TFT_eSprite(&M5.Lcd);
TaskHandle_t xhandle_wifi_connect = NULL;
TaskHandle_t xhandle_user_ntp_init = NULL;
TaskHandle_t xhandle_user_qweather_init = NULL;
TaskHandle_t xhandle_user_countdown_init = NULL;

/* Functions statement */
static void PageChangRefresh(SysPageType_e NewPage);
void WiFiConnectTask(void *arg);
void PowerDisplay();
SysPageType_e IMUJudge(float accX, float accY, float accZ);

void SystemInit(SysTypeDef *SysAttr)
{
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
#ifdef SYSTEM_INITIAL_PAGE_SELF_ADAPTION
	SysPageType_e page;
	float accX, accY, accZ;

	if ((page = IMUJudge(accX, accY, accZ)) != PAGE_UNKNOWN) {
		SysAttr->SysPage = page;
	}
	else {
		SysAttr->SysPage = SYSTEM_DEFAULT_PAGE;
	}
#else
	SysAttr->SysPage = SYSTEM_DEFAULT_PAGE;
#endif

	Serial.println("System init OK");
	delay(500);

	/* Initialize 4 modules */
	xTaskCreate(WiFiConnectTask, "WiFiConnectTask", 1024*2, \
		(void*)SysAttr, 3, &xhandle_wifi_connect
	);

	xTaskCreate(NTPClockInitTask, "NTPClockInitTask", 1024*2, \
		(void*)SysAttr, 4, &xhandle_user_ntp_init
	);
	xTaskCreate(QWeatherInitTask, "QWeatherInitTask", 1024*4, \
		(void*)SysAttr, 4, &xhandle_user_qweather_init
	);
	xTaskCreate(CountdownTimerInitTask, "CountdownTimerInitTask", 1024, \
		(void*)SysAttr, 4, &xhandle_user_countdown_init
	);
}

void PageUpdate(void *arg)
{
	SysTypeDef *SysAttr = (SysTypeDef*)arg;
	SysPageType_e new_page;
	float accX, accY, accZ;
	
	while(1) {
		M5.IMU.getAccelData(&accX, &accY, &accZ);

		if ((new_page = IMUJudge(accX, accY, accZ)) != PAGE_UNKNOWN) {
			if (new_page != SysAttr->SysPage) {
				PageChangRefresh(new_page);
				SysAttr->SysPage = new_page;
			}
		}
		
		vTaskDelay(50 / portTICK_RATE_MS);
	}
}

static void PageChangRefresh(SysPageType_e NewPage)
{
	switch (NewPage) {
		case PAGE_WEATHER:
			/* Leave */
			User_CountdownTimer.Leave();
			User_NTPClock.Leave();

			/* Suspend Clock display task */
			if (xhandle_clock_display != NULL) {
				vTaskSuspend(xhandle_clock_display);
			}

			User_QWeather.OnMyPage();

			break;

		case PAGE_NTPCLOCK:
			/* Leave */
			User_CountdownTimer.Leave();
			User_QWeather.Leave();

			User_NTPClock.OnMyPage();

			if (xhandle_clock_display != NULL) {
				vTaskResume(xhandle_clock_display);
			}

			break;
		
		case PAGE_SET_ALARM:
			/* Leave */
			User_CountdownTimer.Leave();
			User_NTPClock.Leave();
			User_QWeather.Leave();
			
			/* Suspend Clock display task */
			if (xhandle_clock_display != NULL) {
				vTaskSuspend(xhandle_clock_display);
			}

			break;
		
		case PAGE_TIMER:
			/* Leave */
			User_NTPClock.Leave();
			User_QWeather.Leave();
			/*
				Suspend tasks of other pages
			*/
			// 1. Clock display task
			if (xhandle_clock_display != NULL) {
				vTaskSuspend(xhandle_clock_display);
			}

			/* Tell user_coundown that it is on my page */
			User_CountdownTimer.OnMyPage();
			break;
		
		default: break;
	}
	
#ifdef DEBUG_MODE
	Serial.println(PageStr[NewPage]);
#endif
}

void ButtonsUpdate(void *arg)
{
	SysTypeDef *SysAttr = (SysTypeDef*)arg;

    while (1) {
        M5.update();

        switch (SysAttr->SysPage) {
			case PAGE_WEATHER:
				User_QWeather.ButtonsUpdate(SysAttr);
				break;

			case PAGE_NTPCLOCK:
				User_NTPClock.ButtonsUpdate(SysAttr);

				break;
			
			case PAGE_SET_ALARM:
				break;

			case PAGE_TIMER:
				User_CountdownTimer.ButtonsUpdate();
				
				break;
			
			default: break;
		}

		vTaskDelay(100 / portTICK_RATE_MS);
    }
}

void WiFiConnectTask(void *arg)
{
	EventGroupHandle_t events = ((SysTypeDef*)arg)->SysEvents;
	unsigned long dt;

	Serial.printf("Connecting to %s\n", _SSID);
	
	WiFi.begin(_SSID, _PASSWORD);
	
	dt = millis();
	while (WiFi.status() != WL_CONNECTED) {
        vTaskDelay(500);
        Serial.print(".");

		if (millis() - dt > WIFI_CONNECTION_TIMEOUT * 1000UL) {
			Serial.println("\nWiFi connected failed! Please check ssid and password!");
			vTaskDelete(NULL);
		}
    }
    
	Serial.println("\nConnected!");

	xEventGroupSetBits(events, EVENT_WIFI_CONNECTED);
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

	// xSemaphoreTake(lcd_draw_sem, (TickType_t)10);
	// Disbuff.setTextSize(1);
	// Disbuff.fillRect(10, 10, TFT_LANDSCAPE_WIDTH, Disbuff.fontHeight(), TFT_BLACK);
	// Disbuff.setCursor(10, 10);
	// Disbuff.setTextColor(TFT_RED);

	// Disbuff.printf("V/I/B: %.2f/%.2f/%.2f\n", vbat, ibat, bat);

	// Disbuff.pushSprite(0, 0);
	// xSemaphoreGive(lcd_draw_sem);
}

SysPageType_e IMUJudge(float accX, float accY, float accZ)
{
    if (1 - accX < 0.1) {
        /* accX approx 1 */
        return PAGE_NTPCLOCK;
    }
    else if (1 + accX < 0.1) {
        /* accX approx -1 */
        return PAGE_TIMER;
    }
    else if (1 - accY < 0.1) {
        /* accY approx 1 */
        return PAGE_WEATHER;
    }
    else if (1 + accY < 0.1) {
        /* accY approx -1 */
        return PAGE_SET_ALARM;
    }

    return PAGE_UNKNOWN;
}