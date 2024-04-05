#include "can_timer.h"
#include "canp_hostcom.h"
#include "debug.h"
#include "tba.h"

#if defined(_USE_NEW_BOARD_)

#include "seven_segment_display.h"

#endif // _USE_NEW_BOARD_

// 初始化CAN数据检查(ZigBee和WiFi)定时器(TIM3)
void CanTimer_Init(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);

    TIM_InitStructure.TIM_Period = arr;
    TIM_InitStructure.TIM_Prescaler = psc;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM3, &TIM_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3, ENABLE);
}

void TIM3_IRQHandler(void)
{
    if (TIM_GetITStatus(TIM3, TIM_IT_Update) == SET)
    {
        Can_WifiRx_Check();
        Can_ZigBeeRx_Check();

#if defined(_USE_NEW_BOARD_)

        // 刷新数码管
        // warning:不推荐使用定时器一直刷新数码管
        // 控制线共用
        // SevenSegmentDisplay_Refresh();

#endif

        TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
    }
}
