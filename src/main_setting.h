#ifndef MAIN_SETTING_H
#define MAIN_SETTING_H

#define RESET_OUT_PIN D7

#include <Arduino.h>
#include <WiFiS3.h>
#include <RTC.h>

// #include <WiFiServer.h> //WiFiS3.h → WiFi.h → WiFiServer.h でインクルードされるめ不要

#include "arduino_secrets.h"
#include "main_setting_html.h"
#include "ATP301x_Arduino_SPI.h"

/* 通常モードと共用のプロトタイプ宣言や数 */
/* extern 付き宣言してメイン側に置いたものをリンクしてもらう */

extern void printRTCtime(void);
extern void setupRTC(void);

extern ATP301x_ARDUINO_SPI atp301x;
extern const bool USE_ATP301X;
extern char atpbuf[];

/* 設定特化のプロトタイプ宣言や数 */

void main_settingMode_setup(void);
void main_settingMode_loop(void);

void sendHTML(WiFiClient, String);
void send404(WiFiClient);
void printWiFiStatus(void);

void announceWifiModuleNotFound();
void announceWifiInitializeFailed();
void announcePleaseConnectWiFi();

#endif // MAIN_SETTING_H