/*
 * board_config.h - 开发板硬件配置文件
 * 功能：定义AT89C52开发板的引脚映射、外设连接
 * 路径：boards/AT89C52/board_config.h
 */

#ifndef __BOARD_CONFIG_H__
#define __BOARD_CONFIG_H__

#include "system_config.h"   // 引用系统配置

// ==================== LED引脚定义（P0口，共阳接法）====================
// 根据Proteus电路图：LED阳极接VCC，阴极接P0口
// 所以：输出LOW(0)点亮LED，输出HIGH(1)熄灭LED

#define LED_PORT        P0      // LED所在端口

sbit LED1 = P2^0;       // D1 - 红色LED
sbit LED2 = P2^1;       // D2 - 红色LED
sbit LED3 = P2^2;       // D3 - 红色LED
sbit LED4 = P2^3;       // D4 - 红色LED

// LED控制宏
#define LED1_ON()    LED1 = 0    // 点亮
#define LED1_OFF()   LED1 = 1    // 熄灭
#define LED1_TOGGLE() LED1 = ~LED1

#define LED2_ON()    LED2 = 0
#define LED2_OFF()   LED2 = 1
#define LED2_TOGGLE() LED2 = ~LED2

#define LED3_ON()    LED3 = 0
#define LED3_OFF()   LED3 = 1
#define LED3_TOGGLE() LED3 = ~LED3

#define LED4_ON()    LED4 = 0
#define LED4_OFF()   LED4 = 1
#define LED4_TOGGLE() LED4 = ~LED4

// 批量操作宏
#define LED_ALL_ON()    LED_PORT &= 0xF0    // 低4位置0
#define LED_ALL_OFF()   LED_PORT |= 0x0F    // 低4位置1
#define LED_SET(v)      LED_PORT = (LED_PORT & 0xF0) | ((v) & 0x0F)

// ==================== 按键引脚定义（P1口低4位）====================
// 按键按下时引脚为LOW(0)，释放时为HIGH(1)

#define KEY_PORT        P1      // 按键所在端口

sbit KEY1 = P2^4;       // 开关1 - 控制LED1
sbit KEY2 = P2^5;       // 开关2 - 控制LED2
sbit KEY3 = P2^6;       // 开关3 - 控制LED3
sbit KEY4 = P2^7;       // 开关4 - 控制LED4

// 按键状态读取宏
#define KEY1_PRESSED()  (KEY1 == 0)    // 返回1表示按下
#define KEY2_PRESSED()  (KEY2 == 0)
#define KEY3_PRESSED()  (KEY3 == 0)
#define KEY4_PRESSED()  (KEY4 == 0)

// 获取所有按键状态（低4位有效，按下位为0）
#define KEY_STATE()     (KEY_PORT & 0x0F)

// ==================== 外设初始化函数声明 ====================
void Board_Init(void);      // 开发板初始化
void LED_Init(void);        // LED初始化
void KEY_Init(void);        // 按键初始化

#endif  // __BOARD_CONFIG_H__