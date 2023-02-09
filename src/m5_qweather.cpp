#include "../include/utils.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

extern System_TypeDef UserSystem;
extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;

TaskHandle_t xhandle_cur_weather_update = NULL;
TaskHandle_t xhandle_cur_air_quality_update = NULL;

void CurWeatherUpdateTask(void *arg);
void CurAirQualityUpdateTask(void *arg);

QWeather::QWeather() :                  \
    SubPage(SUB_PAGE_CURRENT_WEATHER),  \
    isInited(0),                        \
    isOnMyPage(0) {};

void QWeather::Init(SysPage_e Page)
{
    if (this->isInited) {
        return;
    }

    this->GetCurWeather();

    if (Page == PAGE_WEATHER) {
        this->isOnMyPage = true;
        this->TFTRecreate();
        this->DisplayCurWeather();
    }

    this->isInited = true;
}

void QWeather::ButtonsUpdate()
{
    if (M5.BtnA.wasReleased()) {
        /* Short press of BtnA for switching the sub page */
        this->SubPage = (WeatherSubPage_e)(1 - (int)this->SubPage);
    }
    else if (M5.BtnA.wasReleasefor(500)) {
        /* Long press of BtnA for updating current weather & air quality immediately */
        xTaskCreate(CurWeatherUpdateTask, "CurWeatherUpdateTask", \
            1024*4, (void*)0, 6, &xhandle_cur_weather_update);
        
        xTaskCreate(CurAirQualityUpdateTask, "CurAirQualityUpdateTask", \
            1024*4, (void*)0, 6, &xhandle_cur_air_quality_update);
    }
    else if (M5.BtnB.wasReleased()) {
        
    }
}

void QWeather::OnMyPage()
{
    this->isOnMyPage = true;
    this->TFTRecreate();
    this->DisplayCurWeather();
}

void QWeather::Leave()
{
    this->isOnMyPage = false;
}

bool QWeather::GetCurWeather()
{
    int _try_time = 0;
    String url = "https://devapi.qweather.com/v7/weather/now?location=" + this->LocationID + \
        "&lang=" + this->Language + "&key=" + this->ApiKey + "&gzip=n";

    if (!this->ParseRequest(url, URL_CURRENT_WEATHER)) {
        do {
            _try_time++;
#ifdef DEBUG_MODE
            Serial.printf("Retries: %d/%d\n", _try_time, QWEATHER_RETRY_MAX_RETRY);
#endif
            if (_try_time == QWEATHER_RETRY_MAX_RETRY) {
#ifdef DEBUG_MODE
                Serial.println("Get current weather failed!");
#endif
                return false;
            }
        } while (!this->ParseRequest(url, URL_CURRENT_WEATHER));
    }

    return true;
}

bool QWeather::GetCurAirQuality()
{
    int _try_time = 0;
    String url = "https://devapi.qweather.com/v7/air/now?location=" + this->LocationID + \
        "&lang=" + this->Language + "&key=" + this->ApiKey + "&gzip=n";
    
    if (!this->ParseRequest(url, URL_CURRENT_AIR_QUALITY)) {
        do {
            _try_time++;
#ifdef DEBUG_MODE
            Serial.printf("Retries: %d/%d\n", _try_time, QWEATHER_RETRY_MAX_RETRY);
#endif
            if (_try_time == QWEATHER_RETRY_MAX_RETRY) {
#ifdef DEBUG_MODE
                Serial.println("Get current air quality failed!");
#endif
                return false;
            }
        } while (!this->ParseRequest(url, URL_CURRENT_AIR_QUALITY));
    }

    return true;
}

