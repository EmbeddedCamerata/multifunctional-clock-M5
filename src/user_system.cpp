#include "../include/utils.h"

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

/* Functions statement */
void PageChangRefresh(SystemPage_e new_page);

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

	Disbuff.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_BLACK);
    Disbuff.setTextSize(2);
    Disbuff.setCursor(Disbuff.width()/2 - Disbuff.textWidth("Hello World")/2, Disbuff.height()/2 - Disbuff.fontHeight()/2);
    Disbuff.setTextColor(TFT_RED);
    Disbuff.printf("Hello World");
    Disbuff.pushSprite(0, 0);

	Serial.println("System init OK");

	delay(500);
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
			/* do page update here */
			PageChangRefresh(new_page);
			UserSystem.SysPage = new_page;
		}

		delay(50);
	}
}

void PageChangRefresh(SystemPage_e new_page)
{
	/* Refresh display */
	// TODO two Disbuff?
	Disbuff.setTextColor(TFT_WHITE);
	Disbuff.setTextSize(1);

	switch (new_page) {
		case PAGE_TEMPERATURE:
			M5.Lcd.setRotation(0);
			Disbuff.fillRect(0, 0, TFT_WIDTH, TFT_HEIGHT, TFT_BLACK);

#ifdef DEBUG_MODE
			Disbuff.setCursor(10, 20);
			Disbuff.print(PageStr[new_page]);
#endif
			break;

		case PAGE_CLOCK:
			M5.Lcd.setRotation(1);
			Disbuff.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_BLACK);
#ifdef DEBUG_MODE
			Disbuff.setCursor(40, 20);
			Disbuff.print(PageStr[new_page]);
#endif
			break;
		
		case PAGE_SET_ALARM:
			M5.Lcd.setRotation(2);
			Disbuff.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_BLACK);
#ifdef DEBUG_MODE
			Disbuff.setCursor(20, 0);
			Disbuff.print(PageStr[new_page]);
#endif
			break;
		
		case PAGE_COUNTDOWN:
			M5.Lcd.setRotation(3);
			Disbuff.fillRect(0, 0, TFT_HEIGHT, TFT_WIDTH, TFT_BLACK);
#ifdef DEBUG_MODE
			Disbuff.setCursor(20, 0);
			Disbuff.print(PageStr[new_page]);
#endif
			user_countdown.PageChangeDisplay();
			break;
		
		default: break;
	}
	
	Disbuff.pushSprite(0, 0);
	Serial.println(PageStr[new_page]);
}

void ButtonsUpdate(void *arg)
{
	uint8_t user_mins = 0;
	uint8_t user_secs = 0;
	
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
				if (user_countdown.isWorking()) {
					/* Buttons judgement in working */
				}
				else {
					/* Buttons judgement in idle */
					if (M5.BtnA.wasReleased()) {
						/* Short press of BtnA for +1 second */
						if (user_secs == 59) {
							user_secs = 0;
						}
						user_secs++;
					}
					else if (M5.BtnA.wasReleasefor(500)) {
						/* Long press of BtnA for +10 seconds */
						if (user_secs >= 50) {
							user_secs = 0;
						}
						user_secs += 10;
					}
					else if (M5.BtnB.wasReleased()) {
						/* Short press of BtnB for +1 minute */
						if (user_mins == 10) {
							user_mins = 0;
						}
						user_mins++;
					}
					else if (M5.BtnB.wasReleasefor(500)) {
						/* Long press of BtnB for start */
						user_countdown.Begin(user_mins, user_secs);
					}

					user_countdown.StaticDisplay(user_mins, user_secs);
				}
				
				break;
			
			default: break;
		}

		delay(50);
    }
}