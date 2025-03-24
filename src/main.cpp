//ToDo：割り込み連打するとUART受信フリーズ（時間経過で復活）
//要調査：wifi起動したら音声合成使用禁止　フリーズする　リソース競合？

//ドライバー番号は1始まりにすること
//設定画面で不正値を渡された際に、toIntが文字とかゼロにしてくれるので判定が楽

#include <Arduino.h>
#include <RTC.h>
#include <EEPROM.h>

#include "main_setting.h"

#include "StartCtrl.h"
#include "rcs660s_app_if.h"
#include "ATP301x_Arduino_SPI.h"

#include "jpdlc_conventional.h"
#include "jpdlc_mynumbercard.h"

#include "pinEEPROM.h"

#include "port_assign_define.h"
#define ENGINE_START_MONITOR_PIN  PORT_A_DEF_ENGINE_START_MONI 
#define BOOT_MODE_PIN             PORT_A_DEF_BOOT_MODE

#define SHOW_DEBUG  false
#define DEVELOP_MODE false

#define EXPIRATION_HOUR_THRESHOLD      12 //有効期限当日の何時を期限切れとするか
#define REMAINING_COUNT_ALART_THRESHOLD 10 //残り照合回数が下回ったら警告する閾値



const bool USE_ATP301X = true;
const uint8_t DRIVER_LIST_NUM = 3;

/* 割り込みから操作 */
volatile uint8_t driverNum;
volatile unsigned long lastIntruuptTime;
volatile unsigned long lastIntruuptTimeEg;


/******************/
/* プロトタイプ宣言 */
/******************/

void intrruptFunc_ChangeDriver(void);
void intrruptFunc_EgStartMoni(void);
void allowDrive();
void disallowDrive();
//void audioOn();
//void audioOff();

//単発アナウンス
void announcePleaseTouch();
void announcePleaseRetry();
void announceExpirationTime(JPDLC_EXPIRATION_DATA);
void announceCurrentTime();
void announcePleaseSavePinOrCheckDriverSelect();

void announceCheckPin();
void announceResetRemainingCount();

bool isEfectiveLicenseCard(JPDLC_EXPIRATION_DATA);

void printRTCtime(void);
void setupRTC(void);



void main_normalMode_setup(void);
void main_normalMode_loop(void);


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

PinEEPROM pinEEPROM;

char atpbuf[ATP_MAX_LEN];

