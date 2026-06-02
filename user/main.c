#include <reg51.h>
#include <intrins.h>

#define uchar unsigned char
#define uint  unsigned int

// LCD引脚连接
sbit LCD_RS = P2^0;
sbit LCD_RW = P2^1;
sbit LCD_E  = P2^2;

// 输出控制引脚
sbit GREEN_LED = P2^3;   // 绿灯（低电平点亮）
sbit RELAY = P2^4;   // 继电器控制（高电平吸合开锁，低电平断开上锁）
sbit RED_LED = P2^5;   // 红灯（低电平点亮）

// 24C02 I2C引脚（备用）
sbit SCK = P2^6;
sbit SDA = P2^7;

// 键盘引脚定义（4x4矩阵键盘）
// 行线（输出）
sbit ROW1 = P1^7;   // 行1 - 键: 7,8,9,/
sbit ROW2 = P1^6;   // 行2 - 键: 4,5,6,C(X)
sbit ROW3 = P1^5;   // 行3 - 键: 1,2,3,B(-)
sbit ROW4 = P1^4;   // 行4 - 键: ON/C,0,=,A(+)

// 列线（输入）
sbit COL1 = P1^0;   // 列1
sbit COL2 = P1^1;   // 列2
sbit COL3 = P1^2;   // 列3
sbit COL4 = P1^3;   // 列4

// 按键值定义
#define KEY_CONFIRM  10   // C键(X) - 确认
#define KEY_CLOSE    11   // B键(-) - 关闭/复位
#define KEY_MODIFY   13   // A键(+) - 修改密码
#define KEY_CLEAR    14   // ON/C键 - 清除

// 三个用户的密码（初始：0001, 0002, 0003）
uchar code init_pwd[3][4] = {
    {0, 0, 0, 1},  // 用户1
    {0, 0, 0, 2},  // 用户2
    {0, 0, 0, 3}   // 用户3
};

uchar pwd_store[3][4];  // 存储三个用户的密码
uchar current_user;      // 当前验证的用户

uchar input_buf[4];
uchar input_cnt = 0;
uchar err_cnt = 0;
bit lock_flag = 0;
bit modify_flag = 0;
uchar modify_user;       // 要修改密码的用户
uchar old_pwd[4];
uchar new_pwd[4];
uchar modify_step = 0;

// 延时函数
void delay_us(uint us)
{
    while(us--)
    {
        _nop_();
    }
}

void delay_ms(uint ms)
{
    uint i, j;
    for(i = 0; i < ms; i++)
        for(j = 0; j < 120; j++);
}

// LCD写指令
void LCD_Cmd(uchar cmd)
{
    LCD_RS = 0;
    LCD_RW = 0;
    P0 = cmd;
    delay_us(10);
    LCD_E = 1;
    delay_us(10);
    LCD_E = 0;
    delay_ms(2);
}

// LCD写数据
void LCD_Data(uchar dat)
{
    LCD_RS = 1;
    LCD_RW = 0;
    P0 = dat;
    delay_us(10);
    LCD_E = 1;
    delay_us(10);
    LCD_E = 0;
    delay_ms(2);
}

// LCD初始化
void LCD_Init()
{
    delay_ms(15);
    LCD_Cmd(0x38);
    delay_ms(5);
    LCD_Cmd(0x0C);
    delay_ms(5);
    LCD_Cmd(0x06);
    delay_ms(5);
    LCD_Cmd(0x01);
    delay_ms(5);
}

// 显示字符串
void LCD_Show(uchar x, uchar y, uchar *str)
{
    uchar addr;
    if(y == 0) 
        addr = 0x80 + x;
    else     
        addr = 0xC0 + x;
    LCD_Cmd(addr);
    while(*str)
    {
        LCD_Data(*str++);
    }
}

// 清屏
void LCD_Clear()
{
    LCD_Cmd(0x01);
    delay_ms(2);
}

