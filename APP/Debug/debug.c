#include "debug.h"
#include "stdarg.h"
#include <stdio.h>
#include <string.h>
#include "canp_hostcom.h"
#include "independent_task.h"
uint8_t debug = 1;
// 与printf的用法一样
// 输出格式化字符串到通讯显示屏幕上
void print_info(char *str, ...)
{
    if (!debug)
        return;
    uint16_t len;
    uint8_t buf[50];
    va_list ap;
    va_start(ap, str);
    vsprintf((char *)buf, str, ap);
    va_end(ap);
    len = strlen((char *)buf);
    Send_InfoData_To_Fifo(buf, len);
}

void Dump_Array(uint8_t *name, uint8_t *array, uint8_t length, uint8_t choose)
{
    print_info("%s ", name);
    for (uint8_t i = 0; i < length; i++)
    {
        if (choose)
        {
            print_info("0x%02X ", array[i]);
        }
        else
        {
            print_info("%c", array[i]);
        }
        delay_ms(10);
    }
    print_info("\r\n");
}

// 初始化调试用引脚
// 操作引脚电平高低即可通过示波器或者LED观察到运行状态、中断进入情况、
// 中断或者任务处理时间等,请善用此工具
void DebugPin_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStructure;

    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);
    RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOF, ENABLE);

    GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;
    GPIO_InitStructure.GPIO_OType = GPIO_OType_PP;
    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9;
    GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_NOPULL;
    GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
    GPIO_Init(GPIOF, &GPIO_InitStructure);

    GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
    GPIO_Init(GPIOH, &GPIO_InitStructure);

    DEBUG_PIN_1_RESET();
    DEBUG_PIN_2_RESET();
    DEBUG_PIN_3_RESET();
}

// ZigBee测试丢包率
void ZigBee_Test(uint16_t count, uint16_t interval)
{
    uint8_t ZigBee_AGVStart[] = {0x55, 0x02, 0xD0, 0x00, 0x00, 0x00, 0x00, 0xBB};
    for (uint16_t i = 0; i < count; i++)
    {
        Send_ZigBeeData(ZigBee_AGVStart);
        delay_ms(interval);
    }
}

void DebugTimer_Init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM5, ENABLE);

    TIM_InitStructure.TIM_Period = arr;
    TIM_InitStructure.TIM_Prescaler = psc;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM5, &TIM_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM5_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 3;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM5, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM5, DISABLE);
}

// 调试定时器中断
// 目前用作白卡和特殊地形的处理中断
void TIM5_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM5, TIM_IT_Update) == SET)
    {
        TIM_ClearITPendingBit(TIM5, TIM_IT_Update);
    }
}
