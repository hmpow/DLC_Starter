//ChatGPTの提案とArduinoのサンプルプログラムを合体してベースを作成

#include <Arduino.h>
#include <WiFiS3.h>
#include <RTC.h>

#include "arduino_secrets.h"
#include "web_page.h"

#include "ATP301x_Arduino_SPI.h"

#define executeReset() digitalWrite(D7, HIGH)

// #include <WiFiServer.h> //WiFiS3.h → WiFi.h → WiFiServer.h でインクルードされる

//要調査：wifi起動したら音声合成使用禁止　フリーズする　リソース競合？

/* プロトタイプ宣言 PlatformIO では必要 */
void blinkLED(int);
void sendHTML(WiFiClient, String);
void send404(WiFiClient);
void printWiFiStatus(void);


void printRTCtime(void);
void setupRTC(void);


/* 設定 */

const char* ssid     = SECRET_SSID;  // アクセスポイントのSSID
const char* password = SECRET_PASS;  // パスワード

WiFiServer server(80);  // Webサーバーのポート


const int ledPin = LED_BUILTIN;  // 内蔵LEDのピン
int blinkCount = 0;  // LEDの点滅回数


/******************/
/* 音声合成LSI関係 */
/******************/

ATP301x_ARDUINO_SPI atp301x;
char atpbuf[ATP_MAX_LEN];

/* メイン関数 */
void setup() {

  pinMode(D2, INPUT);//モード選択

  pinMode(D7, OUTPUT);//リセット端子駆動
  digitalWrite(D7, LOW);//リセット端子駆動

  Serial.begin(9600);

  atp301x.begin();

  setupRTC();

  delay(10000); //Platform IO がアップロードタスクからシリアルモニタタスクに戻るのを待つ

  //通常起動モードか設定モード化を切り替え

  if(digitalRead(D2) == HIGH){
    //設定モード
    Serial.println("設定モード");

    // WiFi モジュール搭載の確認:
    if (WiFi.status() == WL_NO_MODULE) {
      Serial.println("Communication with WiFi module failed!");
      // don't continue
      while (true);
    }  

  #ifdef SECRET_IP_ADDR
    //IPアドレスの設定
    WiFi.config(IPAddress(SECRET_IP_ADDR));
  #endif

    //アナウンス
    sprintf(atpbuf,"sette-mo'-dode/kido-shima'_su.");
    atp301x.talk(atpbuf,true);
    sprintf(atpbuf,"kinaimo'-doni/irete'kara waifaiosetsuzo_kushitekudasa'i.");
    atp301x.talk(atpbuf,true);
    sprintf(atpbuf,"<ALPHA VAL= SSID >wa <ALPHA VAL= %s >.",ssid);
    atp301x.talk(atpbuf,true);

    // アクセスポイントの開始
    WiFi.beginAP(ssid, password);
    while (WiFi.status() != WL_AP_LISTENING) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("\nアクセスポイント開始: " + String(ssid));

    // サーバー開始
    server.begin();
    Serial.println("Webサーバー開始");
    pinMode(ledPin, OUTPUT);

    delay(1000);
    printWiFiStatus();
  }else{
    //通常モード
      //アナウンス
      sprintf(atpbuf,"tsu-jo-mo'-dode/kido-shima'_su.");
      atp301x.talk(atpbuf,true);
      while(1);
  }
}

