#ifndef __UTILS_H__
#define __UTILS_H__

#include <M5StickCPlus.h>
#include "countdown.h"
#include "hw_timer.h"
#include "ntp_clock.h"
#include "user_system.h"

#define DEBUG_MODE

#define RETRY_TIMES (3)
#ifdef RETRY_TIMES
#define NTP_UPDATE_RETRY_TIMES RETRY_TIMES
#else
#define NTP_UPDATE_RETRY_TIMES (3)
#endif

#define INITIAL_PAGE_SELF_ADAPTION
#define INITIAL_DEFAULT_PAGE PAGE_CLOCK

#define TFT_LANDSCAPE_WIDTH TFT_HEIGHT
#define TFT_LANDSCAPE_HEIGHT TFT_WIDTH
#define TFT_VERTICAL_WIDTH TFT_WIDTH
#define TFT_VERTICAL_HEIGHT TFT_HEIGHT

// const char *ssid = "cfpzr";
// const char *password = "20000804";
// const char *ntpServer = "time1.aliyun.com";  // Set the connect NTP server.
#define _SSID "cfpzr"
#define _PASSWORD "20000804"

inline SysPage_e IMUJudge(float accX, float accY, float accZ)
{
    if (1 - accX < 0.1) {
        /* accX approx 1 */
        return PAGE_CLOCK;
    }
    else if (1 + accX < 0.1) {
        /* accX approx -1 */
        return PAGE_COUNTDOWN;
    }
    else if (1 - accY < 0.1) {
        /* accY approx 1 */
        return PAGE_TEMPERATURE;
    }
    else if (1 + accY < 0.1) {
        /* accY approx -1 */
        return PAGE_SET_ALARM;
    }

    return PAGE_UNKNOWN;
}

#endif /* __UTILS_H__ */