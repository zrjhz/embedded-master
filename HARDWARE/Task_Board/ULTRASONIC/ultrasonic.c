#include "stm32f4xx.h"
#include "ultrasonic.h"
#include "delay.h"
#include "tba.h"
#include "analog_switch.h"
// #include "cba.h"

// 超声波引脚定义

#if !defined(_USE_NEW_BOARD_)

#define INC(x) PAout(15) = x

#else

#define INC(x)                               \
	AnalogSwitch_CelectChannel(Channel_INC); \
	AnalogSwitch_Output(x)

#endif

uint32_t Ultrasonic_Num = 0; // 计数值
uint16_t tempDistance = 0;

void Ultrasonic_Port(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB, ENABLE);

	//GPIO_PinAFConfig(GPIOB,GPIO_PinSource4,GPIO_AF_SWJ);
	//GPIO_PinAFConfig(GPIOB,GPIO_PinSource3,GPIO_AF_SWJ);
	//GPIO_PinAFConfig(GPIOA,GPIO_PinSource15,GPIO_AF_SWJ);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource14, GPIO_AF_SWJ);
	GPIO_PinAFConfig(GPIOA, GPIO_PinSource13, GPIO_AF_SWJ);

#if !defined(_USE_NEW_BOARD_)

	//GPIOA15---INC--RX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  //通用输出
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; //推挽输出
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   //上拉
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);

#endif // _USE_NEW_BOARD_

	//GPIOB4---INT0--TX
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN;
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; //浮空
	GPIO_Init(GPIOB, &GPIO_InitStructure);
}

void Ultrasonic_TIM(uint16_t arr, uint16_t psc)
{
	//GPIO_InitTypeDef GPIO_InitStructure;
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM6, ENABLE);

	TIM_InitStructure.TIM_Period = arr;
	TIM_InitStructure.TIM_Prescaler = psc;
	TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;

	TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1; // 有毒！！！
	TIM_InitStructure.TIM_RepetitionCounter = 0;

	TIM_TimeBaseInit(TIM6, &TIM_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = TIM6_DAC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2; // warning:优先级可能太低
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);

	TIM_ITConfig(TIM6, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM6, DISABLE);
}

void Ultrasonic_EXTI(void)
{
	EXTI_InitTypeDef EXTI_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1); // 有毒！！！

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_SYSCFG, ENABLE);

	SYSCFG_EXTILineConfig(EXTI_PortSourceGPIOB, EXTI_PinSource4);

	EXTI_InitStructure.EXTI_Line = EXTI_Line4;
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
	EXTI_Init(&EXTI_InitStructure);

	NVIC_InitStructure.NVIC_IRQChannel = EXTI4_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
}

void Ultrasonic_Init(void)
{
	Ultrasonic_Port();
	Ultrasonic_TIM(9, 83);
	Ultrasonic_EXTI();
}

//超声波测距
void Ultrasonic_Ranging(void)
{
	INC(1);
	delay_us(3);
	INC(0);

	TIM_Cmd(TIM6, ENABLE);
	//EXTI_ClearITPendingBit(EXTI_Line4);
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);

	Ultrasonic_Num = 0; // 定时器清零

	delay_ms(30); //等待一段时间,等待发送超声波控制信号
	INC(1);
	delay_ms(5);
}

void TIM6_DAC_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM6, TIM_IT_Update) == SET)
	{
		Ultrasonic_Num++;
	}
	TIM_ClearITPendingBit(TIM6, TIM_IT_Update);
}

void EXTI4_IRQHandler(void)
{
	if (EXTI_GetITStatus(EXTI_Line4) == SET)
	{
		if (GPIO_ReadInputDataBit(GPIOB, GPIO_Pin_4) == RESET)
		{
			TIM_Cmd(TIM6, DISABLE);

			// 计算距离定时10us,S = v * t / 2;
			tempDistance = (uint16_t)((float)Ultrasonic_Num * 1.72f);
		}
		EXTI_ClearITPendingBit(EXTI_Line4);
	}
}

// 获得测距平均值
uint16_t Ultrasonic_GetAverage(uint8_t times)
{
	uint32_t tmp_dis = 0;
	Ultrasonic_Ranging(); //丢弃第一次数据
	delay_ms(50);
	for (uint8_t i = 0; i < times; i++)
	{
		Ultrasonic_Ranging();
		delay_ms(50);
		tmp_dis += tempDistance;
	}
	return (tmp_dis / times) - UltrasonicErrorValue;
}