// 显示输入的密码（直接显示数字）
void LCD_ShowInput(uchar x, uchar y, uchar *buf, uchar cnt)
{
    uchar i;
    uchar addr;
    if(y == 0) 
        addr = 0x80 + x;
    else     
        addr = 0xC0 + x;
    LCD_Cmd(addr);
    for(i = 0; i < cnt; i++)
    {
        LCD_Data(buf[i] + '0');
    }
    // 补齐空位
    for(i = cnt; i < 4; i++)
    {
        LCD_Data(' ');
    }
}
// 键盘扫描函数（改进防抖）
uchar KeyScan()
{
    uchar key_value = 0xFF;
    
    // 扫描第1行 (P1.7) - 键位: 7,8,9,/
    ROW1 = 0; ROW2 = 1; ROW3 = 1; ROW4 = 1;
    delay_us(50);
    if(COL1 == 0) { key_value = 7; }
    else if(COL2 == 0) { key_value = 8; }
    else if(COL3 == 0) { key_value = 9; }
    else if(COL4 == 0) { key_value = 0xFF; }
    if(key_value != 0xFF) goto found;
    
    // 扫描第2行 (P1.6) - 键位: 4,5,6,C(X键)
    ROW1 = 1; ROW2 = 0; ROW3 = 1; ROW4 = 1;
    delay_us(50);
    if(COL1 == 0) { key_value = 4; }
    else if(COL2 == 0) { key_value = 5; }
    else if(COL3 == 0) { key_value = 6; }
    else if(COL4 == 0) { key_value = KEY_CONFIRM; }
    if(key_value != 0xFF) goto found;
    
    // 扫描第3行 (P1.5) - 键位: 1,2,3,B(-键)
    ROW1 = 1; ROW2 = 1; ROW3 = 0; ROW4 = 1;
    delay_us(50);
    if(COL1 == 0) { key_value = 1; }
    else if(COL2 == 0) { key_value = 2; }
    else if(COL3 == 0) { key_value = 3; }
    else if(COL4 == 0) { key_value = KEY_CLOSE; }
    if(key_value != 0xFF) goto found;
    
    // 扫描第4行 (P1.4) - 键位: ON/C,0,=,A(+键)
    ROW1 = 1; ROW2 = 1; ROW3 = 1; ROW4 = 0;
    delay_us(50);
    if(COL1 == 0) { key_value = KEY_CLEAR; }
    else if(COL2 == 0) { key_value = 0; }
    else if(COL3 == 0) { key_value = 0xFF; }
    else if(COL4 == 0) { key_value = KEY_MODIFY; }
    if(key_value != 0xFF) goto found;
    
    found:
    // 恢复所有行线
    ROW1 = 1; ROW2 = 1; ROW3 = 1; ROW4 = 1;
    
    if(key_value != 0xFF)
    {
        // 防抖延时
        delay_ms(30);
        
        // 等待按键完全释放（关键！防止重复触发）
        while(COL1 == 0 || COL2 == 0 || COL3 == 0 || COL4 == 0);
        
        // 额外延时确保完全释放
        delay_ms(50);
    }
    
    return key_value;
}

// 串口初始化 (9600bps, 11.0592MHz晶振)
void Uart_Init()
{
    TMOD |= 0x20;   // 定时器1模式2
    TH1 = 0xFD;     // 9600波特率
    TL1 = 0xFD;
    TR1 = 1;        // 启动定时器1
    SCON = 0x50;    // 模式1, 允许接收
    PCON = 0x00;    // SMOD=0
}

// 串口发送字节
void Uart_Send(uchar dat)
{
    SBUF = dat;
    while(!TI);
    TI = 0;
}

// 串口发送字符串
void Uart_SendStr(uchar *str)
{
    while(*str)
    {
        Uart_Send(*str++);
    }
}

// 串口发送回车换行
void Uart_SendCRLF()
{
    Uart_Send(0x0D);  // 回车
    Uart_Send(0x0A);  // 换行
}

// 发送数字到串口
void Uart_SendNum(uchar *num)
{
    uchar i;
    for(i = 0; i < 4; i++)
    {
        Uart_Send(num[i] + '0');
    }
}

// 密码转数值（0-255）
uint PwdToNum(uchar *p)
{
    return (uint)(p[0]*1000 + p[1]*100 + p[2]*10 + p[3]);
}

// 检查密码是否在有效范围（0-255）
bit CheckPwdRange(uchar *p)
{
    uint num = PwdToNum(p);
    return (num <= 255);
}

// 验证密码（检查三个用户）
bit CheckPwd(uchar *input, uchar *user_id)
{
    uchar i, j;
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 4; j++)
        {
            if(input[j] != pwd_store[i][j])
                break;
            if(j == 3)
            {
                *user_id = i;
                return 1;
            }
        }
    }
    return 0;
}

