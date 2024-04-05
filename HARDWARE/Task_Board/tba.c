#include "tba.h"
#include "analog_switch.h"
#include "seven_segment_display.h"

// 任务板光敏电阻配置
void Tba_Photoresistance_Config(void)
{
#if !defined(_USE_NEW_BOARD_)

	GPIO_InitTypeDef GPIO_TypeDefStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOG, ENABLE);

	GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_8;
	GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_IN; //输入
	GPIO_TypeDefStructure.GPIO_PuPd = GPIO_PuPd_UP; //上拉
	GPIO_Init(GPIOA, &GPIO_TypeDefStructure);

#endif // _USE_NEW_BOARD_
}

uint8_t Get_tba_phsis_value(void)
{
	return GPIO_ReadInputDataBit(GPIOG, GPIO_Pin_8);
}

// 任务板蜂鸣器配置
void Tba_Beep_Config(void)
{

#if !defined(_USE_NEW_BOARD_)

	GPIO_InitTypeDef GPIO_TypeDefStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOC, ENABLE);

	GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_13;
	GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_OUT;	 //输入
	GPIO_TypeDefStructure.GPIO_PuPd = GPIO_PuPd_UP;		 //上拉
	GPIO_TypeDefStructure.GPIO_Speed = GPIO_Speed_50MHz; // 有毒！！
	GPIO_Init(GPIOC, &GPIO_TypeDefStructure);
	GPIO_SetBits(GPIOC, GPIO_Pin_13);

#endif // _USE_NEW_BOARD_
}

/**
函数功能：设置任务板蜂鸣器状态
参    数：swch SET 开启 RESET 关闭
返 回 值：无
*/
void Set_tba_Beep(uint8_t swch)
{
#if !defined(_USE_NEW_BOARD_)

	if (swch == SET)
	{
		GPIO_ResetBits(GPIOC, GPIO_Pin_13);
	}
	else if (swch == RESET)
	{
		GPIO_SetBits(GPIOC, GPIO_Pin_13);
	}

#else // _USE_NEW_BOARD_

	AnalogSwitch_CelectChannel(Channel_BEEP);
	AnalogSwitch_Output(!swch);

#endif // _USE_NEW_BOARD_
}

// 任务板转向灯配置
void Tba_WheelLED_Config(void)
{
#if !defined(_USE_NEW_BOARD_)

	GPIO_InitTypeDef GPIO_TypeDefStructure;
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOH, ENABLE);

	// LED_L -- PH10  LED_R --PH11
	GPIO_TypeDefStructure.GPIO_Pin = GPIO_Pin_10 | GPIO_Pin_11;
	GPIO_TypeDefStructure.GPIO_Mode = GPIO_Mode_OUT; //输入
	GPIO_TypeDefStructure.GPIO_PuPd = GPIO_PuPd_UP;  //上拉
	GPIO_Init(GPIOH, &GPIO_TypeDefStructure);
	GPIO_SetBits(GPIOH, GPIO_Pin_10 | GPIO_Pin_11);

#endif // _USE_NEW_BOARD_
}

/**
函数功能: 任务板转向灯控制
参    数：LorR  L_LED--左侧   R_LED--右侧
		  swch  SET -- 开     RESET--关
返 回 值: 无
*/
void Set_tba_WheelLED(uint8_t LorR, uint8_t swch)
{
#if !defined(_USE_NEW_BOARD_)

	switch (LorR)
	{
	case L_LED:
		if (swch)
			GPIO_ResetBits(GPIOH, GPIO_Pin_10);
		else
			GPIO_SetBits(GPIOH, GPIO_Pin_10);
		break;
	case R_LED:
		if (swch)
			GPIO_ResetBits(GPIOH, GPIO_Pin_11);
		else
			GPIO_SetBits(GPIOH, GPIO_Pin_11);
		break;
	}

#else // _USE_NEW_BOARD_

	// 新板不能双闪
	switch (LorR)
	{
	case L_LED:
		AnalogSwitch_CelectChannel(Channel_LED_L);
		AnalogSwitch_Output(swch);
		break;
	case R_LED:
		AnalogSwitch_CelectChannel(Channel_LED_R);
		AnalogSwitch_Output(!swch);
		break;
	}

#endif // _USE_NEW_BOARD_
}

// 任务板初始化
void Tba_Init(void)
{
#if !defined(_USE_NEW_BOARD_)

	Tba_Photoresistance_Config();
	Tba_Beep_Config();
	Tba_WheelLED_Config();

#else // _USE_NEW_BOARD_

	AnalogSwitch_PortInit();
	SevenSegmentDisplay_PortInit();

#endif // _USE_NEW_BOARD_
}
