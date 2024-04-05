#include "seven_segment_display.h"
#include "analog_switch.h"
#include "my_lib.h"
#include "delay.h"

#define SerialInput PCout(13) // SER
#define LatchClock PFout(11)  // RCK
#define ShiftClock PGout(8)   // SCK

// 控制数码管A
#define Control_A(x)                           \
    AnalogSwitch_CelectChannel(Channel_SMG_A); \
    AnalogSwitch_Output(x);

// 控制数码管B
// AB的控制电平相反
#define Control_B(x)                           \
    AnalogSwitch_CelectChannel(Channel_SMG_B); \
    AnalogSwitch_Output(x);

// 数码管显示数据 0-F + 熄灭
const uint8_t SevenSegmentDisplayCode[] = {
    0xC0, 0xF9, 0xA4, 0xB0, 0x99, 0x92, 0x82, 0xF8,
    0x80, 0x90, 0x88, 0x83, 0xC6, 0xA1, 0x86, 0x8E,
    0xFF};

// 显示缓冲
static uint8_t displayData = 0x00;

// 数码管端口初始化
void SevenSegmentDisplay_PortInit(void)
{
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOF | RCC_AHB1Periph_GPIOG, ENABLE);

    GPIO_InitTypeDef GPIO_InitStructure;
    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;

    // PC13
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13;
    GPIO_Init(GPIOC, &GPIO_InitStructure);

    // PF11
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    // PG8
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8;
    GPIO_Init(GPIOG, &GPIO_InitStructure);

    ShiftClock = 0;
    SerialInput = 0;
    LatchClock = 0;
}

// 74HC595写入数据并输出
// MSB First
void HC595_Write_Data(uint8_t data)
{
    LatchClock = 0;
    for (uint8_t i = 0; i < 8; i++)
    {
        SerialInput = !!(data & (1 << (7 - i)));
        ShiftClock = 1;
        ShiftClock = 0;
    }
    LatchClock = 1;
}

// 刷新数码管显示内容
// 刷新一次显示一半
void SevenSegmentDisplay_Refresh(void)
{
    static bool A_OR_B = true;
    uint8_t data;

    if (A_OR_B)
    {
        data = (displayData >> 4) & 0x0F;
        Control_B(1);
        Control_A(1);
    }
    else
    {
        data = displayData & 0x0F;
        Control_A(0);
        Control_B(0);
    }
    HC595_Write_Data(SevenSegmentDisplayCode[data]);
    A_OR_B = !A_OR_B;
}

// 数码管更新显示数据 DEC
void SevenSegmentDisplay_Update(uint8_t data)
{
    displayData = HEX2BCD(data % 100);
}

// 数码管更新显示数据 HEX
void SevenSegmentDisplay_UpdateHex(uint8_t data)
{
    displayData = data;
}

// 显示数字,循环显示数字
// 死循环
void Display_Number(uint8_t number)
{
    SevenSegmentDisplay_Update(number);

    while (1)
    {
        SevenSegmentDisplay_Refresh();
        delay_ms(10);
    }
}