/* メイン関数 */
void setup() {

  /*************/
  /* モード共通 */
  /*************/

  startCtrl.setup(); //どちらのモードでも禁止側に初期化必要

  pinMode(BOOT_MODE_PIN, INPUT);//起動モード選択

  pinMode(RESET_OUT_PIN, OUTPUT);//リセット端子駆動
  digitalWrite(RESET_OUT_PIN, LOW);//リセット端子駆動

  Serial.begin(9600);

  atp301x.begin();

  setupRTC();

  /*************/
  /* モード切替 */
  /*************/

  //起動モード判定
  if(digitalRead(BOOT_MODE_PIN) == LOW){
    bootMode = SETTING;
  }else{
    bootMode = NORMAL;
  }

  #if DEVELOP_MODE
    //delay(10000); //Platform IO がアップロードタスクからシリアルモニタタスクに戻るのを待つ
    Serial.println("EEPROMのデータ");
    pinEEPROM.debugPrintEEPROM(24);
  #endif


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

void main_normalMode_setup(void){

  pinMode(ENGINE_START_MONITOR_PIN, INPUT);//エンジンスタートSW監視
  driverNum = 1;
  lastIntruuptTime = 0;
  lastIntruuptTimeEg = 0;
  
  rcs660sAppIf.begin();

  setReaderInstance(&rcs660sAppIf); //jpdlc_base_reader_if.h にリーダー渡す

  #if DEVELOP_MODE
    //アナウンス
    sprintf(atpbuf,"tsu-jo-mo'-dode/kido-shima'_su.");
    atp301x.talk(atpbuf,true);
    Serial.println("通常モード");
  #endif

  return;
}


void main_normalMode_loop() {
//起動処理
  disallowDrive();
  //audioOn();
  
  bool isDriveAllowed = false;
  bool readError = false;

  //インスタンス
  JpDrvLicNfcCommandConventional jpdlcConventional;
  JpDrvLicNfcCommandMynumber jpdlcMyNumberCard;
  
  //基底クラスは純粋仮想関数を持つためポインタ変数でしか置けない
  JpDrvLicNfcCommandBase *drvLicCard = &jpdlcMyNumberCard; //マイナ免許証優先で判定
  
  while(!isDriveAllowed){

      rcs660sAppIf.resetDevice();

      attachInterrupt(digitalPinToInterrupt(BOOT_MODE_PIN), intrruptFunc_ChangeDriver, FALLING );
      attachInterrupt(digitalPinToInterrupt(ENGINE_START_MONITOR_PIN), intrruptFunc_EgStartMoni, FALLING );

      //失敗してループされた場合は失敗フラグが立っているためアナウンスを切り替える
      if(readError){
        //音声合成「読み取りエラー　もう一度タッチしてください」
        announcePleaseRetry();
        readError = false;
      }else{
        //音声合成「免許証をタッチしてください」
        announcePleaseTouch();
      }

  
     //STEP1：NFC Type-Bをポーリング

     rcs660sAppIf.setNfcType(NFC_TYPE_B);
     rcs660sAppIf.updateTxAndRxFlag({false, false, 3, false});//従来免許はこれで読めるがマイナ免許は応答がない→マイナ免許もこれでいけた　タイムアウト短すぎた
     bool isCatch = rcs660sAppIf.catchNfc(RETRY_CATCH_INFINITE);

     /* catchNFCは無限ループに設定しているため捕捉できるまで抜けてこない */

     //カードと通信中はスイッチ割り込み無効
     detachInterrupt(digitalPinToInterrupt(BOOT_MODE_PIN));
     detachInterrupt(digitalPinToInterrupt(ENGINE_START_MONITOR_PIN));
     /* noInterrupts() 使うとUART受信割り込みまで止まってしまいNG */


    if(USE_ATP301X){
      //タッチ音「ポーン」
      atp301x.chimeJ(false);
    }else{
      //ビープ「ピッ」
      // beep.oneshotOn(0.1);    
    }

    //STEP2 仕様で規定された3つのAIDが存在するかチェック

    if(SHOW_DEBUG){
      printf("\r\n\r\n =============カードアクセスシーケンス開始============== \r\n\r\r");
    }

    //選択された免許種別でダメだったら切り替えてリトライ
    if(!drvLicCard->isDrvLicCard()){

      if(drvLicCard == &jpdlcMyNumberCard){
        drvLicCard = &jpdlcConventional;  //マイナ免許証でない場合は従来免許証に切り替え
        if(SHOW_DEBUG){
          Serial.println("マイナ免許証でないため従来免許証に切り替え");
        }
      }else{
        drvLicCard = &jpdlcMyNumberCard;  //従来免許証でない場合はマイナ免許証に切り替え
        if(SHOW_DEBUG){
          Serial.println("従来免許証でないためマイナ免許証に切り替え");
        }
      }
      
      //免許証種別を切り替えてもダメなら諦める
      if(!drvLicCard->isDrvLicCard()){
        if(SHOW_DEBUG){
          Serial.println("免許証ではないためリトライ");
        }
        readError = true;
        delay(10000);
        continue;
      }else{
        //OK 次STEPへ
      }
    }else{
      //OK 次STEPへ
    }

    //マイナンバーの場合のみ 暗証番号照合
    if(drvLicCard == &jpdlcMyNumberCard){
      printf("マイナンバーモード\r\n");
      printf("PIN設定有無の確認\r\n");
      
      //PIN設定有無確認
      uint8_t isSetPinMyum = drvLicCard->issetPin();
      printf("PIN設定有無：%d\r\n", isSetPinMyum);
      

      //Verify実行

      if(isSetPinMyum){
        //独自のPINが設定されている

        if(!pinEEPROM.isSetPin(driverNum - 1)){
          //EEPROMにPIN設定がない場合
          if(SHOW_DEBUG){
            printf("EEPROMにPIN設定が保存されていない\r\n");
          }
          announcePleaseSavePinOrCheckDriverSelect();
          
          continue;
        }

        if(SHOW_DEBUG){
          printf("EEPROMにPIN設定あり\r\n");
        }
 
        //残り照合回数確認
        uint8_t mynumcount = drvLicCard->getRemainingCount();
        printf("残り照合回数：%d\r\n", mynumcount);

        if(mynumcount < REMAINING_COUNT_ALART_THRESHOLD){
          //【無限ループ】残り回数が少ないアナウンス
          while(1){
            announceResetRemainingCount();
          }
        }

        type_EEPROM_PIN pinDecimal = pinEEPROM.getPin(driverNum - 1);
    
        if(DEVELOP_MODE){
          sprintf(atpbuf,"pinnwa <NUMK VAL=%d > <NUMK VAL=%d > <NUMK VAL=%d > <NUMK VAL=%d >.",
            pinDecimal[0],pinDecimal[1],pinDecimal[2],pinDecimal[3]);
          atp301x.talk(atpbuf,true);
        
          printf("PIN_decimal_HEX: %02X %02X %02X %02X\n",
            pinDecimal[0],pinDecimal[1],pinDecimal[2],pinDecimal[3]);
        
          atp301x.talk("pi'nnga/hozonnsareteima'_su.");
    
          for(int i = 5; i >= 0; i--){
            printf("PIN照合待機：%d秒\r\n",i);
            sprintf(atpbuf,"<NUMK VAL=%d >.",i);
            atp301x.talk(atpbuf,false);
            delay(1000);
          }
        }
    
        bool isVerified = drvLicCard->executeVerify_DecimalInput(pinDecimal);
    
        if(isVerified){
          if(DEVELOP_MODE){
            atp301x.talk("berifa'i se-ko-.");
            printf("PIN照合成功\r\n");
          }
        }else{
          rcs660sAppIf.releaseNfc();
          
          //【無限ループ】暗証番号間違いアナウンス
          while(1){
            announceCheckPin();
          }    
        }

      }else{
        //独自のPINが保存されていない
        if(SHOW_DEBUG){
          printf("PIN未設定のためDPINで照合\r\n");
        }

        type_PIN pinDpin = {DPIN,DPIN,DPIN,DPIN};
        if(DEVELOP_MODE){
          for(int i = 5; i >= 0; i--){
            printf("PIN照合待機：%d秒\r\n",i);
            sprintf(atpbuf,"<NUMK VAL=%d >.",i);
            atp301x.talk(atpbuf,false);
            delay(1000);
          }
        }

        bool isVerified = drvLicCard->executeVerify(pinDpin);
      }

    }//マイナンバーの場合のみシーケンス終了


    //ベリファイ失敗時は「ご確認ください」アナウンス無限ループに入るため有効期限確認には来ない
    //有効期限読み出し入る段階でベリファイ成功している前提でよい

    //STEP3 有効期限チェック


    //従来・マイナ兼用
    JPDLC_EXPIRATION_DATA expirarionData = drvLicCard->getExpirationData();
    if(expirarionData.yyyy == 0){
      readError = true;
      continue;
    }

    if(isEfectiveLicenseCard(expirarionData)){
        allowDrive();
        
        //ループ終わり
        isDriveAllowed = true;

    }else{
      if(USE_ATP301X){
        atp301x.chimeK(false);
        delay(200);
        atp301x.chimeK(false);
        delay(200);
        atp301x.chimeK(false);
        delay(200);
      
        //音声合成「有効期限 または 設定をご確認ください」
        atp301x.talk("yu-ko-ki'genn mata'wa sette-o gokakuninnkudasa'i.");
        announceCurrentTime();    
      }
    }
      
    //有効期限読み上げ
    delay(1000); //デバッグメッセージ出さないと読み取り速すぎてタッチ音が「ポーン」ではなく「ポッ」って聴こえる
    announceExpirationTime(expirarionData);

    //audioOff();
    
    rcs660sAppIf.releaseNfc();

    delay(1000);
  } //while終わり

  while(1){
    //ここへ来るのはエンジンがかかって走行中
    // 無限ループで止めておく 将来的にはマイコンスリープにしたい
  }

  return;
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
      (int)currentTime.getYear(), (int)Month2int(currentTime.getMonth()), (int)currentTime.getDayOfMonth());
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

void announcePleaseSavePinOrCheckDriverSelect(){
  if(USE_ATP301X){
    //音声合成「設定モードから暗証番号するか、登録済みドライバーを選択してください」
    atp301x.talk("a'nsho-ba'nngo-o/se'teisuru'ka, dora'iba-banngo-o ka'kuninne'kudasai.",true);
    
    //「現在のドライバー選択はXXです」
  }
  return;
}


//無限ループアナウンス：暗証番号照合失敗
void announceCheckPin(){
    if(USE_ATP301X){
      //音声合成「暗証番号が間違っています。カード保護のため動作を停止しました。暗証番号とドライバー選択を確認してください。」
      atp301x.talk("a'nsho-ba'nngo-o/go'kakuninne'kudasai.",true);
      //音声合成「現在のドライバー選択は　__　です。」


      //音声合成「リセットボタンまたは電源入れ直しで再起動します。」

    }
  return;
}

//無限ループアナウンス：残り照合回数が少ない
void announceResetRemainingCount(){
  if(USE_ATP301X){
      //音声合成「残り照合回数が少なくなっています。カード保護のため動作を停止しました。」
      atp301x.talk("nokorisho-go-ka'isu-ga/sukunakunatteimasu.",true);
      //音声合成「公式のマイナ免許読み取りアプリで正しい暗証番号で照合し、残り照合回数をリセットしてください。」
      //音声合成「リセットボタンまたは電源入れ直しで再起動します。」
  }
  return;
}



#if 0
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
#endif

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


void intrruptFunc_ChangeDriver(){
  //DJごっこして遊んでいるとUARTがしばらくフリーズする
  unsigned long lastlast = lastIntruuptTime;
  lastIntruuptTime = millis();
  if((lastIntruuptTime - lastlast) < 500){
    return;
  }

  /*チャタリングではない*/

  driverNum++;
  if(driverNum > DRIVER_LIST_NUM){
    driverNum = 1;
  }
  if((lastIntruuptTime - lastlast) < 1500){
    sprintf(atpbuf,"<ALPHA VAL=%d>.",driverNum);
    atp301x.talk(atpbuf,false);
    return;
  }
  /*連打ではない*/
  sprintf(atpbuf,"mainame'nnkyo dora'iba-<ALPHA VAL=%d>.",driverNum);
  atp301x.talk(atpbuf,false);
  return;
 
}

void intrruptFunc_EgStartMoni(){
  //DJごっこして遊んでいるとUARTがしばらくフリーズする
  unsigned long lastEg = lastIntruuptTimeEg;
  lastIntruuptTimeEg = millis();
  if((lastIntruuptTimeEg - lastEg) < 500){
    return;
  }else{
    announcePleaseTouch();
  }
  return;
 
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

