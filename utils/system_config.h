/*
 * system_config.h - 系统全局配置文件
 * 功能：定义系统时钟、数据类型、全局宏等
 * 路径：utils/system_config.h
 */

#ifndef __SYSTEM_CONFIG_H__
#define __SYSTEM_CONFIG_H__

// ==================== 系统时钟配置 ====================
#define CRYSTAL_FREQ     12000000L    // 晶振频率 12MHz
#define MCU_FREQ         12000000L    // 单片机工作频率

// ==================== 常用数据类型重定义 ====================
typedef unsigned char   uint8_t;      // 无符号8位整数 (0~255)
typedef unsigned int    uint16_t;     // 无符号16位整数 (0~65535)
typedef unsigned long   uint32_t;     // 无符号32位整数
typedef signed char     int8_t;       // 有符号8位整数 (-128~127)
typedef signed int      int16_t;      // 有符号16位整数
typedef signed long     int32_t;      // 有符号32位整数
typedef bit             bool_t;       // 布尔类型

// ==================== 布尔值定义 ====================
#ifndef TRUE
#define TRUE    1
#endif

#ifndef FALSE
#define FALSE   0
#endif

#ifndef ON
#define ON      1
#endif

#ifndef OFF
#define OFF     0
#endif

// ==================== 电平定义 ====================
#define HIGH    1
#define LOW     0

// ==================== 延时相关宏 ====================
// 粗略延时计算（12MHz晶振下，一个循环约1us）
#define DELAY_1US()     { _nop_(); }
#define DELAY_1MS()     { uint16_t i; for(i=0;i<120;i++); }

// ==================== 编译器兼容 ====================
#include <reg52.h>      // 8051寄存器定义
#include <intrins.h>    // 内建函数（_nop_等）

#endif  // __SYSTEM_CONFIG_H__