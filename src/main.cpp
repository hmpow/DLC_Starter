//ChatGPTの提案とArduinoのサンプルプログラムを合体してベースを作成

#define TEST_WAIT_HUMAN_READABLE_INTERVAL_MS 50


#define USE_ATP301X true

#define SHOW_DEBUG true

#define EXPIRATION_HOUR_THRESHOLD 12

#include <Arduino.h>
#include <WiFiS3.h>
#include <RTC.h>

#include "arduino_secrets.h"
#include "web_page.h"

#include "StartCtrl.h"
#include "rcs660s_app_if.h"
#include "ATP301x_Arduino_SPI.h"
#include "jpdlc_conventional.h"


#define executeReset() digitalWrite(D7, HIGH)

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
void announcePleaseRetry();
void announceExpirationTime(JPDLC_EXPIRATION_DATA);
void announceCurrentTime();

void igswRiseIrq();
void errorBeep(int);

//送信系
std::vector<uint8_t> sendSELECTbyAID(const uint8_t*, uint8_t );

//検査系
bool checkATQB(uint8_t*, int);
bool checkSELECTres(std::vector<uint8_t>);
bool checkREAD_BINARYres(std::vector<uint8_t>);

bool isEfectiveLicenseCard(JPDLC_EXPIRATION_DATA);

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

JpDrvLicNfcCommandConventional jpdlcConventional;

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

  setReaderInstance(&rcs660sAppIf); //jpdlc_base_reader_if.h にリーダー渡す

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

    //免許証判定STEP2 3つのAIDが存在するかチェック

    if(!jpdlcConventional.isDrvLicCard()){
      continue;
    }
    /*後でマイナ免許証判定を入れる*/


    //免許証判定STEP3 有効期限チェック

    JPDLC_EXPIRATION_DATA expirarionData = jpdlcConventional.getExpirationData();
    if(expirarionData.yyyy == 0){
      //読み取りエラー
      announcePleaseRetry();
      continue;
    }

    if(isEfectiveLicenseCard(expirarionData)){
        allowDrive();
        
        //ループ終わり
        canDrive = true;

    }else{
      if(USE_ATP301X){
        atp301x.chimeK(false);
        delay(200);
        atp301x.chimeK(false);
        delay(200);
        atp301x.chimeK(false);
        delay(200);
      }
      if(USE_ATP301X){
          //音声合成「有効期限 または 設定をご確認ください」
          atp301x.talk("yu-ko-ki'genn mata'wa sette-o gokakuninnkudasa'i.");
          announceCurrentTime();    
      }
    }
      
  //有効期限読み上げ
  announceExpirationTime(expirarionData);

  //カードリーダの電源OFF
  //rcs620.powerDown();

  //オーディオ回路スリープ
  //audioOff();

  rcs660sAppIf.releaseNfc();

  delay(1000);
  }//while終わり
}//main終わり

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

//リトライアナウンス
void announcePleaseRetry(){
  if(USE_ATP301X){
    //音声合成「読み取りエラー　もう一度タッチしてください」
    atp301x.talk("yomitorie'ra- mo-ichido ta'cchi/shitekudasa'i.",false);
  }
  return;
}

// カレンダーアナウンス
void announceCurrentTime(){
  if(USE_ATP301X){
    RTCTime currentTime;
    RTC.getTime(currentTime);

    //音声合成「時刻設定は」
    atp301x.talk("jiko'_kuse'tte-wa.\r\0");
    //音声合成「＿年＿月＿日」
    sprintf(atpbuf,"<NUMK VAL=%d COUNTER=nenn>/<NUMK VAL=%d COUNTER=gatu>/<NUMK VAL=%d COUNTER=nichi>.\r",
      (int)currentTime.getYear(), (int)Month2int(currentTime.getMonth()) + 1, (int)currentTime.getDayOfMonth());
    atp301x.talk(atpbuf);
    //音声合成「＿時＿分です」
    sprintf(atpbuf,"<NUMK VAL=%d COUNTER=ji>/<NUMK VAL=%d COUNTER=funn>/de_su.\r",
      (int)currentTime.getHour(), (int)currentTime.getMinutes());
    atp301x.talk(atpbuf);
    
    //音声合成「期限切れ判定閾値は、有効期限当日の＿時です」
    sprintf(atpbuf,"kigenngirehannte-_shikii'chiwa yu-ko-ki'genn/to-jitsuno <NUMK VAL=%d COUNTER=ji>de_su.\r",
      EXPIRATION_HOUR_THRESHOLD);
    atp301x.talk(atpbuf);
  }
  return;
}

// 有効期限アナウンス
void announceExpirationTime(JPDLC_EXPIRATION_DATA exData){
  if(USE_ATP301X){

    //音声合成「有効期限は＿年＿月＿日です」
    sprintf(atpbuf,"yu-ko-ki'gennwa <NUMK VAL=%d COUNTER=nenn>/<NUMK VAL=%d COUNTER=gatu>/<NUMK VAL=%d COUNTER=nichi>/de_su.\r",
      exData.yyyy, exData.m, exData.d);
    atp301x.talk(atpbuf,true);
  }
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

//免許証有効期限チェック
bool isEfectiveLicenseCard(JPDLC_EXPIRATION_DATA exData){

  //エラーチェック 0年 をエラーコードと扱う
  if(exData.yyyy == 0){
    return false;
  }

  RTCTime expirationTime;
  expirationTime.setYear(exData.yyyy);
  expirationTime.setMonthOfYear((Month)(exData.m - 1));//0始まりのenumになっているので1引くこと
  expirationTime.setDayOfMonth(exData.d);
  expirationTime.setHour(EXPIRATION_HOUR_THRESHOLD);
  expirationTime.setMinute(0);
  expirationTime.setSecond(0);

  RTCTime currentTime;
  RTC.getTime(currentTime);


  //unix秒に変換
  uint64_t currentUnixTime    = (uint64_t)currentTime.getUnixTime();
  uint64_t expirationUnixTime = (uint64_t)expirationTime.getUnixTime();

  if(SHOW_DEBUG){
    Serial.println("CURRENT Time: ");
    // Print out UNIX time
    Serial.print("UNIX time: ");
    Serial.println(currentUnixTime);
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

    Serial.println("EXPIRATION Time: ");
    // Print out UNIX time
    Serial.print("UNIX time: ");
    Serial.println(expirationUnixTime);
    Serial.print("YYYY/MM/DD - HH/MM/SS:");
    Serial.print(expirationTime.getYear());
    Serial.print("/");
    Serial.print(Month2int(expirationTime.getMonth()));
    Serial.print("/");
    Serial.print(expirationTime.getDayOfMonth());
    Serial.print(" - ");
    Serial.print(expirationTime.getHour());
    Serial.print(":");
    Serial.print(expirationTime.getMinutes());
    Serial.print(":");
    Serial.println(expirationTime.getSeconds());
  }
  
  if(currentUnixTime < expirationUnixTime){
      if(SHOW_DEBUG){
          Serial.println("You can drive!");
      }
      return true;
  }else{
      if(SHOW_DEBUG){
          Serial.println("You cannot drive!");
      }
      return false;
  }
}
