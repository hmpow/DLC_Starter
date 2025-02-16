//ChatGPTの提案とArduinoのサンプルプログラムを合体してベースを作成

#include <Arduino.h>
#include <WiFiS3.h>
#include <RTC.h>

#include "arduino_secrets.h"
#include "web_page.h"

// #include <WiFiServer.h> //WiFiS3.h → WiFi.h → WiFiServer.h でインクルードされる

/* プロトタイプ宣言 PlatformIO では必要 */
void blinkLED(int);
void sendHTML(WiFiClient, const char*);
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

/* メイン関数 */
void setup() {
  delay(10000); //Platform IO がアップロードタスクからシリアルモニタタスクに戻るのを待つ

  Serial.begin(9600);

  setupRTC();

  // WiFi モジュール搭載の確認:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }  
  
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
}

void loop() {
  WiFiClient client = server.available();  // クライアントの接続待機

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


    if (request.indexOf("GET / ") != -1) {
      sendHTML(client, HTML_HOME);  // ホームページ
      printRTCtime();
    }
    else if (request.indexOf("GET /led") != -1) {
      if (request.indexOf("count=") != -1) {
        int start = request.indexOf("count=") + 6;
        int end = request.indexOf(" ", start);
        blinkCount = request.substring(start, end).toInt();
        Serial.println("設定された点滅回数: " + String(blinkCount));
        blinkLED(blinkCount);
      }
      sendHTML(client, HTML_LED);  // LED設定ページ
    }
    else {
      //send404(client);  // 存在しないページ
      sendHTML(client, HTML_HOME);  // ホームページを表示
    }
    
    client.stop();
  }
}

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
void sendHTML(WiFiClient client, const char* page) {
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

  RTC.setTimeIfNotRunning(rtcTime); // 現在時刻を引き継いでRTCをスタート

  printRTCtime(); //表示
}