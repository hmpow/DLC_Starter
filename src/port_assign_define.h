#ifndef PORT_ASSIGN_DEFINE_H
#define PORT_ASSIGN_DEFINE_H

//デジタル入力 割り込み必要
#define PORT_A_DEF_BOOT_MODE            D2
#define PORT_A_DEF_ENGINE_START_MONI    D3

//デジタル出力
#define PORT_A_DEF_START_CTRL_RELAY_OUT D7
#define PORT_A_DEF_SELF_RESET_OUT       D14

//ライブラリが使うポート
//D0,D1　  RC-S660/S シリアル通信　
//D10～D13 ATP301x   SPI通信

//空けておきたいポート
//D18,D19  ATP301x  I2C通信
//始動制御をリレーではなくCAN通信にする場合、ポート重複するSPIは使えなくなる


#endif // PORT_ASSIGN_DEFINE_H