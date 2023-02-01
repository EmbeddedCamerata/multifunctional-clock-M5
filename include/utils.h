#ifndef __UTILS_H__
#define __UTILS_H__

#include <M5StickCPlus.h>
#include "countdown.h"
#include "user_system.h"

#define DEBUG_MODE

inline void led_heartbeat() {
    digitalWrite(M5_LED, 1 - digitalRead(M5_LED));
};

#endif /* __UTILS_H__ */