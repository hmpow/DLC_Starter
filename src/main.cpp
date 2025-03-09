//ChatGPTの提案とArduinoのサンプルプログラムを合体してベースを作成

#define TEST_WAIT_HUMAN_READABLE_INTERVAL_MS 50

#define USE_ATP301X true
#define SHOW_DEBUG true

#define KIGENGIRETOSURUJIKOKU_H 12

#include <Arduino.h>
#include <WiFiS3.h>
#include <RTC.h>

#include "arduino_secrets.h"
#include "web_page.h"

#include "StartCtrl.h"
#include "rcs660s_app_if.h"
#include "ATP301x_Arduino_SPI.h"

#define executeReset() digitalWrite(D7, HIGH)





/** ★↓★↓★↓★↓★↓★↓★↓★↓★↓★ mbed からコピー ★↓★↓★↓★↓★↓★↓★↓★↓★↓★ **/

//NFCコマンド
const std::vector<uint8_t> wireless_SELECT_MF = {0x00,0xA4,0x00,0x00};
const std::vector<uint8_t> wireless_SELECT_EF01_directly = {0x00,0xA4,0x02,0x0C,0x04,0x3F,0x00,0x2F,0x01};
const std::vector<uint8_t> wireless_SELECT_EF01_whileMFselected = {0x00,0xA4,0x02,0x0C,0x02,0x2F,0x01};
const std::vector<uint8_t> wireless_READ_BINARY_noOffset_255 = {0x00,0xB0,0x00,0x00,0x00};

