#ifndef STARTCTRL_H
#define STARTCTRL_H

#include <Arduino.h>

class StartCtrl
{
    public:
        vertual void setup();
        //サブクラスで状態変数更新を使忘れないためfinal化してシーケンス部分のみオーバーライドさせる
        void allow final (void);
        void deny final (void);
        void isStartable final (void);
        virtual ~StartCtrl();
    private:
        vertual void allowSequence();
        vertual void denySequence();
        bool isStartable;
};

// エンジン車想定
class StartCtrl_DigitalOut : public StartCtrl
{
    public:
        void setup();
        // allow と deny は親クラスで実装
        // ~StartCtrlDigitalOut() は自動実装に任せる
    private:
        void allowSequence();
        void denySequence();
};


// 電気自動車など想定
/*
class StartCtrl_Can : public StartCtrl
{
    public:
        StartCtrlSerial();
        void setup();
        void allow();
        void deny();
        // ~StartCtrlSerial() は自動実装に任せる
};
*/

#endif // STARTCTRL_H