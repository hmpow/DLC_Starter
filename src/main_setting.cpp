#include "main_setting.h"

#define executeReset() digitalWrite(RESET_OUT_PIN, HIGH)


const char* ssid     = SECRET_SSID;  // アクセスポイントのSSID
const char* password = SECRET_PASS;  // パスワード


WiFiServer server(80);  // Webサーバーのポート


void main_settingMode_setup(void){
    //設定モード
    Serial.println("設定モード");

    // WiFi モジュール搭載の確認:
    if (WiFi.status() == WL_NO_MODULE) {
      while (true){
        announceWifiModuleNotFound();
        delay(1000);
      }
    }  

#ifdef SECRET_IP_ADDR
    //IPアドレスの設定
    WiFi.config(IPAddress(SECRET_IP_ADDR));
#endif

    announcePleaseConnectWiFi();

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

    delay(1000);
    printWiFiStatus();
    return;
}

void main_settingMode_loop(void){
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
    else if (request.indexOf("GET /pinsetting") != -1) { //暗証番号ページ
      if (request.indexOf("count=") != -1) { // getがあれば
        int start = request.indexOf("count=") + 6;
        int end = request.indexOf(" ", start);
      }
      sendHTML(client, HTML_PIN_SETTING);  // 暗証番号設定ページ表示
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
      sendHTML(client, HTML_HOME);  // ホームページを表示
    }
    
    client.stop();
  }

  if(executeReset){
    //設定終了
    server.end();
    // アクセスポイントの終了
    WiFi.end();
    Serial.println("設定終了");
    executeReset();
    while(1);
  }
  return;
}


// HTMLページを送信する関数
void sendHTML(WiFiClient client, String page) {
    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/html");
    client.println("Connection: close");
    client.println();
    client.println(page);
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


//Wi-Fiモジュール初期化失敗アナウンス
void announceWifiInitializeFailed(){
    if(USE_ATP301X){
      //音声合成「Wi-Fiモジュール初期化エラー　再起動してください」
      atp301x.talk("waifaimoju'-rusho_kika/e'ra- sai'ki'do-shitekudasai.");
    }
    return;
  }
  
  //WiFiモジュールが見つからない
  void announceWifiModuleNotFound(){
    if(USE_ATP301X){
      //音声合成「無線設定機能はワイファイモジュール搭載ハードが必要です」
      atp301x.talk("musennsette-ki'no-wa waifaimoju'-ruto-sa'i/ha'-doga/_hitsuyo'-de_su.");
    }
    return;
  }
  
  //WiFi接続アナウンス
  void announcePleaseConnectWiFi(){
    if(USE_ATP301X){
      //アナウンス
      sprintf(atpbuf,"sette-mo'-dode/kido-shima'_su.");
      atp301x.talk(atpbuf,true);
      sprintf(atpbuf,"kinaimo'-doni/irete'kara waifaiosetsuzo_kushitekudasa'i.");
      atp301x.talk(atpbuf,true);
      sprintf(atpbuf,"<ALPHA VAL= SSID >wa <ALPHA VAL= %s >.",ssid);
      atp301x.talk(atpbuf,true);
    }
    return;
  }