#ifndef STARTCTRL_H
#define STARTCTRL_H

#include <Arduino.h>

#include "port_assign_define.h"
// デジタル出力の場合の設定
#define START_CTRL_PIN PORT_A_DEF_START_CTRL_RELAY_OUT
#define ALLOW_LOGIC HIGH
#define DENY_LOGIC  LOW


/*************/
/* 基底クラス */
/*************/

class StartCtrl
{
    public:
        virtual ~StartCtrl(); //基底クラスのデストラクタにvertual付けると良いらしい

        virtual void setup(void);    

        //サブクラスで状態変数更新を使忘れないためfinal化してシーケンス部分のみオーバーライドさせる
        virtual void allow (void) final;
        virtual void deny (void) final;
        virtual bool isStartable (void) final;
    private:
        virtual void allowSequence(void);
        virtual void denySequence(void);
        bool is_startable;
};


/*************/
/* 派生クラス */
/*************/

// エンジン車想定
class StartCtrl_DigitalOut : public StartCtrl
{
    public:
        void setup(void) override;
    private:
        // HW制御シーケンス部分のみオーバーライド
        void allowSequence(void) override;
        void denySequence(void) override;
};

#if 0
// 電気自動車など想定
class StartCtrl_Can : public StartCtrl
{
    public:
        void setup(void);
    private:
        // HW制御シーケンス部分のみオーバーライド
        void allowSequence(void);
        void denySequence(void);
};
#endif

#endif // STARTCTRL_H