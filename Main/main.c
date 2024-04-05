#include "hardware.h"

int main(void)
{
	uint32_t PowerCheckStamp = 0;
	uint32_t RFIDCheckStamp = 0;
	Can_ZigBeeRx_Check(); // 初始化can和zigbee
	Hardware_Init();	  // 初始化硬件

	while (1)
	{
		KEY_Check(); // 按键检测 阅读顺序1
		if (autoRunEnable)
		{
			Auto_Run(Route_Task, ROUTE_TASK_NUMBER, &CurrentStatus);
			autoRunEnable = 0;
			// 执行� ��务后进入死循环
			infinity_loop();
		}
		if (IsTimeOut(PowerCheckStamp, 200))
		{
			PowerCheckStamp = millis();
			Power_Check();
		}
		if (IsTimeOut(RFIDCheckStamp, 300))
		{
			RFIDCheckStamp = millis();
			if (Rc522_GetLinkFlag() == 0)
			{
				Readcard_Device_Init();
				MP_SPK = !MP_SPK;
			}
			else
			{
				MP_SPK = 0;
				LED4 = !LED4;
				Rc522_LinkTest();
			}
		}
	}
}

// 参数错误的处理
void assert_failed(uint8_t *file, uint32_t line)
{
	print_info("ERR: %s, %d\r\n", file, line);
	infinity_loop();
}
