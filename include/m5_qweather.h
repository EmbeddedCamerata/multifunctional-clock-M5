#ifndef __M5_QWEATHER_H__
#define __M5_QWEATHER_H__

#include "sys_main.h"
#include "config.h"

typedef enum
{
    MAIN_PAGE_CHANGED = 0U,
    SUB_PAGE_CHANGED
} PageChangedType_e;

typedef enum
{
    SUBPAGE_CURRENT_WEATHER = 0U,
    SUBPAGE_CURRENT_AIR_QUALITY
} WeatherSubPageType_e;

typedef enum
{
    URL_CURRENT_WEATHER = 0U,
    URL_CURRENT_AIR_QUALITY
} QWeatherUrlType_e;

typedef struct QCurWeather_Attr
{
    String updateTime;
    int temp;
    int icon;
    String text;
    int humidity;
} QCurWeather_TypeDef;

typedef struct QCurAirQuality_Attr
{
    String updateTime;
    int aqi;
    int level;
    String category;
    String primary;
    int pm10;
    int pm2p5;
} QCurAirQuality_TypeDef;

class QWeather
{
public:
    QWeather();

    void Init(SysTypeDef *SysAttr);
    void ButtonsUpdate(SysTypeDef *SysAttr);
    void OnMyPage();
    void Leave();

    bool GetCurWeather(EventGroupHandle_t *Events_ptr);
    bool GetCurAirQuality(EventGroupHandle_t *Events_ptr);

    void CurWeatherUpdate(EventGroupHandle_t *Events_ptr);
    void CurAirQualityUpdate(EventGroupHandle_t *Events_ptr);

    void DisplaySubPage();

    inline void Inited() { this->isInited = true; };
    inline bool IsOnMyPage() { return this->isOnMyPage; };

private:
    bool isInited;
    bool isOnMyPage;
    WeatherSubPageType_e SubPage;
    QCurWeather_TypeDef CurWeatherData;
    QCurAirQuality_TypeDef CurAirQualityData;

    String ApiKey = QWEATHER_API_KEY;
    String LocationID = QWEATHER_LOCATION_ID;
    String Language = "en"; /* Or zh */

private:
    void TFTRecreate();
    void DisplayCurWeather();
    void DisplayCurAirQuality();

    bool ParseRequest(String Url, QWeatherUrlType_e UrlType);
    bool ParseCurWeather(String Payload);
    bool ParseCurAirQuality(String Payload);
};

void QWeatherInitTask(void *arg);

extern QWeather User_QWeather;

#endif /* __M5_QWEATHER_H__ */