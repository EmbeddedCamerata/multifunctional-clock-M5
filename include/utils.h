#ifndef __UTILS_H__
#define __UTILS_H__

#include <M5StickCPlus.h>
#include "alarm.h"
#include "countdown.h"
#include "hw_timer.h"
#include "m5_qweather.h"
#include "ntp_clock.h"
#include "sys_main.h"

/* Definitions of debug mode */
#define DEBUG_MODE
#ifdef DEBUG_MODE
#define POWER_DISPLAY
#endif /* DEBUG_MODE */

/* Definitions of error handling */
#define MAX_RETRY 					(3)
#ifdef MAX_RETRY
#define NTP_UPDATE_MAX_RETRY  		MAX_RETRY
#define QWEATHER_RETRY_MAX_RETRY    MAX_RETRY
#else
#define NTP_UPDATE_MAX_RETRY  		(3)
#define QWEATHER_RETRY_MAX_RETRY    (3)
#endif /* MAX_RETRY */

#define WIFI_CONNECTION_TIMEOUT		(10)	/* Unit: second */
#define HTTP_CONNECTION_TIMEOUT		(10)

/* Definitions of system */
#define SYSTEM_INITIAL_PAGE_SELF_ADAPTION
#define SYSTEM_DEFAULT_PAGE			PAGE_NTPCLOCK
#define IMU_JUDGE_THRESHOLD			(0.1)
#define WIFI_SMARTCONFIG_ENABLE				/* Enable WIFI smart config via ESPTouch app before using ssid and password */
#define WIFI_SSID 					"E2524 Pro Max"
#define WIFI_PASSWORD 				"25242524"

#define TFT_LANDSCAPE_WIDTH     	TFT_HEIGHT
#define TFT_LANDSCAPE_HEIGHT    	TFT_WIDTH
#define TFT_VERTICAL_WIDTH      	TFT_WIDTH
#define TFT_VERTICAL_HEIGHT     	TFT_HEIGHT

/* Definitions of NTP clock */
#define NTP_REGULAR_SYNC                	/* NTP clock synchronization regularly */
#ifdef NTP_REGULAR_SYNC
#define NTP_CALIBRATION_INTERVAL 	(60)	/* Unit: minute */
#endif /* NTP_REGULAR_SYNC */

#define QWEATHER_RELUGAR_SYNC				/* QWeather synchronization regularly */
#ifdef QWEATHER_RELUGAR_SYNC
#define QWEATHER_SYNC_INTERVAL		(120)	/* Unit: minute */
#endif /* QWEATHER_RELUGAR_SYNC */

/* Difinitions of countdown */
#define RESET_TO_THE_BEGINNING				/* When countdown completed, reset the countdown to the last time */
#define COUNTDOWN_TIMER_DEFAULT_MIN	(0)
#define COUNTDOWN_TIMER_DEFAULT_SEC	(0)

/* Difinitions of QWeather */
#define QWEATHER_HTTP_TIMEOUT 		HTTP_CONNECTION_TIMEOUT

#endif /* __UTILS_H__ */