#include "../include/utils.h"
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

extern System_TypeDef UserSystem;
extern TFT_eSprite Disbuff;
extern SemaphoreHandle_t lcd_draw_sem;

QWeather::QWeather() :  \
    isInited(0),        \
    isOnMyPage(0) {};

void QWeather::Init(SysPage_e Page)
{
    if (this->isInited) {
        return;
    }

    this->GetCurrent(this->LocationID, this->Language);

    if (Page == PAGE_WEATHER) {
        this->isOnMyPage = true;
        this->TFTRecreate();
    }

    this->isInited = true;
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

void QWeather::GetCurrent(String LocationID, String Language)
{
    bool ret;
    String url = "https://devapi.qweather.com/v7/weather/now?location=" + LocationID + \
        "&lang=" + Language + "&key=" + this->ApiKey + "&gzip=n";

    ret = this->ParseRequest(url, URL_CURRENT_WEATHER);

}

void QWeather::DisplayCurWeather()
{

}

void QWeather::TFTRecreate()
{
    M5.Lcd.setRotation(PAGE_WEATHER);

    xSemaphoreTake(lcd_draw_sem, portMAX_DELAY);
    if (Disbuff.width() != TFT_LANDSCAPE_HEIGHT) {
        Disbuff.deleteSprite();
		Disbuff.createSprite(TFT_LANDSCAPE_HEIGHT, TFT_LANDSCAPE_WIDTH);
    }

    Disbuff.fillRect(0, 0, TFT_LANDSCAPE_HEIGHT, TFT_LANDSCAPE_WIDTH, TFT_BLACK);
    Disbuff.pushSprite(0, 0);
    xSemaphoreGive(lcd_draw_sem);
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
    
    httpCode = http.GET();

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
        
        if ((millis() - dt) > 10*1000UL) {
            Serial.println("HTTP header timeout!");
            http.end();
            return false;
        }
    }

    if (UrlType == URL_CURRENT_WEATHER) {
        ret = this->ParseCurWeather(payload);
    }

    http.end();
    
    return ret;
}

bool QWeather::ParseCurWeather(String Payload)
{
	// String input;
	StaticJsonDocument<128> filter;
	// filter["code"] = true;
	// filter["updateTime"] = true;

	// JsonObject filter_now = filter.createNestedObject("now");
	// filter_now["temp"] = true;
	// filter_now["icon"] = true;
	// filter_now["text"] = true;
	// filter_now["windScale"] = true;
	// filter_now["humidity"] = true;

	// StaticJsonDocument<256> doc;

	// DeserializationError error = deserializeJson(doc, Payload, DeserializationOption::Filter(filter));

	// if (error) 
	// {
	// 	Serial.print("deserializeJson() failed: ");
	// 	Serial.println(error.c_str());
	// 	return false;
	// }

	// const char* code = doc["code"]; // "200"
	// const char* updateTime = doc["updateTime"]; // "2021-12-16T11:57+08:00"

	// if (strcmp(code, "200") != 0)
	// {
	// 	Serial.println("Current weather request failed!");
	// 	return false;
	// }

	// JsonObject now = doc["now"];
	// const char* now_temp = now["temp"]; // "3"
	// const char* now_icon = now["icon"]; // "100"
	// const char* now_text = now["text"]; // "Sunny"
	// const char* now_windScale = now["windScale"]; // "5"
	// const char* now_humidity = now["humidity"]; // "20"

	// current -> wea_updateTime = updateTime;
	// current -> temp = now_temp;
	// current -> icon = now_icon;
	// current -> text = now_text;
	// current -> windscale = now_windScale;
	// current -> humidity = now_humidity;
	return true;
}

void QWeatherInitTask(void *arg)
{	
    xEventGroupWaitBits(UserSystem.SysEvents, EVENT_WIFI_CONNECTED_FLAG | EVENT_NTP_INITIAL_OK, pdTRUE, pdTRUE, portMAX_DELAY);
    User_QWeather.Init(UserSystem.SysPage);
    vTaskDelete(NULL);
}

QWeather User_QWeather;