void QWeather::DisplayCurWeather()
{
    xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
    //Disbuff.setTextDatum(ML_DATUM);
    Disbuff.setTextSize(4);
    Disbuff.setTextColor(TFT_RED);

    /* Draw the temperature */
    if (this->CurWeatherData.temp >= 10) {
        Disbuff.fillRect(
            TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99")/2,   \
            40 - Disbuff.fontHeight()/2,                        \
            Disbuff.textWidth("99"), Disbuff.fontHeight(),      \
            TFT_BLACK
        );
        Disbuff.setCursor(
            TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99")/2, \
            40 - Disbuff.fontHeight()/2
        );
        Disbuff.printf("%02d", this->CurWeatherData.temp);
    }
    else if (0 < this->CurWeatherData.temp < 10) {
        Disbuff.fillRect(
            TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("9")/2,    \
            40 - Disbuff.fontHeight()/2,                        \
            Disbuff.textWidth("9"), Disbuff.fontHeight(),       \
            TFT_BLACK
        );
        Disbuff.setCursor(
            TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("9")/2,    \
            40 - Disbuff.fontHeight()/2
        );
        Disbuff.printf("%d", this->CurWeatherData.temp);

        /* Draw the "oc" text besides the number */
        //Disbuff.setTextDatum(TL_DATUM);
        Disbuff.fillRect(
            TFT_VERTICAL_WIDTH/2 + Disbuff.textWidth("9")/2,    \
            80 - Disbuff.fontHeight(4),                         \
            Disbuff.textWidth("o"), Disbuff.fontHeight(2),      \
            TFT_BLACK
        );
        Disbuff.setCursor(
            TFT_VERTICAL_WIDTH/2 + Disbuff.textWidth("9")/2,    \
            80 - Disbuff.fontHeight(4)
        );
        Disbuff.setTextColor(TFT_WHITE);
        Disbuff.setTextSize(2);
        Disbuff.print("o");
    }
    else {
        // TODO -9/-10
        Disbuff.fillRect(
            TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("-9")/2, 40,    \
            Disbuff.textWidth("-9"), Disbuff.fontHeight(),           \
            TFT_BLACK
        );
        Disbuff.setCursor(TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("-9")/2, 40);
        Disbuff.printf("-%d", this->CurWeatherData.temp);
    }

    Disbuff.setTextSize(4);
    Disbuff.setTextColor(TFT_RED);
	Disbuff.fillRect(TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99%")/2, 120, Disbuff.textWidth("99%"), Disbuff.fontHeight(), TFT_BLACK);
    Disbuff.setCursor(TFT_VERTICAL_WIDTH/2 - Disbuff.textWidth("99%")/2, 120);
    Disbuff.printf("%d%%", this->CurWeatherData.humidity);
    Disbuff.pushSprite(0, 0);

    xSemaphoreGive(lcd_draw_sem);
}

void QWeather::DisplayCurAirQuality()
{
    
}

void QWeather::TFTRecreate()
{
    M5.Lcd.setRotation(PAGE_WEATHER);

    xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
    if (Disbuff.width() != TFT_VERTICAL_WIDTH) {
        Disbuff.deleteSprite();
		Disbuff.createSprite(TFT_VERTICAL_WIDTH, TFT_VERTICAL_HEIGHT);
    }

    Disbuff.fillRect(0, 0, TFT_VERTICAL_WIDTH, TFT_VERTICAL_HEIGHT, TFT_BLACK);
    Disbuff.pushSprite(0, 0);
    xSemaphoreGive(lcd_draw_sem);
}

void QWeather::CurWeatherUpdate()
{
    this->GetCurWeather();
    
    if (this->isOnMyPage and this->SubPage == SUB_PAGE_CURRENT_WEATHER) {
        this->DisplayCurWeather();
    }
}

void QWeather::CurAirQualityUpdate()
{
    this->GetCurAirQuality();

    if (this->isOnMyPage and this->SubPage == SUB_PAGE_CURRENT_AIR_QUALITY) {
        this->DisplayCurAirQuality();
    }
}

bool QWeather::ParseRequest(String Url, QWeatherUrlType UrlType)
{
    String payload;
    HTTPClient http;
    int httpCode;
    uint32_t dt;
    bool ret;

    if (!http.begin(Url)) {
        Serial.println("Connected to Qweather failed!");
    	return false;
    }

    dt = millis();
    httpCode = http.sendRequest("GET");

    while (http.connected()) {
        if (httpCode > 0) {
            if (httpCode == HTTP_CODE_OK) {
                payload = http.getString();
#ifdef DEBUG_MODE
                Serial.println(payload);
#endif
                break;
            }
        }
        else {
            Serial.println("HTTP get failed!");
        }
        
        if ((millis() - dt) > QWEATHER_HTTP_TIMEOUT * 1000UL) {
            Serial.println("HTTP header timeout!");
            http.end();
            return false;
        }
    }

    if (UrlType == URL_CURRENT_WEATHER) {
        ret = this->ParseCurWeather(payload);
    }
    else if (UrlType == URL_CURRENT_AIR_QUALITY) {
        ret = this->ParseCurAirQuality(payload);
    }

    http.end();
    
    return ret;
}

bool QWeather::ParseCurWeather(String Payload)
{
	StaticJsonDocument<128> filter;
	filter["code"] = true;
	filter["updateTime"] = true;

	JsonObject filter_now = filter.createNestedObject("now");
	filter_now["temp"] = true;
	filter_now["icon"] = true;
	filter_now["text"] = true;
	filter_now["humidity"] = true;

	StaticJsonDocument<256> doc;

	DeserializationError error = deserializeJson(doc, Payload, DeserializationOption::Filter(filter));

	if (error) {
		Serial.print("deserializeJson() failed: ");
		Serial.println(error.c_str());
		return false;
	}

	if (doc["code"].as<int>() != 200) {
		Serial.println("Current weather request failed!");
		return false;
	}

	JsonObject now = doc["now"];

	this->CurWeatherData.updateTime = doc["updateTime"].as<String>();
	this->CurWeatherData.temp = now["temp"].as<int>();
	this->CurWeatherData.icon = now["icon"].as<int>();
	this->CurWeatherData.text = now["text"].as<String>();
	this->CurWeatherData.humidity = now["humidity"].as<int>();

	return true;
}

bool QWeather::ParseCurAirQuality(String Payload)
{
    StaticJsonDocument<128> filter;
	filter["code"] = true;
	filter["updateTime"] = true;

	JsonObject filter_now = filter.createNestedObject("now");
	filter_now["aqi"] = true;
	filter_now["level"] = true;
	filter_now["category"] = true;
	filter_now["primary"] = true;

	StaticJsonDocument<256> doc;

	DeserializationError error = deserializeJson(doc, Payload, DeserializationOption::Filter(filter));

	if (error) {
		Serial.print("deserializeJson() failed: ");
		Serial.println(error.c_str());
		return false;
	}

	if (doc["code"].as<int>() != 200) {
		Serial.println("Current weather request failed!");
		return false;
	}

	JsonObject now = doc["now"];

	this->CurAirQualityData.updateTime = doc["updateTime"].as<String>();
	this->CurAirQualityData.aqi = now["aqi"].as<int>();
	this->CurAirQualityData.level = now["level"].as<int>();
	this->CurAirQualityData.category = now["category"].as<String>();
    this->CurAirQualityData.primary = now["primary"].as<String>();
    this->CurAirQualityData.pm10 = now["pm10"].as<int>();
    this->CurAirQualityData.pm2p5 = now["pm2p5"].as<int>();

	return true;
}

void QWeatherInitTask(void *arg)
{	
    xEventGroupWaitBits(
        UserSystem.SysEvents,                                   \
        EVENT_WIFI_CONNECTED_FLAG | EVENT_NTP_INITIAL_OK_FLAG,  \
        pdTRUE, pdTRUE, portMAX_DELAY
    );
    User_QWeather.Init(UserSystem.SysPage);
    vTaskDelete(NULL);
}

void CurWeatherUpdateTask(void *arg)
{
    User_QWeather.CurWeatherUpdate();
    vTaskDelete(NULL);
}

void CurAirQualityUpdateTask(void *arg)
{
    User_QWeather.CurAirQualityUpdate();
    vTaskDelete(NULL);
}

QWeather User_QWeather;