void loop() {
  WiFiClient client = server.available();  // クライアントの接続待機

  bool executeReset = false;

  if (client) {
    Serial.println("クライアント接続");
    String request = "";
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        request += c;
        if (c == '\n') break;
      }
    }
    Serial.println("レスポンス取得：request: " + request);

    if (request.indexOf("GET / ") != -1) {
      sendHTML(client, HTML_HOME);  // ホームページ
      printRTCtime();
    }
    else if (request.indexOf("GET /led") != -1) { //暗証番号ページ
      if (request.indexOf("count=") != -1) { // getがあれば
        int start = request.indexOf("count=") + 6;
        int end = request.indexOf(" ", start);
        blinkCount = request.substring(start, end).toInt();
        Serial.println("設定された点滅回数: " + String(blinkCount));
        blinkLED(blinkCount);
      }
      sendHTML(client, HTML_LED);  // LED設定ページ表示
    }
    else if (request.indexOf("GET /calendar") != -1) { //カレンダーページ
      String html_calendar = HTML_CALENDAR;

      if (request.indexOf("date=") != -1) { // getがあれば

        int start = request.indexOf("date=") + 5;
        String dateStr = request.substring(start, start + 10);
        Serial.println("getされた日付: " + dateStr);

        start = request.indexOf("time=") + 5;
        String timeStr = request.substring(start, start + 17);
        Serial.println("getされた時刻: " + timeStr);
        //%3A をsscanfに入れると可読性皆無になるので置き換える
        timeStr.replace("%3A", "-");
        Serial.println("getされた時刻 書式変換: " + timeStr);

        int yyyy, mm, dd , hh, mi, ss = 0;
        //フォーマットチェック
        if (sscanf(dateStr.c_str(), "%4d-%2d-%2d", &yyyy, &mm, &dd) != 3) {
          Serial.println("日付：書式が不正");
        } else if(sscanf(timeStr.c_str(), "%2d-%2d-%2d", &hh, &mi, &ss) != 3) {
          Serial.println("時刻：書式が不正");
        }else {
          //値チェック
          if(yyyy < 2025 || yyyy > 2090 || mm < 1 || mm > 12 || dd < 1 || dd > 31){
            Serial.println("日付：範囲外データ");
          }else if(hh < 0 || 24 < hh || mi < 0 || 60 < mi || ss < 0 || 60 < ss){
            Serial.println("時刻：範囲外データ");
          }else{
            //RTC更新
            RTCTime newTime;
            RTC.getTime(newTime); //更新しない部分現状維持
            newTime.setYear(yyyy);
            newTime.setMonthOfYear((Month)(mm - 1));//0始まりのenumになっているので1引くこと
            newTime.setDayOfMonth(dd);
            newTime.setHour(hh);
            newTime.setMinute(mi);
            newTime.setSecond(ss);
            RTC.setTime(newTime);
            Serial.println("RTCの日付を設定しました");
            printRTCtime();
          }
        }
      }

      //rtcの時刻を取得して表示
      RTCTime currentTime;
      RTC.getTime(currentTime);

      //プレースホルダを置き換え
      html_calendar.replace("%RTC_Y%", String(currentTime.getYear()));
      html_calendar.replace("%RTC_M%", String(Month2int(currentTime.getMonth())));
      html_calendar.replace("%RTC_D%", String(currentTime.getDayOfMonth()));

      sendHTML(client, html_calendar);  // カレンダー設定ページ表示

    }else if (request.indexOf("GET /endsetting") != -1) { //設定終了ページ
      executeReset = true;
      sendHTML(client, HTML_ENDSETTING);  // 設定終了ページを表示
    }else {
      //send404(client);  // 存在しないページ
      sendHTML(client, HTML_HOME);  // ホームページを表示
    }
    
    client.stop();
  }

  if(executeReset){
    //設定終了
    Serial.println("設定終了");
    executeReset();
    while(1);
  }

} //loop終わり

// LEDを指定回数点滅させる関数
void blinkLED(int count) {
  for (int i = 0; i < count; i++) {
    digitalWrite(ledPin, HIGH);
    delay(500);
    digitalWrite(ledPin, LOW);
    delay(500);
  }
}

// HTMLページを送信する関数
void sendHTML(WiFiClient client, String page) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(page);
}

void send404(WiFiClient client) {
  client.println("HTTP/1.1 404 Not Found");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println("<h1>404 Not Found</h1>");
}

void printWiFiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in a browser:
  Serial.print("To see this page in action, open a browser to http://");
  Serial.println(ip);

}


void printRTCtime(void){
  RTCTime currentTime;
  bool running = RTC.isRunning();

  if (running) {
    Serial.println("RTC is running!");
  }else{
    Serial.println("RTC is not running!");
  }

  // Get current time from RTC
  RTC.getTime(currentTime);

  // Print out UNIX time
  Serial.print("UNIX time: ");
  Serial.println(currentTime.getUnixTime());


  Serial.print("YYYY/MM/DD - HH/MM/SS:");
  Serial.print(currentTime.getYear());
  Serial.print("/");
  Serial.print(Month2int(currentTime.getMonth()));
  Serial.print("/");
  Serial.print(currentTime.getDayOfMonth());
  Serial.print(" - ");
  Serial.print(currentTime.getHour());
  Serial.print(":");
  Serial.print(currentTime.getMinutes());
  Serial.print(":");
  Serial.println(currentTime.getSeconds());
}


void setupRTC(void){
  //何故かRTCの開始と時刻設定が一体化しているAPI仕様のため
  //一旦取得して時刻設定という動きをしないと枚リセットごとに時計が初期化されてしまう

  RTC.begin(); // RTCの初期化　これだけではRTC動き始めない
  printRTCtime(); //表示

  RTCTime rtcTime;
  RTC.getTime(rtcTime);// RTCから現在時刻を取得

  //もし2000年(リセット)されていたら、必ず有効期限切れになるように未来を設定
  //2000年のままだと有効期限が全部OKになってしまうため
  if(rtcTime.getYear() == 2000){
    rtcTime.setYear(2090);
  }


  RTC.setTimeIfNotRunning(rtcTime); // 現在時刻を引き継いでRTCをスタート

  printRTCtime(); //表示
}