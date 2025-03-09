#include "StartCtrl.h"

/*************/
/* 基底クラス */
/*************/

StartCtrl::~StartCtrl()
{
    //何もしない 書かないとundefined reference toでコンパイルが通らない
}

void StartCtrl::allow(void)
{
    allowSequence();
    is_startable = true;
    return;
}

void StartCtrl::deny(void)
{
    denySequence();
    is_startable = false;
    return;
}

bool StartCtrl::isStartable(void)
{
    return is_startable;
}

/*************/
/* 派生クラス */
/*************/

void StartCtrl_DigitalOut::setup(void)
{
    pinMode(START_CTRL_PIN, OUTPUT);//リセット端子駆動
    deny();
    return;
}

void StartCtrl_DigitalOut::allowSequence(void)
{
    digitalWrite(START_CTRL_PIN, ALLOW_LOGIC);
    return;
}

void StartCtrl_DigitalOut::denySequence(void)
{
    digitalWrite(START_CTRL_PIN, DENY_LOGIC);
    return;
}
