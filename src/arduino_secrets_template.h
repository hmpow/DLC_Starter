//arduino_secrets.hにリネームして使用

#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H

//SSID設定
#define SECRET_SSID "DLC_SETTING"
#define SECRET_PASS ""

//いたずら防止用暗証番号
#define SECRET_SECURITY_NO 1234

//IPアドレスをデフォルトから変えたい場合のみdefine (ifdefで動作)
//192.168.SECRET_IP_ADDR_UPPER.SECRET_IP_ADDR_LOWER
#define SECRET_IP_ADDR_UPPER 10
#define SECRET_IP_ADDR_LOWER 1


#endif // ARDUINO_SECRETS_H
