/*
 * board_config.c - 开发板硬件配置实现
 * 功能：实现开发板初始化函数
 * 路径：boards/AT89C52/board_config.c
 */

#include "board_config.h"

/*
 * 函数：LED_Init
 * 功能：初始化LED相关硬件
 * 参数：无
 * 返回：无
 */
void LED_Init(void)
{
    // LED连接P0口，初始状态全部熄灭（输出高电平）
    LED_PORT = 0xFF;
    
    // 可选：设置P0口为推挽模式（传统8051默认是开漏，需外接上拉）
    // 在Proteus仿真中，P0口驱动LED需外接上拉电阻，或使用共阳接法
}

/*
 * 函数：KEY_Init
 * 功能：初始化按键相关硬件
 * 参数：无
 * 返回：无
 */
void KEY_Init(void)
{
    // 设置P1口为输入状态（先输出高电平，使能内部上拉）
    KEY_PORT = 0xFF;
}

/*
 * 函数：Board_Init
 * 功能：开发板总初始化
 * 参数：无
 * 返回：无
 */
void Board_Init(void)
{
    LED_Init();     // 初始化LED
    KEY_Init();     // 初始化按键
}