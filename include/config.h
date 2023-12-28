#ifndef __CONFIG_H__
#define __CONFIG_H__

/* Definitions of debug mode */
#define DEBUG_MODE
#ifdef DEBUG_MODE
#define DEBUG_POWER_DISPLAY
#endif /* DEBUG_MODE */

/* Definitions of error handling */
#define MAX_N_RETRY (3)
#ifdef MAX_N_RETRY
#define NTP_UPDATE_MAX_RETRY MAX_N_RETRY
#define QWEATHER_RETRY_MAX_RETRY MAX_N_RETRY
#else
#define NTP_UPDATE_MAX_RETRY (3)
#define QWEATHER_RETRY_MAX_RETRY (3)
#endif /* MAX_N_RETRY */

#define WIFI_CONNECTION_TIMEOUT (10) /* Unit: second */
#define HTTP_CONNECTION_TIMEOUT (10)

/* Definitions of system */
#define SYSTEM_INITIAL_PAGE_SELF_ADAPTATION
#define SYSTEM_DEFAULT_PAGE PAGE_NTPCLOCK
#define IMU_JUDGE_THRESHOLD (0.1)
// #define WIFI_SSID 					"CMCC-i5cg"
// #define WIFI_PASSWORD 				"f9s9f3tj"
#define WIFI_SSID "APGroup"
#define WIFI_PASSWORD "asdf1234"

#define TFT_LANDSCAPE_WIDTH TFT_HEIGHT
#define TFT_LANDSCAPE_HEIGHT TFT_WIDTH
#define TFT_VERTICAL_WIDTH TFT_WIDTH
#define TFT_VERTICAL_HEIGHT TFT_HEIGHT

/* Definitions of NTP clock */
#define NTP_REGULAR_SYNC /* NTP clock synchronization regularly */
#ifdef NTP_REGULAR_SYNC
#define NTP_CALIBRATION_INTERVAL (60) /* Unit: minute */
#endif                                /* NTP_REGULAR_SYNC */

#define QWEATHER_RELUGAR_SYNC /* QWeather synchronization regularly */
#ifdef QWEATHER_RELUGAR_SYNC
#define QWEATHER_SYNC_INTERVAL (120) /* Unit: minute */
#endif                               /* QWEATHER_RELUGAR_SYNC */

/* Difinitions of countdown */
#define RESET_TO_THE_SET_TIME /* When countdown completed, reset the countdown to the set time */
#define COUNTDOWN_TIMER_DEFAULT_MIN (0)
#define COUNTDOWN_TIMER_DEFAULT_SEC (0)

/* Difinitions of QWeather */
#define QWEATHER_HTTP_TIMEOUT HTTP_CONNECTION_TIMEOUT
#define QWEATHER_API_KEY "dca23824f42248bb9e01ff278b463e40" /* API Key */
#define QWEATHER_LOCATION_ID "101010100"                    /* Beijing */

#endif /* __CONFIG_H__ */