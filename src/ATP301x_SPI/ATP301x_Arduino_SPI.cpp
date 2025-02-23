#include "Atp301x_Arduino_SPI.h"

//public function

//コンストラクタでSPI.begin()を呼び出す構成にするとグローバル変数定義時(setupより前)に呼ばれる可能性あるため明示的に用意
void ATP301x_ARDUINO_SPI::begin(){
    pinMode(nSS_PIN, OUTPUT); //SS
    ss_inactive();
    SPI.begin();
    SPI.beginTransaction(SPISettings(SPI_CLK_HZ, MSBFIRST, SPI_MODE_SELECT));
    return;
}
    
    
void ATP301x_ARDUINO_SPI::stop(){
    const char stop1 = '>';
    int rx;

    ss_active();

    //ブレークコマンド $
    rx = SPI.transfer('$');
    delayMicroseconds(SPI_SEND_PERIOD_US);
     
    //breakコマンドに対する応答”E255”がバッファに溜まってるのでダミーデータを送って">"が返ってくるまで取出す
    //データシート「ブレークコマンド」参照
    while(rx != (int)stop1){
        rx = SPI.transfer(0xFF);
        delayMicroseconds(SPI_SEND_PERIOD_US);
    }

    ss_inactive();
    return;
}

//ATP301xの音声記号を受け取ってしゃべる
void ATP301x_ARDUINO_SPI::talk(char input[], bool useWait){
       
    stop();
    
    bool tooLong = false;
    int i = 0;
    
    ss_active();

    while(input[i] != 0xFF && input[i] != '\0' && input[i] != NULL){
        SPI.transfer(input[i]);
        delayMicroseconds(SPI_SEND_PERIOD_US);
            
        //発音記号の終端（ピリオド）を送ったらbreak
        //※while終了条件に組み込むと、Whileの外で.を送ることになりチャイムが鳴らせない
        if(input[i] == '.'){
            break;
        }
            
        i++;
            
        //ATP301xが受信できる最大文字数を超えたら強制終了
        if(i > ATP_MAX_LEN){
            tooLong = true;
            break;
        }
    }
    //ATP301xコマンドの終端を示すキャレッジリターンを送信
    SPI.transfer('\r');

    ss_inactive();
        
    if(useWait){
        atp_wait();
    }
    
    if(tooLong){
        //音声合成「音声記号が長すぎます。」
        talk("onnse-ki'go-ga/nagasugima'_su.",true);
    }
    return;
}

void ATP301x_ARDUINO_SPI::chimeJ(bool useWait){
    talk("#J", useWait);
    return;
}

void ATP301x_ARDUINO_SPI::chimeK(bool useWait){
    talk("#K", useWait);
    return;
}

//private functin
    
//発音完了まで別の処理をさせたくない場合にwaitでマイコン全体を止める
//ダミーデータを送信し、Busyである（＝発音中）ならばwait()
//ATP301xに発話完了時のアクションはなく
//”Ready待ちはポーリングで行う。”とデータシートに記載あり
//データはシート「SPI通信」参照

void ATP301x_ARDUINO_SPI::atp_wait(){
    const char busyRes = '*'; //Busy時の返ってくるはずのレスポンス
    int rx;

    ss_active();   
          
    //ダミーデータを送信してポーリング
    do{
        delay(POLLING_CYCLE_MS);
        rx = SPI.transfer(0xFF);
    }while(rx == busyRes);

    ss_inactive();

    return;
}

void ATP301x_ARDUINO_SPI::ss_active(){
    digitalWrite(nSS_PIN, LOW);
    delayMicroseconds(SPI_SEND_PERIOD_US);
    return;
}

void ATP301x_ARDUINO_SPI::ss_inactive(){
    digitalWrite(nSS_PIN, HIGH);
    delayMicroseconds(SPI_SEND_PERIOD_US);
    return;
}