#ifndef MAIN_SETTING_H
#define MAIN_SETTING_H

#define RESET_OUT_PIN D7

#include <Arduino.h>
#include <WiFiS3.h>
#include <RTC.h>
#include <EEPROM.h>

#include "Arduino_LED_Matrix.h"

// #include <WiFiServer.h> //WiFiS3.h → WiFi.h → WiFiServer.h でインクルードされるめ不要

#include "arduino_secrets.h"
#include "main_setting_html.h"
#include "ATP301x_Arduino_SPI.h"

#include "pinEEPROM.h"

/* 通常モードと共用のプロトタイプ宣言や数 */
/* extern 付き宣言してメイン側に置いたものをリンクしてもらう */

extern void printRTCtime(void);
extern void setupRTC(void);

extern ATP301x_ARDUINO_SPI atp301x;
extern PinEEPROM pinEEPROM;
extern const bool USE_ATP301X;
extern char atpbuf[];
extern const uint8_t DRIVER_LIST_NUM;


/* 設定特化のプロトタイプ宣言や数 */

void main_settingMode_setup(void);
void main_settingMode_loop(void);

void sendHTML(WiFiClient, String);
void send404(WiFiClient);
void printWiFiStatus(void);

bool verifySecurityNo(uint16_t);

void announceWifiModuleNotFound();
void announceWifiInitializeFailed();
void announcePleaseConnectWiFi();
void announceEndSettingMode();



#endif // MAIN_SETTING_H