// 正确动作
// 继电器逻辑：RELAY=0吸合开锁，2秒后RELAY=1断开上锁
// 在 Action_Ok() 函数中修改
// 正确的 Action_Ok


void Action_Ok()
{
    GREEN_LED = 0;
    RED_LED   = 1;

    // 强制输出高电平，驱动NPN导通、继电器吸合
    RELAY = 0;
    delay_ms(20);   // 给三极管/继电器足够响应时间

    LCD_Clear();
    LCD_Show(0, 0, "Unlock Success!");
    LCD_Show(0, 1, "Door Opened    ");

    Uart_SendStr("User ");
    Uart_Send(current_user + '1');
    Uart_SendStr(" Password OK -> Unlock");
    Uart_SendCRLF();

    delay_ms(2000); // 保持开锁2秒

    // 恢复上锁
    GREEN_LED = 1;
    RELAY = 1;
    delay_ms(20);

    err_cnt = 0;
}


// 错误动作
// 继电器保持断开状态（上锁）
void Action_Err()
{
    RED_LED   = 0;   // 红灯亮
    GREEN_LED = 1;   // 绿灯灭
    RELAY     = 1;   // 继电器保持断开（上锁）
    err_cnt++;
    LCD_Clear();
    LCD_Show(0, 0, "Password Error!");
    LCD_Show(0, 1, "Count: ");
    LCD_Data(err_cnt + '0');
    LCD_Data('/');
    LCD_Data('3');
    
    Uart_SendStr("Password Error! Count: ");
    Uart_Send(err_cnt + '0');
    Uart_SendStr("/3");
    Uart_SendCRLF();
    
    delay_ms(1000);
    RED_LED = 1;     // 红灯灭
    
    if(err_cnt >= 3)
    {
        lock_flag = 1;
        LCD_Clear();
        LCD_Show(0, 0, "SYSTEM LOCKED!");
        LCD_Show(0, 1, "3 TIMES ERROR ");
        Uart_SendStr("SYSTEM LOCKED! Too many errors");
        Uart_SendCRLF();
    }
}

// 初始化密码
void InitPasswords()
{
    uchar i, j;
    for(i = 0; i < 3; i++)
    {
        for(j = 0; j < 4; j++)
        {
            pwd_store[i][j] = init_pwd[i][j];
        }
    }
}

