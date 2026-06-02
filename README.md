# Electronic_lock
基于 **AT89C51单片机** 的嵌入式电子密码锁系统，支持Keil编译开发与Proteus硬件仿真，采用模块化分层架构设计。

## 项目简介
本项目是51单片机经典课程设计/嵌入式实践项目，实现电子密码锁的核心控制功能。
工程采用标准化分层设计，代码结构清晰，包含完整的Keil工程文件、Proteus仿真工程，可直接编译、仿真、烧录运行。

- 主控芯片：AT89C51
- 开发架构：模块化分层设计
- 运行方式：Proteus仿真 / 实物烧录运行
- 编译输出：`.hex` 单片机可执行文件

## 硬件平台
- 核心主控：AT89C51 单片机
- 外设组件：矩阵键盘、显示模块、电子锁驱动机构

## 开发环境
- 编译环境：Keil uVision4/5 (C51 编译器)
- 仿真环境：Proteus 8.0 及以上版本
- 烧录工具：STC-ISP 等51单片机烧录软件

## 工程目录结构
```
Electronic_lock
├─boards               # 板级硬件配置
│  └─AT89C51
│          board_config.c
│          board_config.h
├─Doc                  # 项目文档
│      read.txt
├─drivers              # 外设驱动层
├─module               # 功能模块层
├─project              # Keil 工程目录
│  │     Electronic_lock.uvproj
│  ├─Listings          编译中间文件
│  └─Objects           编译输出文件
│          Electronic_lock.hex
├─proteusProject       # Proteus 仿真工程
│      Electronic_lock.pdsprj
├─user                 # 应用层代码
│      main.c
└─utils                # 系统工具配置
        system_config.h
```

## 快速开始
### 1. 工程编译
1. 打开 `project/Electronic_lock.uvproj` 工程文件
2. 使用Keil完成编译，无报错后自动生成HEX文件
3. 输出路径：`project/Objects/Electronic_lock.hex`

### 2. Proteus 仿真
1. 打开 `proteusProject/Electronic_lock.pdsprj`
2. 双击AT89C51芯片，加载生成的HEX文件
3. 启动仿真，测试电子锁功能

### 3. 实物运行
将HEX文件通过烧录工具下载至AT89C51单片机，外接硬件外设即可运行。

## 核心文件说明
| 文件路径 | 功能 |
| ---- | ---- |
| user/main.c | 主函数入口，系统总逻辑控制 |
| boards/AT89C51/board_config.c | 硬件初始化、IO口配置 |
| utils/system_config.h | 系统全局配置、宏定义 |
| project/Electronic_lock.uvproj | Keil工程配置文件 |
| proteusProject/Electronic_lock.pdsprj | Proteus仿真电路文件 |

## 系统功能
- 密码输入与验证
- 电子锁开关控制
- 模块化扩展：支持修改密码、显示、报警等功能

## 注意事项
1. 编译前需安装Keil C51编译插件
2. Proteus仿真必须正确加载HEX文件
3. 硬件参数可在配置头文件中自定义修改
```
