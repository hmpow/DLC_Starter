# DLC_Starter
"Driver's License Checker Starter"

Use Arduino to check the expiration date of a Japanese driver's license and start the engine.

Arduino を用いて、日本の運転免許証の有効期限を確認して、有効な免許である場合のみエンジン始動可能にします。

従来免許証、マイナ免許証の両方に対応しています。

WiFi 機能を生かし、設定はスマホからワイヤレスで行います。

## 基本操作




## ソフトウェア

### 開発環境

OS : Windows 11 23H2/24H2

IDE : Visual Studio Code

プラグイン : PlatformIO

※ ハードウエアは Arduino を使っていますが PlatformIO 前提のコードになっており Arduino IDE ではコンパイルできません

### 設定

arduino_secrets_template.h に下記を設定して arduino_secrets.h にリネームします。


| 種別 | 定数名 | 用途 | 設定値 | 不要な場合 |
| :--- | :--- | :--- | :--- | :--- |
| 必須 | SECRET_SSID| 設定画面アクセスポイントSSID | 英数字:32文字まで | － |
| オプション | SECRET_PASS | 設定画面アクセスポイントWPAパスワード | 英数字:8~63文字 | コメントアウト or 削除 or ""を定義 |
| 必須 | SECRET_SECURITY_NO | 暗証番号設定画面用<br/>いたずら防止用暗証番号 |　数字:0~9999 | － |
| オプション | SECRET_IP_ADDR_UPPER<br/>SECRET_IP_ADDR_LOWER | 設定画面IPアドレス |　数字:1～254 | 削除 or コメントアウト |
 
 ※SECRET_IP_ADDR_UPPER と SECRET_IP_ADDR_LOWER は、片方のみの設定はできません(片方のみ定義の場合は未設定扱いになります)

 ※SECRET_IP_ADDR_UPPER と SECRET_IP_ADDR_LOWER 未設定の場合は、Arduino WiFi ライブラリのデフォルト値となります


### カスタマイズ

#### カードリーダーを変える

JpDrvLicNfcCommand\jpdlc_base_reader_if がラッパー層となっています。

#### 音声合成LSIを使用しない



src\ATP301x_SPIディレクトリを削除



### ハードウェア

マイコンボード : Arduino Uno R4 WiFi

NFCカードリーダ : SONY RC-S/660S

音声合成LSI(オプション) : AQUEST AquesTalk pico ATP301x シリーズ

その他 : 電磁リレー、トランジスタ、抵抗など

### 回路例

スタータモーターのC端子回路かスタータリレー回路へ追加のリレーを割り込ませて制御する、最もスタンダードな構成例です

電磁リレーはノーマリークローズ接点を使用し、故障時は電源を切ればエンジンがかかるようになります

※B端子回路に割り込ませると燃えます



### 

## 注

スマホは機内モードに入れてから　Arduino Uno に接続しないと、WAN 側を探しに行ってページが見つからないエラーになります。

Clean してから Build し直さないと WiFi が繋がらなくなるようです。

arduino_secrets_template.h に好きな SSID と いたずら防止用暗証番号 を設定し、 arduino_secrets.h にリネームします。