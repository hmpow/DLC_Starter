//ChatGPTの提案とArduinoのサンプルプログラムを合体してベースを作成

#include <Arduino.h>
#include <WiFiS3.h>
#include "arduino_secrets.h"
#include "web_page.h"

// #include <WiFiServer.h> //WiFiS3.h → WiFi.h → WiFiServer.h でインクルードされる

// https://docs.arduino.cc/language-reference/en/functions/wifi/server/
// https://docs.arduino.cc/language-reference/en/functions/wifi/client/
// https://docs.arduino.cc/libraries/wifi/


/* プロトタイプ宣言 PlatformIO では必要 */
void blinkLED(int);
void sendHTML(WiFiClient, const char*);
void send404(WiFiClient);
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

  // check for the WiFi module:
  if (WiFi.status() == WL_NO_MODULE) {
    Serial.println("Communication with WiFi module failed!");
    // don't continue
    while (true);
  }  
  
  // アクセスポイントの開始
  WiFi.beginAP(ssid, password);
  delay(500);
  while (WiFi.status() != WL_AP_LISTENING) {
    delay(500);
    Serial.print(".");
  }
  delay(500);
  Serial.println("\nアクセスポイント開始: " + String(ssid));
  delay(500);
  // サーバー開始
  server.begin();
  Serial.println("Webサーバー開始");
  delay(500);
  pinMode(ledPin, OUTPUT);

  delay(1000);

  printWiFiStatus();
  delay(500);
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