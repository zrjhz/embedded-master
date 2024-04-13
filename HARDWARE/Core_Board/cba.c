#include "stm32f4xx.h"
#include "infrared.h"
#include "delay.h"
#include "cba.h"
#include "tba.h"
#include "pid.h"
#include "calculator.h"
#include "my_lib.h"
#include "infrared.h"
#include "Rc522.h"
#include "a_star.h"
#include "movement.h"
#include "task.h"
#include "can_user.h"
#include "canp_hostcom.h"
#include "protocol.h"
#include "roadway_check.h"
#include "ultrasonic.h"
#include "agv.h"
#include "voice.h"
#include "data_interaction.h"
#include "debug.h"
#include "bh1750.h"
#include "seven_segment_display.h"
#include "independent_task.h"

void Cba_Init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOI | RCC_AHB1Periph_GPIOH, ENABLE);

	// GPIOI4\5\6\7----KEY
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN; // ͨ�����?
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP; // ����
	GPIO_Init(GPIOI, &GPIO_InitStructure);

	// GPIOH12\13\14\15-----LED
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // ͨ�����?
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // �������?
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   // ����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOH, &GPIO_InitStructure);

	// GPIOH5 ---- MP_SPK
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_OUT;  // ͨ�����?
	GPIO_InitStructure.GPIO_OType = GPIO_OType_PP; // �������?
	GPIO_InitStructure.GPIO_PuPd = GPIO_PuPd_UP;   // ����
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOH, &GPIO_InitStructure);
}
extern Block_Info_t RFID1_Block[2];

void KEY_Check(void)
{
	if (S1 == 0)
	{
		delay_ms(10);
		if (S1 == 0)
		{
			while (!S1)
				;
			Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStatus); //  阅�?�顺�?2
		}
	}
	if (S2 == 0)
	{
		delay_ms(10);
		if (S2 == 0)
		{
			while (!S2)
				;
			Task_F2();
		}
	}
	if (S3 == 0)
	{
		delay_ms(10);
		if (S3 == 0)
		{
			while (!S3)
				;
		}
	}
	if (S4 == 0)
	{
		delay_ms(10);
		if (S4 == 0)
		{
			while (!S4)
				;
		}
	}
}
