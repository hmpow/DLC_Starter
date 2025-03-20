#include "main_setting.h"

#define executeReset() digitalWrite(RESET_OUT_PIN, HIGH)

const char* ssid     = SECRET_SSID;  // アクセスポイントのSSID
const char* password = SECRET_PASS;  // パスワード

WiFiServer server(80);  // Webサーバーのポート

ArduinoLEDMatrix matrix;

const uint32_t matrix_WiFi[] = {
	0x6019820,
	0x44f29092,
	0x640f0060
};

void main_settingMode_setup(void){
    //設定モード
    Serial.println("設定モード");

    matrix.begin();

    // WiFi モジュール搭載の確認:
    if (WiFi.status() == WL_NO_MODULE) {
      while (true){
        announceWifiModuleNotFound();
        delay(1000);
      }
    }  

    matrix.loadFrame(matrix_WiFi);

#ifdef SECRET_IP_ADDR_UPPER
#ifdef SECRET_IP_ADDR_LOWER
    //IPアドレスの設定
    if(0 < SECRET_IP_ADDR_UPPER
      && SECRET_IP_ADDR_UPPER < 255 
      && 0 < SECRET_IP_ADDR_LOWER 
      && SECRET_IP_ADDR_LOWER < 255){
        
        WiFi.config(IPAddress(192, 168, SECRET_IP_ADDR_UPPER, SECRET_IP_ADDR_LOWER));
    }
#endif
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

    //クライアントからリクエストを受信

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

    //リクエストに応じた処理

    if (request.indexOf("GET / ") != -1) {
      sendHTML(client, HTML_HOME);  // ホームページ
      printRTCtime();
    }
    else if (request.indexOf("GET /pinsetting") != -1) { //暗証番号ページ

        String html_pinsetting = HTML_PIN_SETTING;

        //いたずら防止判定
        uint16_t inputNo = 10000; // "0000" はありるので範囲外に初期化
        if (request.indexOf("secno=") != -1) { // getがあれば
            int start = request.indexOf("secno=") + 6;
            int end = request.indexOf(" ", start);
            printf("SeqNo start: %d, end: %d\n", start, end);
            String secNumStr = request.substring(start, end);
            Serial.println("getされたsecNo_Str: " + secNumStr);
            inputNo = (uint16_t)secNumStr.toInt();
            printf("getされたsecNo_Int: %d\n" , inputNo);

            bool isVerifyOk = verifySecurityNo(inputNo);

            if(!isVerifyOk){
                //暗証番号が不一致
                Serial.println("いたずら防止暗証番号間違い");
                html_pinsetting.replace("%MESSAGE%", "いたずら防止暗証番号が違います");
            }else{
                //暗証番号が一致
                Serial.println("いたずら防止暗証番号一致");

                uint8_t drvNum = 0;

                //ドライバー番号の処理
                if (request.indexOf("driver=") != -1) { // getがあれば
                    int start = request.indexOf("driver=") + 7;
                    int end = request.indexOf("&dlcpin", start);

                    printf("ドライバー start: %d, end: %d\n", start, end);

                    String drvNumStr = request.substring(start, end); 

                    //String drvNumStr = request.substring(start, start + 1); 
                    //固定長で良い→良くない　アドレスバーで200とか入れるテストしたら 2 になった
                    //文字とか入った際はtoIntでゼロにしてくれるのでOK　ドライバーを 1 始まりにすることがポイント
                    //マイナス入ったら→符号ビットがunsignedへのキャストされ範囲外の巨大数になるのでOK
                    Serial.println("getされたdrvNum_str: " + drvNumStr);
                    drvNum = (uint8_t)drvNumStr.toInt();

                    printf("getされたdrvNum_Int: %d\n" , drvNum);

                    if(0 < drvNum && drvNum <= DRIVER_LIST_NUM){
                        //ドライバー番号照合OK
                        Serial.println("ドライバー番号が正常");

                        type_EEPROM_PIN dcPin = pinEEPROM.getPin(drvNum - 1);
                        printf("EEPROMから取得したPIN: %d %d %d %d\n" , dcPin[0], dcPin[1], dcPin[2], dcPin[3]);

                        //PINの処理
                        if (request.indexOf("dlcpin=") != -1) { // getがあれば
                            int start = request.indexOf("dlcpin=") + 7;
                            int end = request.indexOf("&secno", start);

                            printf("マイナPIN start: %d, end: %d\n", start, end);

                            if(start == end){
                                //PINが空白
                                Serial.println("PINが空白");
                                pinEEPROM.clearPin(drvNum - 1);
                                pinEEPROM.debugPrintEEPROM(24);
                                html_pinsetting.replace("%MESSAGE%", "ドライバー " +  String(drvNum) + " の設定を削除しました");
                                
                            }else{
                                //PINがある
                                String dlcPinStr = request.substring(start, end);
                                Serial.println("getされたdlcPin_str: " + dlcPinStr);
                                uint16_t dlcPinInt = (uint16_t)dlcPinStr.toInt();
                                
                                if(dlcPinInt < 10000){
                                    //PINが範囲内
                                    type_EEPROM_PIN dlcPin = {0,0,0,0};
                                    dlcPin[0] = (uint8_t)(dlcPinInt / 1000);
                                    dlcPin[1] = (uint8_t)((dlcPinInt % 1000) / 100);
                                    dlcPin[2] = (uint8_t)((dlcPinInt % 100) / 10);
                                    dlcPin[3] = (uint8_t)(dlcPinInt % 10);
                                    printf("getされたdlcPin_Int: %d %d %d %d\n" , dlcPin[0], dlcPin[1], dlcPin[2], dlcPin[3]);
                                    pinEEPROM.updatePin(drvNum - 1, dlcPin);
                                    pinEEPROM.debugPrintEEPROM(24);
                                    html_pinsetting.replace("%MESSAGE%", "ドライバー " +  String(drvNum) + " の設定を更新しました");

                                }else{
                                    //PINが範囲外
                                    Serial.println("PINが範囲外");
                                    html_pinsetting.replace("%MESSAGE%", "PIN設定エラー");
                                }
                            }
                        }
                        type_EEPROM_PIN dlcPin = pinEEPROM.getPin(drvNum - 1);
                        printf("EEPROMから取得したPIN: %d %d %d %d\n" , dlcPin[0], dlcPin[1], dlcPin[2], dlcPin[3]);

                    }else{
                        //ドライバー番号照合NG
                        Serial.println("ドライバー番号が不正");
                        html_pinsetting.replace("%MESSAGE%", "ドライバー番号設定エラー"); 
                    }
                }
            }
        }else{
            //プレースホルダメッセージ無し
            html_pinsetting.replace("%MESSAGE%", "");
        }
        sendHTML(client, html_pinsetting);  // 暗証番号設定ページ表示
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

    //サーバー終了
    server.end();
    // アクセスポイントの終了
    WiFi.end();
    
    Serial.println("設定終了");

    announceEndSettingMode();
    
    //リセット実行
    executeReset();
    while(1){};
  }

  return;

} //main_settingMode_loop END


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


bool verifySecurityNo(uint16_t inputNo){

    printf("verifySecurityNo input_hex:  %X\n", inputNo);
    printf("verifySecurityNo SECURITY_NO_hex:  %X\n", (uint16_t)SECRET_SECURITY_NO);

    if(9999 < inputNo){
        //uintを使っているため負の数判定は不要
        return false;
    }

    if(inputNo == (uint16_t)SECRET_SECURITY_NO){
        return true;
    }
    
    return false;
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

  //設定モード終了アナウンス
  void announceEndSettingMode(){
    //「設定モードを終了し本体を再起動します」
    atp301x.talk("sette-mo'-doo/shu-ryo-_shi,ho'nntaio/saiki'do-shima_su.");
    return;
  }