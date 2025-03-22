# DLC_Starter
"Driver's License Checker Starter"

Use Arduino to check the expiration date of a Japanese driver's license and start the engine.

Arduino を用いて、日本の運転免許証の有効期限を確認して、有効な免許である場合のみエンジン始動可能にします。

従来免許証、マイナ免許証の両方に対応しています。

WiFi 機能を生かし、設定はスマホからワイヤレスで行います。

## 基本操作

### 通常モード



### 設定モード

スマホを機内モードに入れてから　Arduino Uno に接続しないと設定画面に接続できません。

モバイル回線側が接続されていると、~~WAN 側を探しに行ってページが見つからないエラーになるか~~ (上位2バイトをローカルIP割り当て範囲の 192.168 に固定したことで解消？)、検索キーワードとして扱われてIPアドレスに関する検索結果が出てきます。



## ソフトウェア

### 注 使用ライブラリのライセンスについて

DLC starter 自体は MIT ライセンスで公開していますが、ビルドするとLGPLライセンスのライブラリが静的リンクされます。

このリポジトリでJGPLライセンスのライブラリの "再頒布" はしていません。各々の開発環境で Arduino からダウンロードされ各PC内でリンクされます。

https://github.com/arduino/ArduinoCore-renesas/

うち LGPLのライブラリ(DLC starter から include している階層であり、ライブラリ内の芋づる式includeは未確認です)

+ SPI
+ EEPROM
+ WiFi3 

### 動作確認済み開発環境

OS : Windows 11 Pro 23H2/24H2

Visual Studio Code : 1.98 (Japanese Language Pack 有効)

IDE : PlatformIO IDE 3.3.4 (VScode 拡張機能)

※ ハードウェアは Arduino を使っていますが PlatformIO 前提のコードになっており Arduino IDE ではビルドできません

※ Clean してから Build し直さないと WiFi が繋がらなくなるようです

### 設定ファイル arduino_secrets.h

arduino_secrets_template.h を arduino_secrets.h にリネームして使用します。


| 種別 | 定数名 | 用途 | 設定値 | 不要な場合 |
| :--- | :--- | :--- | :--- | :--- |
| 必須 | SECRET_SSID| 設定画面アクセスポイントSSID | 英数字:32文字まで | － |
| オプション | SECRET_PASS | 設定画面アクセスポイントWPAパスワード | 英数字:8~63文字 | コメントアウト or 削除 or ""を定義 |
| 必須 | SECRET_SECURITY_NO | 暗証番号設定画面用<br/>いたずら防止用暗証番号 |　数字:0~9999 | － |
| オプション | SECRET_IP_ADDR_UPPER<br/>SECRET_IP_ADDR_LOWER | 設定画面IPアドレス |　数字:1～254 | 削除 or コメントアウト |
 
 ※SECRET_IP_ADDR_UPPER と SECRET_IP_ADDR_LOWER は、片方のみの設定はできません(片方のみ定義の場合は未設定扱いになります)

 ※SECRET_IP_ADDR_UPPER と SECRET_IP_ADDR_LOWER 未設定の場合は、Arduino WiFi ライブラリのデフォルト値となります

### ディレクトリ構造




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

