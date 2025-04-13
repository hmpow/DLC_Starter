#include "dlcRTC.h"

void setupRTC(void){
    // setTimeIfNotRunningはRTCの開始と時刻設定が一体化しているAPI仕様のため
    // 一旦取得して再度設定という動きをしないと毎リセットごとに時計が初期化されてしまう
  
    RTC.begin(); // RTCの初期化　これだけではRTC動き始めない
    debugPrintCurrentRtcTime(); //表示
  
    // RTCから現在時刻を取得 バッテリバックアップされている値or初期値が取れる
    RTCTime rtcTime;
    RTC.getTime(rtcTime);
  
    //もし2000年(初期値)ならば、必ず有効期限切れになるように未来を設定
    //2000年のままだと有効期限が全部OKになってしまうため
    if(rtcTime.getYear() == 2000){
      rtcTime.setYear(2090);
    }
  
    RTC.setTimeIfNotRunning(rtcTime); // 現在時刻を引き継いでRTCをスタート
  
    debugPrintCurrentRtcTime(); //表示

    return;
  }


//免許証有効期限チェック
bool isEfectiveLicenseCard(
    JPDLC_EXPIRATION_DATA exData, uint8_t exHourTh){

    //エラーチェック
    if(exData.yyyy == 0){
        // 0年 をエラーコードと扱う
      return false;
    }
    if(exHourTh < 0 || 24 < exHourTh){
      return false;
    }
  
    RTCTime expirationTime;
    expirationTime.setYear(exData.yyyy);
    expirationTime.setMonthOfYear((Month)(exData.m - 1));//0始まりのenumになっているので1引くこと
    expirationTime.setDayOfMonth(exData.d);
    expirationTime.setHour(exHourTh);
    expirationTime.setMinute(0);
    expirationTime.setSecond(0);
  
    RTCTime currentTime;
    RTC.getTime(currentTime);
    
    //unix秒に変換
    uint64_t currentUnixTime    = (uint64_t)currentTime.getUnixTime();
    uint64_t expirationUnixTime = (uint64_t)expirationTime.getUnixTime();
  
#ifdef DLC_RTC_DEBUG
    Serial.println("dlcRTC :: isEfectiveLicenseCard : CURRENT Time");
    debugPrintRtcTime(currentTime);

    Serial.println("dlcRTC :: isEfectiveLicenseCard : EXPIRATION Time");
    debugPrintRtcTime(expirationTime);
#endif
    
    if(currentUnixTime < expirationUnixTime){
        return true;
    }else{
        return false;
    }
  }

void updateRtcSetting(uint16_t yyyy, uint8_t mm, uint8_t dd, uint8_t hh, uint8_t mi, uint8_t ss){
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
        debugPrintCurrentRtcTime();
    }
}

void debugPrintRtcTime(RTCTime rtcTime){

    Serial.print("RTC : UNIX time: ");
    Serial.println((uint64_t)rtcTime.getUnixTime());
  
    Serial.print("RTC : YYYY/MM/DD - HH/MM/SS:");
    Serial.print(rtcTime.getYear());
    Serial.print("/");
    Serial.print(Month2int(rtcTime.getMonth()));
    Serial.print("/");
    Serial.print(rtcTime.getDayOfMonth());
    Serial.print(" - ");
    Serial.print(rtcTime.getHour());
    Serial.print(":");
    Serial.print(rtcTime.getMinutes());
    Serial.print(":");
    Serial.println(rtcTime.getSeconds());
    Serial.print("\n");
    return;
  }


void debugPrintCurrentRtcTime(void){
    #ifdef DLC_RTC_DEBUG

    RTCTime currentTime;
    bool running = RTC.isRunning();

    if(running){
        Serial.println("RTC : Running");
    }else{ 
        Serial.println("RTC : notRunning");
    }

    RTC.getTime(currentTime);
    debugPrintRtcTime(currentTime);
    
    Serial.println("RTC : CurrentTime");

    #endif

    return;
}

  
  