//免許証固有のAID （これを使ったSELECTコマンドの組み立てはプログラムで実施）
const uint8_t dlc_DF1_AID[] = {0xA0,0x00,0x00,0x02,0x31,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const uint8_t dlc_DF2_AID[] = {0xA0,0x00,0x00,0x02,0x31,0x02,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};
const uint8_t dlc_DF3_AID[] = {0xA0,0x00,0x00,0x02,0x48,0x03,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00};


//共通データ要素の中身整理用構造体
//※有効期限しか使いませんが、RFUとして全てのデータを整理しておきます。
typedef struct{
  uint8_t shiyoVer[3];
  uint8_t kofuYYYYMMDD[4];
  uint8_t yukoYYYYMMDD[4];
}mfDataStruct;


void mbed_main();

/****************************************************************************/

/******************/
/* プロトタイプ宣言 */
/******************/

//制御系
void allowDrive();
void disallowDrive();
void reset();
//void audioOn();
//void audioOff();
void announcePleaseTouch();
void igswRiseIrq();
void errorBeep(int);

//送信系
std::vector<uint8_t> sendSELECTbyAID(const uint8_t*, uint8_t );

//検査系
bool checkATQB(uint8_t*, int);
bool checkSELECTres(std::vector<uint8_t>);
bool checkREAD_BINARYres(std::vector<uint8_t>);

bool isEfectiveLicenseCard(tm);

//時刻系
bool isSetRTC();
int packedBCDtoInt(uint8_t);
tm packedBCDdata_to_tmStruct(uint8_t*);
mfDataStruct mfData_toStruct(std::vector<uint8_t>);

//表示機能
void debugPrintATQB(uint8_t*);
void debugPrintMFdata(std::vector<uint8_t>);

/** ★↑★↑★↑★↑★↑★↑★↑★↑★↑★ mbed からコピー ★↑★↑★↑★↑★↑★↑★↑★↑★↑★ **/





// #include <WiFiServer.h> //WiFiS3.h → WiFi.h → WiFiServer.h でインクルードされる

//要調査：wifi起動したら音声合成使用禁止　フリーズする　リソース競合？

/* プロトタイプ宣言 PlatformIO では必要 */
void blinkLED(int);
void sendHTML(WiFiClient, String);
void send404(WiFiClient);
void printWiFiStatus(void);


void printRTCtime(void);
void setupRTC(void);

void main_settingMode_setup(void);
void main_settingMode_loop(void);

void main_normalMode_setup(void);
void main_normalMode_loop(void);


void printCardRes(const std::vector<uint8_t>);

/* 設定 */

const char* ssid     = SECRET_SSID;  // アクセスポイントのSSID
const char* password = SECRET_PASS;  // パスワード


WiFiServer server(80);  // Webサーバーのポート


const int ledPin = LED_BUILTIN;  // 内蔵LEDのピン
int blinkCount = 0;  // LEDの点滅回数

//起動モード　増やすかもしれないため enum + switch~case にしておく
enum BOOT_MODE{
  NORMAL = 0,
  SETTING
} bootMode;

/*****************/
/* 自作クラスたち */
/*****************/
Rcs660sAppIf rcs660sAppIf;
ATP301x_ARDUINO_SPI atp301x;
StartCtrl_DigitalOut startCtrl;

char atpbuf[ATP_MAX_LEN];

/* メイン関数 */
void setup() {

  /*************/
  /* モード共通 */
  /*************/

  startCtrl.setup(); //どちらのモードでも禁止側に初期化必要

  pinMode(D2, INPUT);//起動モード選択

  pinMode(D7, OUTPUT);//リセット端子駆動
  digitalWrite(D7, LOW);//リセット端子駆動

  pinMode(ledPin, OUTPUT);

  Serial.begin(9600);

  atp301x.begin();

  setupRTC();

  delay(10000); //Platform IO がアップロードタスクからシリアルモニタタスクに戻るのを待つ

  /*************/
  /* モード切替 */
  /*************/

  //起動モード判定
  if(digitalRead(D2) == LOW){
    bootMode = SETTING;
  }else{
    bootMode = NORMAL;
  }

  //起動モードに応じて処理切替
  switch (bootMode)
  {
    case NORMAL:
      main_normalMode_setup();
      break;
    case SETTING:
      main_settingMode_setup();
      break;
    default:
      break;
  }
}

void loop() {
  //起動モードに応じて処理切替
  switch (bootMode)
  {
    case NORMAL:
      main_normalMode_loop();
      break;
    case SETTING:
      main_settingMode_loop();
      break;
    default:
      break;
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



void main_settingMode_setup(void){
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
  return;
}

void main_normalMode_setup(void){
  
  rcs660sAppIf.begin();

  //アナウンス
  sprintf(atpbuf,"tsu-jo-mo'-dode/kido-shima'_su.");
  atp301x.talk(atpbuf,true);
  Serial.println("通常モード");

  return;
}

void main_normalMode_loop(void){
  mbed_main();
  startCtrl.allow();
  atp301x.chimeK();
  startCtrl.deny();
  atp301x.chimeJ();
  return;
}



void printCardRes(const std::vector<uint8_t> vec){
  if(vec.empty() == false){
    debugPrintMsg("CARD RES START");
    for (size_t i = 0; i < vec.size(); i++)
    {
      debugPrintHex(vec[i]);
    }
    debugPrintMsg("CARD RES END");
  }else{
    debugPrintMsg("ERROR! CARD RES is empty");
  }
  uart_wait_ms(TEST_WAIT_HUMAN_READABLE_INTERVAL_MS);
  return;
}



/** ★↓★↓★↓★↓★↓★↓★↓★↓★↓★ mbed からコピー ★↓★↓★↓★↓★↓★↓★↓★↓★↓★ **/

/************/
/*** main ***/
/************/

void mbed_main() {
//起動処理
  disallowDrive();
  //audioOn();
  
  bool canDrive = false;
  char atpbuf[ATP_MAX_LEN];
  
  //uint8_t* resArray = new uint8_t[rcs620.RETURN_ENVELOPE_SIZE];
  std::vector<uint8_t> resVector;

  int resLen;
  
  // ig_start_sw.mode (PullDown);
  
  while(!canDrive){
      // beep.setFreq(BEEP_FREQ);
      reset();

#if 0
      //マイコン起動より早くキーひねると割り込みかからないから初回手動でブザー鳴らす
      if(ig_start_sw.read()){
          // beep.turnOn();
      }

      ig_start_sw.rise(&igswRiseIrq);
      ig_start_sw.fall(callback(&beep, &PwmBeep::turnOff));
#endif

      //音声合成「免許証をタッチしてください」
      announcePleaseTouch();
  
     //NFC Type-Bをポーリング
     rcs660sAppIf.setNfcType(NFC_TYPE_B);
     rcs660sAppIf.updateTxAndRxFlag({false, false, 3, false});
     bool isCatch = rcs660sAppIf.catchNfc(RETRY_CATCH_INFINITE);

     /* 捕捉できるまでcatchNfcを抜けてこない */

#if 0
      //警報割り込み解除
      ig_start_sw.rise(callback(&beep, &PwmBeep::turnOff));
      // beep.turnOff();
#endif      
      if(USE_ATP301X){
          //タッチ音「ポーン」
          atp301x.chimeJ(false);
      }else{
          //ビープ「ピッ」
          // beep.oneshotOn(0.1);    
      }

#if 0      
      //ポーリングレスポンス読み出し
      rcs620.getCardRes(resArray, &resLen);
      
      debugPrintATQB(resArray);
      
  //免許証判定STEP1 ATQBをチェック
  
      if(!checkATQB(resArray, resLen)){
          //ビープ「ピーピー」
          errorBeep(2);
          if(USE_ATP301X){
              //音声合成「ATQB応用データ照合失敗」
              atp301x.talk("<ALPHA VAL=ATQB>/o-yo-de'-ta/sho-go-shippai.");
          }
          continue;
      }
#endif     
  //免許証判定STEP2 3つのAIDが存在するかチェック
  
      //DF1があるか確認
      std::vector<uint8_t> txData;
      std::vector<uint8_t> rxData;
      uint8_t resArray[2] = {0};     

      txData = sendSELECTbyAID(dlc_DF1_AID, sizeof(dlc_DF1_AID)/sizeof(dlc_DF1_AID[0]));
      rxData = rcs660sAppIf.communicateNfc(txData, 60);
      printCardRes(rxData);
         
      if(!checkSELECTres(rxData)){
          //ビープ「ピーピーピー」
          errorBeep(3);
          if(USE_ATP301X){
              //音声合成「DF1 照合失敗」
              atp301x.talk("<ALPHA VAL=DF1>/sho-go-shippai.\r\0");
          }
          continue;
      }

      txData.clear();
      rxData.clear();
      
      //AID2があるか確認
      txData = sendSELECTbyAID(dlc_DF2_AID, sizeof(dlc_DF2_AID)/sizeof(dlc_DF2_AID[0]));
      rxData = rcs660sAppIf.communicateNfc(txData, 60);
      printCardRes(rxData);

      if(!checkSELECTres(rxData)){
          //ビープ「ピーピーピー」
          errorBeep(3);
          if(USE_ATP301X){
              //音声合成「DF2 照合失敗」
              atp301x.talk("<ALPHA VAL=DF2>/sho-go-shippai.\r\0");
          }
          continue;
      }
      
      txData.clear();
      rxData.clear();
      
      //AID3があるか確認
      txData = sendSELECTbyAID(dlc_DF3_AID, sizeof(dlc_DF3_AID)/sizeof(dlc_DF3_AID[0]));
      rxData = rcs660sAppIf.communicateNfc(txData, 60);
      printCardRes(rxData);

      if(!checkSELECTres(rxData)){
          //ビープ「ピーピーピー」
          errorBeep(3);
          if(USE_ATP301X){
              //音声合成「DF3 照合失敗」
              atp301x.talk("<ALPHA VAL=DF3>/sho-go-shippai.\r\0");
          }
          continue;
      }

      txData.clear();
      rxData.clear();

  //ここまで来られたらタッチされたカードは免許証！
  //共通データ要素読み出し
      
      //MFを選択
      rxData = rcs660sAppIf.communicateNfc(wireless_SELECT_MF, 60);
      printCardRes(rxData);
      
      if(!checkSELECTres(rxData)){
          //ビープ「ピーピーピーピー」
          errorBeep(4);
          if(USE_ATP301X){
              //音声合成「MF選択失敗」
              atp301x.talk("<ALPHA VAL=MF>se'nnta_ku/shippai.\r\0");
          }
          continue;
      }

      txData.clear();
      rxData.clear();
  
      //MF内のEF01を選択
      rxData = rcs660sAppIf.communicateNfc(wireless_SELECT_EF01_whileMFselected, 60);
      printCardRes(rxData);
      
      if(!checkSELECTres(rxData)){
          //ビープ「ピーピーピーピー」
          errorBeep(4);
          if(USE_ATP301X){
              //音声合成「EF01選択失敗」
              atp301x.talk("<ALPHA VAL=EF01>/sennta_ku/shippai.\r\0");
          }
          continue;
      }

      txData.clear();
      rxData.clear();      
      
      //EF01(共通データ要素)をREAD BINARY
      rxData = rcs660sAppIf.communicateNfc(wireless_READ_BINARY_noOffset_255, 60);
      printCardRes(rxData);

      //ステータスが正常か確認
      if(!checkREAD_BINARYres(rxData)){
          //ビープ「ピーピーピーピーピー」
          errorBeep(5);
          if(USE_ATP301X){
              //音声合成「共通データ要素読み取り失敗」
              atp301x.talk("kyo-tsu-de-tayo'-so/yomitori/shippai.\r\0");
          }
          continue;
      }
                    
      //データ長が短すぎないか確認
      //※将来的に暗証番号の領域を使えるようcheckREAD_BINARYres関数の外で確認してます
      if(rxData.size() < 19){
          if(USE_ATP301X){
              //音声合成「共通データ要素読み取り失敗」
              atp301x.talk("kyo-tsu-de-tayo'-so/yomitori/shippai.\r\0");
          }
          continue;
      }
      
      
      debugPrintMFdata(rxData);
  
  //共通データ要素の中身チェック
  
      //共通データ要素の中身を分割して構造体へ保存
      mfDataStruct mfData;
      mfData = mfData_toStruct(rxData);
      
      //パック2進化10進数形式の有効期限をC標準の tm 構造体へ変換
      tm yukoKigen_tm;
      yukoKigen_tm = packedBCDdata_to_tmStruct(mfData.yukoYYYYMMDD);
                
      if(isEfectiveLicenseCard(yukoKigen_tm)){
          allowDrive();
          
          //ループ終わり
          canDrive = true;
          
          //ビープ「ピッ(高音)」
          // beep.setFreq(BEEP_FREQ_AFTER_READ);
          // beep.oneshotOn(0.1);
      }else{
          if(USE_ATP301X){
              //音声合成「有効期限が切れいてるか、時計がズレています」
              atp301x.talk("yu-ko-ki'gennga/ki'rete+iru'ka toke-ga/zu'rete+ima_su.\r\0");

          //RTC時刻読み上げ
              time_t seconds = time(NULL); //read RTC as UNIX time data
              struct tm *t = localtime(&seconds);
      
              //音声合成「RTC時刻は」
              atp301x.talk("a-ruthi-shi'-/ji'kokuwa.\r\0");
              //音声合成「＿年＿月＿日」
              sprintf(atpbuf,"<NUMK VAL=%d COUNTER=nenn>/<NUMK VAL=%d COUNTER=gatu>/<NUMK VAL=%d COUNTER=nichi>.\r",t->tm_year+1900,t->tm_mon+1,t->tm_mday);
              atp301x.talk(atpbuf);
              //音声合成「＿時＿分＿秒です」
              sprintf(atpbuf,"<NUMK VAL=%d COUNTER=ji>/<NUMK VAL=%d COUNTER=funn>/de_su.\r",t->tm_hour,t->tm_min);
              atp301x.talk(atpbuf);
          }
      }
      
  //有効期限読み上げ
  
      if(USE_ATP301X){
          //音声合成「有効期限は＿年＿月＿日です」
          sprintf(atpbuf,"yu-ko-ki'gennwa <NUMK VAL=%d COUNTER=nenn>/<NUMK VAL=%d COUNTER=gatu>/<NUMK VAL=%d COUNTER=nichi>/de_su.\r",
                      yukoKigen_tm.tm_year+1900, yukoKigen_tm.tm_mon+1, yukoKigen_tm.tm_mday);
          atp301x.talk(atpbuf,true);
      }
  }//while終わり
  
  //カードリーダの電源OFF
  //rcs620.powerDown();

  //オーディオ回路スリープ
  //audioOff();

  rcs660sAppIf.releaseNfc();

  while(1){
    //無限ループ
  }
}

/****************************************************************************/

/*************/
/*** 制御系 ***/
/*************/

//エンジン始動OKの処理
void allowDrive(){
  startCtrl.allow();
  return;
}

//エンジン始動NGの処理
void disallowDrive(){
  startCtrl.deny();
  return;
}

#if 0
void audioOn(){
  //ATP3011 Highだと動作、NJM2113 LOWだと動作
  afAmpSleep = 0;
  nAtpSleep = 1;
}

void audioOff(){
  afAmpSleep = 1;
  nAtpSleep = 0;
}
#endif

void reset(){
  rcs660sAppIf.resetDevice();
  return;
}

//「免許証をタッチしてください」アナウンス
void announcePleaseTouch(){
  //音声合成「免許証をタッチしてください」
  if(USE_ATP301X){
      atp301x.talk("mennkyo'sho-o/ta'cchi/shitekudasa'i.\r\0",false);
  }

#if 0
  if(!ig_start_sw.read()){
      // beep.NshotOn(3, 0.07, 0.03);
  }
#endif

  return;
}

//無免許警報ブザー処理
void igswRiseIrq(){
  // beep.turnOn();
  announcePleaseTouch();
}

//USE_ATP301X 設定に応じビープモード切替
//ATP3011をしゃべらせる場合はatp3011でwait()、しゃべらせない場合はbeepでwait()
void errorBeep(int shotnum){
  if(USE_ATP301X){
      // beep.NshotOn(shotnum, 0.2, 0.1);
  }else{
      // beep.NshotOnwithWait(shotnum, 0.2, 0.1);
  }
  return;
}

/*************/
/*** 送信系 ***/
/*************/

//AIDからSELECTコマンドを作成しIndataexchanfeへ投げる
std::vector<uint8_t> sendSELECTbyAID(const uint8_t aid[], uint8_t aidLen){
  
  std::vector<uint8_t> wireless_command;

  const uint8_t cla = 0x00;
  const uint8_t ins = 0xA4;
  const uint8_t p1  = 0x04;
  const uint8_t p2  = 0x0C;

  const uint8_t Lc  = aidLen;

  wireless_command.push_back(cla);
  wireless_command.push_back(ins);
  wireless_command.push_back(p1);
  wireless_command.push_back(p2);
  wireless_command.push_back(Lc);
  
  for(uint8_t i = 0; i < aidLen; i++){
    wireless_command.push_back(aid[i]);
  }

  return wireless_command;
}


/*************/
/*** 検査系 ***/
/*************/

//ATQB応用データを免許証仕様書記載値と照合
bool checkATQB(uint8_t cardRes[], int len){
  if(len < 14){
      return false;
  }  
  //ATQB応用データが "00 00 00 00" であるか確認
  for(int j = 6; j<=9;j++){
      if(cardRes[j] != 0x00){
          return false;
      }
  }
  return true;
}

//SELECTに対する応答をチェック
bool checkSELECTres(std::vector<uint8_t> cardRes){
  if(cardRes.empty() == true  || cardRes.size() < 2){
      return false;
   }
  //カードのレスポンスが”90 00"(正常終了)または"62 83"(暗証番号間違えすぎてロック)であればフォルダが存在と判断
  if(cardRes[0] == 0x90 && cardRes[1] == 0x00){
      if(SHOW_DEBUG){printf("Card Status OK! %02X %02X\r\n",cardRes[0],cardRes[1]);}
      return true;
  }else if(cardRes[0] == 0x62 && cardRes[1] == 0x83){
      if(SHOW_DEBUG){printf("Card Status LOCKED but OK! %02X %02X\r\n",cardRes[0],cardRes[1]);}
      return true;
  }else{
      if(SHOW_DEBUG){printf("Card Status ERROR! %02X %02X\r\n",cardRes[0],cardRes[1]);}
      return false;
  }
}

//READ_BINARYに対する応答をチェック
bool checkREAD_BINARYres(std::vector<uint8_t> cardRes){
  
  unsigned int len = cardRes.size();

  if(cardRes.empty() == true  || len < 2){
      return false;
  }
  
  //カードのレスポンスが”90 00"(正常終了)であるかチェック
  if(cardRes[len-2] == 0x90 && cardRes[len-1] == 0x00){
      if(SHOW_DEBUG){printf("Card Status OK! %02X %02X\r\n",cardRes[len-2],cardRes[len-1]);}
      return true;
  }else{
      if(SHOW_DEBUG){printf("Card Status ERROR! %02X %02X\r\n",cardRes[len-2],cardRes[len-1]);}
      return false;
  }
}

//免許証有効期限チェック
bool isEfectiveLicenseCard(tm yukoKigen){
  //unix時刻で比較する
  RTCTime currentTime;
  RTC.getTime(currentTime);
  std::time_t currentUnixTime;

  currentUnixTime = currentTime.getUnixTime();

  //有効期限を当日指定のunix秒に変換
  struct tm yuko_t;
  time_t yuko_unixTime;

  yuko_t.tm_sec = 00;    // 0-59
  yuko_t.tm_min = 00;    // 0-59
  yuko_t.tm_hour = KIGENGIRETOSURUJIKOKU_H;   // 0-23
  yuko_t.tm_mday = yukoKigen.tm_mday;   // 1-31
  yuko_t.tm_mon = yukoKigen.tm_mon;     // 0-11
  yuko_t.tm_year = yukoKigen.tm_year;  // 1900年からの経過年
  yuko_unixTime = mktime(&yuko_t); 
  

  if(SHOW_DEBUG){
      //★重要★現在時刻はポインター変数！！！！！
      
      char buffer[32];
      strftime(buffer, 32, "%F %T\n", localtime(&currentUnixTime));
      printf("CurrentTime = %s\r\n", buffer);
      
      strftime(buffer, 32, "%F %T\n", localtime(&yuko_unixTime));
      printf("Yuko-Kigen  = %s\r\n", buffer);
  }

  if(currentUnixTime < yuko_unixTime){
      if(SHOW_DEBUG){
          printf("You can drive!\r\n");
      }
      return true;
  }else{
      if(SHOW_DEBUG){
          printf("You cannot drive!\r\n");
      }
      return false;
  }
}


/*************/
/*** 時間系 ***/
/*************/

//免許証の日付は、上位ビット・下位ビットで2桁を表す「パック2進化10進数」形式なのでintへ変換
int packedBCDtoInt(uint8_t input){
  uint8_t ten,one;
  int out = 0;
  
  ten = (input >> 4) & 0x0F;
  one = input & 0x0F;
  out = 10 * (int)ten + (int)one;

  return out;
}

//免許書のパック2進化10進数形式の日付データをtm構造体へ変換
tm packedBCDdata_to_tmStruct(uint8_t yyyymmdd[]){
  int year;
  int month;
  int day;
  tm out_tm;
  
  year = 100 * packedBCDtoInt(yyyymmdd[0]) + packedBCDtoInt(yyyymmdd[1]);
  month = packedBCDtoInt(yyyymmdd[2]);
  day = packedBCDtoInt(yyyymmdd[3]);
    
  out_tm.tm_year = year - 1900;   //1900年からの経過年
  out_tm.tm_mon = month - 1;  //1月からの月数 (0 ～ 11)
  out_tm.tm_mday = day;
  
  return out_tm;
}

//共通データ要素の中身を整理して構造体へ保存
mfDataStruct mfData_toStruct(std::vector<uint8_t> cardRes){
  mfDataStruct mf_Data;
  for(int i = 0; i < sizeof(mf_Data.shiyoVer);i++){
      mf_Data.shiyoVer[i] = cardRes[i + 2];
  }
  for(int i = 0; i < sizeof(mf_Data.kofuYYYYMMDD);i++){
      mf_Data.kofuYYYYMMDD[i] = cardRes[i + 5];
  }
  for(int i = 0; i < sizeof(mf_Data.yukoYYYYMMDD);i++){
      mf_Data.yukoYYYYMMDD[i] = cardRes[i + 9];
  }
  return mf_Data;
}

//RTCが設定されているか確認
bool isSetRTC(){
  time_t currentUnixTime;
  currentUnixTime = time(NULL); //unix時刻でRTCを取得
  if(localtime(&currentUnixTime)->tm_year == 70){ //1970年だったら未設定
      return false;
  }else{
      return true;
  }
}


/***************/
/*** 表示機能 ***/
/***************/

//ATQBレスポンスを整理して表示
void debugPrintATQB(uint8_t cardRes[]){
  if(SHOW_DEBUG){
      printf("--- TypeB_inListPassiveTarget_Res ---\r\n");
      printf(" Tg = %02X\r\n",cardRes[0]);
      printf(" ---------------ATQB----------------\r\n");
      printf(" FirstByte = %02X\r\n",cardRes[1]);
      printf(" PUPI = ");
      for(int j = 2; j<=5;j++){
          printf("%02X ",cardRes[j]);
      }
      printf("\r\n");
      printf(" AppData = ");
      for(int j = 6; j<=9;j++){
          printf("%02X ",cardRes[j]);
      }
      printf("\r\n");
      printf(" ProtocolInfo = ");
      for(int j = 10; j<=12;j++){
          printf("%02X ",cardRes[j]);
      }
      printf("\r\n");
      printf(" ------------END of ATQB------------\r\n");  
      printf(" ATTRIB_LEN = %02X\r\n",cardRes[13]);
      printf(" ATTRIB_RES = ");
      for(int j = 14; j<14+(int)cardRes[13];j++){
          printf("%02X ",cardRes[j]);
      }
      printf("\r\n");
      printf("------------END of RES------------\r\n");
  }
}

//共通データ要素を整理して表示
void debugPrintMFdata(std::vector<uint8_t> cardRes){
  if(SHOW_DEBUG){
      printf("\r\n---------------------MF DATA--------------------\r\n");
      printf("\r\n------------Card Hakko-sha Data------------\r\n");
      printf(" Tag\r\n  %02X\r\n",cardRes[0]);
      printf(" LEN\r\n  %02X\r\n",cardRes[1]);
      printf(" Shiyo-sho Ver\r\n  ");
      for(int i = 2; i <= 4 ;i++){
          printf("%02X ",cardRes[i]);
      }
      printf("\r\n Ko-hunengappi\r\n  ");
      for(int i = 5; i <= 8; i++){
          printf("%02X ",cardRes[i]);
      }
      printf("\r\n Yu-ko-kigen\r\n  ");
      for(int i = 9; i < 12; i++){
          printf("%02X ",cardRes[i]);
      }
      printf("\r\n");
      printf("\r\n------------Card Hakko-mae Data------------\r\n");
      printf(" Tag\r\n  %02X\r\n",cardRes[13]);
      printf(" LEN\r\n  %02X\r\n",cardRes[14]);
      printf(" Seizo-gyo-sha shikibetsushi\r\n  %02X\r\n",cardRes[15]);
      printf(" Ango-kansu- shikibetsushi\r\n  %02X\r\n",cardRes[16]);
      printf("-----------------End of MF DATA-----------------\r\n\n");
  }
}
