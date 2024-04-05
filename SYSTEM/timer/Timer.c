#include "stm32f4xx.h"
#include "Timer.h"
#include "stdbool.h"
#include "debug.h"

// 毫秒时间戳
volatile uint32_t global_times = 0;

// 全局时间（TIM10）
void Timer_Init(uint16_t arr, uint16_t psc)
{
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM10, ENABLE);

	TIM_InitStructure.TIM_Period = arr;
	TIM_InitStructure.TIM_Prescaler = psc;
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
	TIM_InitStructure.TIM_RepetitionCounter = 0;
	TIM_TimeBaseInit(TIM10, &TIM_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM1_UP_TIM10_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM10, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM10, ENABLE);
}

void TIM1_UP_TIM10_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM10, TIM_IT_Update) == SET)
	{
		// DEBUG_PIN_3_TOGGLE();
		global_times++;
	}
	TIM_ClearITPendingBit(TIM10, TIM_IT_Update);
}
