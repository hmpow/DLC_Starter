//arduino_secrets.hにリネームして使用

#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H

//SSID設定
#define SECRET_SSID "DLC_SETTING"
#define SECRET_PASS ""

//いたずら防止用暗証番号
#define SECURITY_NO 1234

//IPアドレスをデフォルトから変えたい場合のみdefine (ifdefで動作)
//#define SECRET_IP_ADDR 192,48,56,2 

#endif // ARDUINO_SECRETS_H
