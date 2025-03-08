#include "StartCtrl.h"

StartCtrl::StartCtrl()
{
    isStartable = false;
}

void StartCtrl::setup()
{
    // 何もしない
}

void StartCtrl::allow()
{
    allowSequence();
    isStartable = true;
}

void StartCtrl::deny()
{
    denySequence();
    isStartable = false;
}

bool StartCtrl::isStartable()
{
    return isStartable;
}

