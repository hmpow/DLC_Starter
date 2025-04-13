#ifndef DLC_RTC_H
#define DLC_RTC_H

#include <Arduino.h>
#include <RTC.h>
#include "jpdlc_typedef.h"

#define DLC_RTC_DEBUG

void setupRTC(void);
bool isEfectiveLicenseCard(JPDLC_EXPIRATION_DATA,uint8_t);
void updateRtcSetting(uint16_t, uint8_t, uint8_t, uint8_t, uint8_t, uint8_t);

void debugPrintCurrentRtcTime(void);
void debugPrintRtcTime(RTCTime);

#endif // DLC_RTC_H