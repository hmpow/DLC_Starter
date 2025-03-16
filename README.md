# DLC_Starter
Use Arduino to check the expiration date of a Japanese driver's license and start the engine.

Arduino Uno R4 WiFi を用いて、日本の運転免許証の有効期限を確認して、有効な免許である場合のみエンジン始動可能にします。

従来免許証、マイナ免許証の両方に対応しています。

WiFi 機能を生かし、設定はスマホからワイヤレスで行います。

## 注

スマホは機内モードに入れてから　Arduino Uno に接続しないと、WAN 側を探しに行ってページが見つからないエラーになります。

Clean してから Build し直さないと WiFi が繋がらなくなるようです。

arduino_secrets_template.h に好きな SSID と いたずら防止用暗証番号 を設定し、 arduino_secrets.h にリネームします。