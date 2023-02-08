#ifndef __M5_QWEATHER_H__
#define __M5_QWEATHER_H__

#include "user_system.h"

typedef enum QWeatherUrl
{
	URL_CURRENT_WEATHER,
	URL_CURRENT_AIR_POLLUTION
} QWeatherUrlType;

typedef struct QCurWeather_Attr
{
	String updateTime;
	int temp;
	int icon;
	String text;
	int humidity;
} QCurWeather_TypeDef, *QCurWeather_Ptr;

typedef struct QCurAirPollution_Attr
{
	String pol_updateTime;
	String level;
	String category;
	String pm10;
	String pm2p5;
} QCurAirPollution_TypeDef, *QCurAirPollution_Ptr;

class QWeather
{
public:
	QWeather();

	void Init(SysPage_e Page);
	void ButtonsUpdate();
	void OnMyPage();
    void Leave();

	void GetCurWeather();
	void GetCurAirPollution();
	void DisplayCurWeather();

	inline void Inited() { this->isInited = true; };
	inline bool IsOnMyPage() { return this->isOnMyPage; };
	
private:
	bool isInited;
    bool isOnMyPage;
	QCurWeather_TypeDef CurWeatherData;
	QCurAirPollution_TypeDef CurAirPollutionData;

	String ApiKey = "dca23824f42248bb9e01ff278b463e40";
	String LocationID = "101010100";	// Beijing
	String Language = "en";	// zh

private:
	void TFTRecreate();
	bool CurWeatherUpdate();
	bool ParseRequest(String Url, QWeatherUrlType UrlType);
	bool ParseCurWeather(String Payload);
	bool ParseCurAirPollution(String Payload);
};

void QWeatherInitTask(void *arg);

extern QWeather User_QWeather;

#endif /* __M5_QWEATHER_H__ */