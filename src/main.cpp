//ChatGPTの提案とArduinoのサンプルプログラムを合体してベースを作成

#include <Arduino.h>
#include <WiFiS3.h>
#include "arduino_secrets.h"
#include "web_page.h"

// #include <WiFiServer.h> //WiFiS3.h → WiFi.h → WiFiServer.h でインクルードされる


/* プロトタイプ宣言 PlatformIO では必要 */
void blinkLED(int);
void sendHTML(WiFiClient);
void printWiFiStatus(void);

/* 設定 */

const char* ssid     = SECRET_SSID;  // アクセスポイントのSSID
const char* password = SECRET_PASS;  // パスワード

WiFiServer server(80);  // Webサーバーのポート

const int ledPin = LED_BUILTIN;  // 内蔵LEDのピン
int blinkCount = 0;  // LEDの点滅回数

/* メイン関数 */
void setup() {
  Serial.begin(9600);
  
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

  delay(10000);

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

    // クエリパラメータから点滅回数を取得
    if (request.indexOf("GET /?count=") != -1) {
      int start = request.indexOf("count=") + 6;
      int end = request.indexOf(" ", start);
      blinkCount = request.substring(start, end).toInt();
      Serial.println("設定された点滅回数: " + String(blinkCount));
      blinkLED(blinkCount);
    }

    // HTMLレスポンスを送信
    sendHTML(client);
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
void sendHTML(WiFiClient client) {
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println("Connection: close");
  client.println();
  client.println(SETTING_HTML);
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