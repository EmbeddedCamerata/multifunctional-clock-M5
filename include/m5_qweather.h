#ifndef __M5_QWEATHER_H__
#define __M5_QWEATHER_H__

#include "user_system.h"

typedef enum PageChanged
{
    MAIN_PAGE_CHANGED = 0U,
    SUB_PAGE_CHANGED
} PageChangedType;

typedef enum WeatherSubPage
{
    SUB_PAGE_CURRENT_WEATHER = 0U,
    SUB_PAGE_CURRENT_AIR_QUALITY
} WeatherSubPageType;

typedef enum QWeatherUrl
{
	URL_CURRENT_WEATHER = 0U,
	URL_CURRENT_AIR_QUALITY
} QWeatherUrlType;

typedef struct QCurWeather_Attr
{
	String updateTime;
	int temp;
	int icon;
	String text;
	int humidity;
} QCurWeather_TypeDef, *QCurWeather_Ptr;

typedef struct QCurAirQuality_Attr
{
	String updateTime;
	int aqi;
	int level;
	String category;
	String primary;
	int pm10;
	int pm2p5;
} QCurAirQuality_TypeDef, *QCurAirQuality_Ptr;

class QWeather
{
public:
	QWeather();

	void Init(SysPageType Page);
	void ButtonsUpdate();
	void OnMyPage();
    void Leave();

	bool GetCurWeather();
	bool GetCurAirQuality();

	void CurWeatherUpdate();
	void CurAirQualityUpdate();

	inline void Inited() { this->isInited = true; };
	inline bool IsOnMyPage() { return this->isOnMyPage; };
	
private:
	bool isInited;
    bool isOnMyPage;
	WeatherSubPageType SubPage;
	QCurWeather_TypeDef CurWeatherData;
	QCurAirQuality_TypeDef CurAirQualityData;

	String ApiKey = "dca23824f42248bb9e01ff278b463e40";	/* Dev API Key */
	String LocationID = "101010100";					/* Beijing */
	String Language = "en";								/* Or zh */

private:
	void TFTRecreate();
	void DisplaySubPage();
	void DisplayCurWeather();
	void DisplayCurAirQuality();

	bool ParseRequest(String Url, QWeatherUrlType UrlType);
	bool ParseCurWeather(String Payload);
	bool ParseCurAirQuality(String Payload);
};

void QWeatherInitTask(void *arg);

extern QWeather User_QWeather;

#endif /* __M5_QWEATHER_H__ */