// arduino_secrets.hにリネームして使用

#ifndef ARDUINO_SECRETS_H
#define ARDUINO_SECRETS_H

//【必須】設定画面アクセスポイントSSID
// 英数字:32文字まで

#define SECRET_SSID "DLC_SETTING"

//【オプション】設定画面アクセスポイントパスワード
// 英数字:8~63文字
// 不要な場合:削除かコメントアウト
// 未設定時の動作:パスワードなし

#define SECRET_PASS "abcd12345"

//【必須】暗証番号設定画面用いたずら防止用暗証番号
// 数字:0～9999

#define SECRET_SECURITY_NO 1234

// 設定画面IPアドレス
// 数字:1～254
// 不要な場合:削除かコメントアウト
// 未設定時の動作:Arduino WiFi ライブラリのデフォルト値
// UPPER,LOWER の片方のみの設定はできません(片方のみ定義の場合は未設定扱いになります)

//192.168.SECRET_IP_ADDR_UPPER.SECRET_IP_ADDR_LOWER になります

#define SECRET_IP_ADDR_UPPER 10
#define SECRET_IP_ADDR_LOWER 1


#endif // ARDUINO_SECRETS_H
