#ifndef __M5_QWEATHER_H__
#define __M5_QWEATHER_H__

#include "user_system.h"
#include <Arduino.h>

typedef enum QWeatherUrl
{
	URL_CURRENT_WEATHER,
	URL_CURRENT_POLLUTION
} QWeatherUrlType;

typedef struct QCurWeather_Attr
{
	String wea_updateTime;
	String temp;
	String icon;
	String text;
	String windscale;
	String humidity;
} QCurWeather_TypeDef, *QCurWeather_Ptr;

class QWeather
{
public:
	QWeather();

	void Init(SysPage_e Page);
	void OnMyPage();
    void Leave();

	void GetCurrent(String LocationID, String Language);
	void DisplayCurWeather();

	inline void Inited() { this->isInited = true; };
	inline bool IsOnMyPage() { return this->isOnMyPage; };
	
private:
	bool isInited;
    bool isOnMyPage;
	QCurWeather_TypeDef current;

	String ApiKey = "dca23824f42248bb9e01ff278b463e40";
	String LocationID = "101010100";	// Beijing
	String Language = "en";	// zh

private:
	void TFTRecreate();
	bool ParseRequest(String Url, QWeatherUrlType UrlType);
	bool ParseCurWeather(String Payload);
};

void QWeatherInitTask(void *arg);

extern QWeather User_QWeather;

#endif /* __M5_QWEATHER_H__ */