#ifndef ATP301X_ARDUINO_SPI_H
#define ATP301X_ARDUINO_SPI_H

//#include "mbed.h"
#include <Arduino.h>
#include <SPI.h>

//https://docs.arduino.cc/language-reference/en/functions/communication/SPI/

/** 動作パラメータ設定
* ※カッコ内はデータシートに記載された限度値です。
*/
 
//最大文字数制限 (<= 127バイト-2バイト('.'＋'\r'))
#define ATP_MAX_LEN 125

//発話中ポーリング周期(ms) (>= 10ms)
#define POLLING_CYCLE_MS 20

//SPI通信クロック (<= 1MHz)
#define SPI_CLK_HZ 500000

//SPI送信周期 (>= 20μs)
#define SPI_SEND_PERIOD_US 25

//SPI通信モード (SPI_MODE0 or SPI_MODE3) 
#define SPI_MODE_SELECT SPI_MODE0

//SS端子は手動操作が必要？
#define nSS_PIN 10

//SPI仕様端子
// T.B.D. 一旦D10～D13固定

class ATP301x_ARDUINO_SPI{
public:
    /** 
     * @brief ATP301x_SPIクラスのSPI通信を初期化します。
     */
    void begin();
    
    /**
     * @brief breakコマンドを送信し発話中断します。ATP301xが次のコマンドを受け付けられる状態になる(Readyになる)までwait()します。
     */
    void stop();
    
    /**
     * @brief ATP301xに音声コマンドを送信します。NULL/\o/0xFF終端(音声記号を受け取った場合、文末記号であるピリオドでbreak)に対応します。
     * ATP301xへ送信するコマンド終端を表す最後のキャレッジリターンは自動付与します。
     * @param input[] ATP301xへ送信するデータのchar配列(音声記号 または 制御コマンド)
     * ※音声記号が ATP_MAX_LEN で指定する文字数を超えた場合、ATP301xが受け取れる文字数を超えてしまうため送信を強制終了します。
     * 文字数オーバーとなった場合、運よく音声記号が成立する位置でカットされた場合は途中まで発話されますが、
     * 音声記号が成立しない位置でカット（例：子音で終了）された場合は警告メッセージのみが発話されます。
     * @param useWait コマンド送信後、送ったコマンドを発話終了まで(＝ATP301xがReadyになるまで) waitするか指定(デフォルト:true)
     * ※割り込み等で発話中にstop()が呼ばれた場合、useWaitによらずbreakコマンドを送信し発話を中止します。
     */
    void talk(char input[], bool useWait = true);
    
    /**
     * @brief ATP301xの内蔵チャイムJを鳴らします。
     * @param useWait trueの場合、チャイム再生終了まで(＝ATP301xがReadyになるまで) wait()します。(デフォルト:true)
     */
    void chimeJ(bool useWait = true);
    
    /**
     * @brief ATP301xの内蔵チャイムKを鳴らします。
     * @param useWait trueの場合、チャイム再生終了まで(＝ATP301xがReadyになるまで) wait()します。(デフォルト:true)
     */
    void chimeK(bool useWait = true);

private:
    void atp_initialize();
    void atp_wait();
    void ss_active();
    void ss_inactive();
};
#endif //ATP301X_ARDUINO_SPI_H