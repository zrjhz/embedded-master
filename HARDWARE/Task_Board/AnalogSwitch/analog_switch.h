#if !defined(_ANALOG_SWITCH_H_)
#define _ANALOG_SWITCH_H_

#include "sys.h"

#define Switch_COM PBout(15)
#define Switch_A PHout(10)
#define Switch_B PHout(11)
#define Switch_C PAout(15)

// 通道定义
enum
{
    Channel_LED_R = 0, // LED_R
    Channel_LED_L = 1, // LED_L
    Channel_SMG_A = 2, // 数码管A
    Channel_SMG_B = 3, // 数码管B
    Channel_INC = 4,   // 超声波发射
    Channel_IR = 5,    // 红外发射
    Channel_BEEP = 6,  // 蜂鸣器
    Channel_NONE = 7,  // 未使用
};


void AnalogSwitch_PortInit(void);
// 选择通道
static inline void AnalogSwitch_CelectChannel(uint8_t channel)
{
    // 超出7（0b0111）的数值会被截取后三位
	Switch_A = !!(channel & 0x01);
	Switch_B = !!(channel & 0x02);
	Switch_C = !!(channel & 0x04);
}

// 在当前通道输出数据
static inline void AnalogSwitch_Output(bool status)
{
    Switch_COM = status;
}

#endif // _ANALOG_SWITCH_H_