// 主函数
void main()
{
    uchar key;
    uchar i;
    
    LCD_Init();
    Uart_Init();
    InitPasswords();
    
    // 初始化状态：继电器断开（上锁状态），灯灭
    GREEN_LED = 1;   // 绿灯灭
    RED_LED   = 1;   // 红灯灭
    RELAY     = 1;   // 继电器断开 = 上锁状态（默认）
    
    LCD_Show(0, 0, "Enter Password:");
    LCD_Show(0, 1, "                ");
    
    Uart_SendStr("Electronic Lock System Started");
    Uart_SendCRLF();
    Uart_SendStr("3 Users: 0001, 0002, 0003");
    Uart_SendCRLF();
    Uart_SendStr("Press X to confirm, - to reset, + to modify, ON/C to clear");
    Uart_SendCRLF();
    
	/*// 测试继电器和LED（开机时）
    GREEN_LED = 0;  // 绿灯亮
    RED_LED = 0;    // 红灯亮
    RELAY = 0;      // 继电器吸合
    delay_ms(2000); // 保持1秒
    
    GREEN_LED = 1;  // 绿灯灭
    RED_LED = 1;    // 红灯灭
    RELAY = 1;      // 继电器断开
    delay_ms(1000);
	*/
	
    while(1)
    {
        key = KeyScan();
        if(key == 0xFF) 
        {
            delay_ms(50);
            continue;
        }
        
        // 系统锁定 - 按清除键(ON/C)复位
        if(lock_flag)
        {
            if(key == KEY_CLEAR)
            {
                lock_flag = 0;
                err_cnt = 0;
                input_cnt = 0;
                LCD_Clear();
                LCD_Show(0, 0, "Enter Password:");
                LCD_Show(0, 1, "                ");
                Uart_SendStr("System Reset");
                Uart_SendCRLF();
            }
            continue;
        }
        
        // 修改键 '+' (KEY_MODIFY)
        if(key == KEY_MODIFY && !modify_flag)
        {
            modify_flag = 1;
            modify_step = 0;
            input_cnt = 0;
            LCD_Clear();
            LCD_Show(0, 0, "Enter User(1-3):");
            LCD_Show(0, 1, "                ");
            Uart_SendStr("Modify Mode - Enter User ID (1-3)");
            Uart_SendCRLF();
            continue;
        }
        
        // 清除键 'ON/C' (KEY_CLEAR)
        if(key == KEY_CLEAR)
        {
            input_cnt = 0;
            LCD_Clear();
            if(modify_flag)
            {
                if(modify_step == 0)
                {
                    LCD_Show(0, 0, "Enter User(1-3):");
                    LCD_Show(0, 1, "                ");
                }
                else if(modify_step == 1)
                {
                    LCD_Show(0, 0, "Enter Old PWD:");
                    LCD_Show(0, 1, "                ");
                }
                else
                {
                    LCD_Show(0, 0, "Enter New PWD:");
                    LCD_Show(0, 1, "                ");
                }
            }
            else
            {
                LCD_Show(0, 0, "Enter Password:");
                LCD_Show(0, 1, "                ");
            }
            continue;
        }
        
        // 关闭键 '-' (KEY_CLOSE) - 复位所有状态
        if(key == KEY_CLOSE)
        {
            modify_flag = 0;
            modify_step = 0;
            input_cnt = 0;
            err_cnt = 0;
            LCD_Clear();
            LCD_Show(0, 0, "Enter Password:");
            LCD_Show(0, 1, "                ");
            Uart_SendStr("System Reset by User");
            Uart_SendCRLF();
            continue;
        }
        
        // 数字键0-9
        if(key >= 0 && key <= 9 && input_cnt < 4)
        {
            if(!modify_flag)
            {
                input_buf[input_cnt] = key;
                // 直接显示输入的数字
                LCD_ShowInput(0, 1, input_buf, input_cnt + 1);
                input_cnt++;
            }
            else
            {
                if(modify_step == 0)
                {
                    // 输入用户号(1-3)
                    if(input_cnt == 0 && key >= 1 && key <= 3)
                    {
                        modify_user = key - 1;
                        LCD_Data(key + '0');
                        input_cnt++;
                        modify_step = 1;
                        input_cnt = 0;
                        LCD_Clear();
                        LCD_Show(0, 0, "Enter Old PWD:");
                        LCD_Show(0, 1, "                ");
                        Uart_SendStr("Modify User ");
                        Uart_Send(modify_user + '1');
                        Uart_SendStr(" - Enter Old Password");
                        Uart_SendCRLF();
                    }
                    else if(input_cnt == 0 && (key < 1 || key > 3))
                    {
                        LCD_Clear();
                        LCD_Show(0, 0, "Invalid User!");
                        LCD_Show(0, 1, "Enter 1-3     ");
                        delay_ms(1000);
                        LCD_Clear();
                        LCD_Show(0, 0, "Enter User(1-3):");
                        LCD_Show(0, 1, "                ");
                    }
                }
                else if(modify_step == 1)
                {
                    old_pwd[input_cnt] = key;
                    LCD_ShowInput(0, 1, old_pwd, input_cnt + 1);
                    input_cnt++;
                }
                else if(modify_step == 2)
                {
                    new_pwd[input_cnt] = key;
                    LCD_ShowInput(0, 1, new_pwd, input_cnt + 1);
                    input_cnt++;
                }
            }
        }
        
        // 确认键 'X' (KEY_CONFIRM)
        if(key == KEY_CONFIRM)
        {
            if(!modify_flag)  // 普通验证模式
            {
                if(input_cnt == 4)
                {
                    // 串口显示输入的密码
                    Uart_SendStr("Input Password: ");
                    Uart_SendNum(input_buf);
                    Uart_SendCRLF();
                    
                    if(CheckPwd(input_buf, &current_user))
                    {
                        Action_Ok();
                        LCD_Clear();
                        LCD_Show(0, 0, "Enter Password:");
                        LCD_Show(0, 1, "                ");
                    }
                    else
                    {
                        Action_Err();
                        if(!lock_flag)
                        {
                            LCD_Clear();
                            LCD_Show(0, 0, "Enter Password:");
                            LCD_Show(0, 1, "                ");
                        }
                    }
                    input_cnt = 0;
                }
                else if(input_cnt > 0)
                {
                    LCD_Clear();
                    LCD_Show(0, 0, "Need 4 digits!");
                    LCD_Show(0, 1, "                ");
                    delay_ms(1000);
                    LCD_Clear();
                    LCD_Show(0, 0, "Enter Password:");
                    LCD_Show(0, 1, "                ");
                    input_cnt = 0;
                }
            }
            else  // 修改密码模式
            {
                if(modify_step == 1 && input_cnt == 4)  // 验证旧密码
                {
                    Uart_SendStr("Old Password: ");
                    Uart_SendNum(old_pwd);
                    Uart_SendCRLF();
                    
                    if(CheckPwdRange(old_pwd))
                    {
                        uchar j;
                        bit pwd_correct = 0;
                        for(j = 0; j < 4; j++)
                        {
                            if(old_pwd[j] != pwd_store[modify_user][j])
                                break;
                            if(j == 3)
                                pwd_correct = 1;
                        }
                        if(pwd_correct)
                        {
                            modify_step = 2;
                            input_cnt = 0;
                            LCD_Clear();
                            LCD_Show(0, 0, "Enter New PWD:");
                            LCD_Show(0, 1, "                ");
                            Uart_SendStr("Enter New Password (0000-0255)");
                            Uart_SendCRLF();
                        }
                        else
                        {
                            LCD_Clear();
                            LCD_Show(0, 0, "Old PWD Error!");
                            LCD_Show(0, 1, "                ");
                            delay_ms(1000);
                            modify_flag = 0;
                            modify_step = 0;
                            LCD_Clear();
                            LCD_Show(0, 0, "Enter Password:");
                            LCD_Show(0, 1, "                ");
                            Uart_SendStr("Old Password Error!");
                            Uart_SendCRLF();
                        }
                    }
                    else
                    {
                        LCD_Clear();
                        LCD_Show(0, 0, "Invalid Range!");
                        LCD_Show(0, 1, "0000-0255 Only");
                        delay_ms(1000);
                        modify_flag = 0;
                        modify_step = 0;
                        LCD_Clear();
                        LCD_Show(0, 0, "Enter Password:");
                        LCD_Show(0, 1, "                ");
                        Uart_SendStr("Password out of range (0-255)!");
                        Uart_SendCRLF();
                    }
                    input_cnt = 0;
                }
                else if(modify_step == 2 && input_cnt == 4)  // 设置新密码
                {
                    Uart_SendStr("New Password: ");
                    Uart_SendNum(new_pwd);
                    Uart_SendCRLF();
                    
                    if(CheckPwdRange(new_pwd))
                    {
                        for(i = 0; i < 4; i++)
                        {
                            pwd_store[modify_user][i] = new_pwd[i];
                        }
                        LCD_Clear();
                        LCD_Show(0, 0, "Modify Success!");
                        LCD_Show(0, 1, "                ");
                        Uart_SendStr("User ");
                        Uart_Send(modify_user + '1');
                        Uart_SendStr(" Password Modified to ");
                        Uart_SendNum(new_pwd);
                        Uart_SendCRLF();
                        delay_ms(1500);
                    }
                    else
                    {
                        LCD_Clear();
                        LCD_Show(0, 0, "Invalid Range!");
                        LCD_Show(0, 1, "0000-0255 Only");
                        Uart_SendStr("New Password out of range (0-255)!");
                        Uart_SendCRLF();
                        delay_ms(1000);
                    }
                    modify_flag = 0;
                    modify_step = 0;
                    LCD_Clear();
                    LCD_Show(0, 0, "Enter Password:");
                    LCD_Show(0, 1, "                ");
                    input_cnt = 0;
                }
                else if(input_cnt != 4 && input_cnt > 0)
                {
                    LCD_Clear();
                    LCD_Show(0, 0, "Need 4 digits!");
                    LCD_Show(0, 1, "                ");
                    delay_ms(1000);
                    LCD_Clear();
                    if(modify_step == 1)
                    {
                        LCD_Show(0, 0, "Enter Old PWD:");
                        LCD_Show(0, 1, "                ");
                    }
                    else if(modify_step == 2)
                    {
                        LCD_Show(0, 0, "Enter New PWD:");
                        LCD_Show(0, 1, "                ");
                    }
                    input_cnt = 0;
                }
            }
        }
        
        delay_ms(100);
    }
}