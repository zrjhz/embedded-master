#include <stdint.h>
#include "stm32f4xx.h"
#include "canP_HostCom.h"
#include "delay.h"
#include "roadway_check.h"

#define __CAN_USER_C__
#include "can_user.h"
#include "tba.h"
#include "cba.h"
#include "infrared.h"
#include <string.h>
#include "Timer.h"

// Gao added
#include "protocol.h"
#include "data_interaction.h"

uint8_t Wifi_Rx_Buf[WIFI_MAX_NUM];
uint8_t Zigb_Rx_Buf[ZIGB_RX_MAX];
uint8_t Wifi_Rx_num;
uint8_t Wifi_Rx_flag; // 接收完成标志位
uint8_t Zigbee_Rx_num;
uint8_t Zigbee_Rx_flag; // 接收完成标志位

uint32_t canu_wifi_rxtime = 0;
uint32_t canu_zibe_rxtime = 0;

void Can_WifiRx_Save(uint8_t res)
{
    if (Wifi_Rx_flag == 0)
    {
        canu_wifi_rxtime = gt_get() + 10;
        Wifi_Rx_num = 0;
        Wifi_Rx_Buf[Wifi_Rx_num] = res;
        Wifi_Rx_flag = 1;
    }
    else if (Wifi_Rx_num < WIFI_MAX_NUM)
    {
        Wifi_Rx_Buf[++Wifi_Rx_num] = res;
    }
}

uint8_t Rx_Flag;

void Normal_data(void) // 正常接收8字节控制指令
{
    u8 sum = 0;

    if (Wifi_Rx_Buf[7] == 0xbb) // 判断包尾
    {
        // 主指令与三位副指令左求和校验
        // warning：在求和溢出时应该对和做256取余。
        sum = (Wifi_Rx_Buf[2] + Wifi_Rx_Buf[3] + Wifi_Rx_Buf[4] + Wifi_Rx_Buf[5]) % 256;
        if (sum == Wifi_Rx_Buf[6])
        {
            Rx_Flag = 1;
        }
        else
            Rx_Flag = 0;
    }
}

void Abnormal_data(void) // 数据异常处理
{
    u8 i, j;
    u8 sum = 0;

    if (Wifi_Rx_num < 8) // 异常数据字节数小于8字节不做处理
    {
        Rx_Flag = 0;
    }
    else
    {
        for (i = 0; i <= (Wifi_Rx_num - 7); i++)
        {
            if (Wifi_Rx_Buf[i] == 0x55) // 寻找包头
            {
                if (Wifi_Rx_Buf[i + 7] == 0xbb) // 判断包尾
                {
                    sum = (Wifi_Rx_Buf[i + 2] + Wifi_Rx_Buf[i + 3] + Wifi_Rx_Buf[i + 4] + Wifi_Rx_Buf[i + 5]) % 256;

                    if (sum == Wifi_Rx_Buf[i + 6]) // 判断求和
                    {
                        for (j = 0; j < 8; j++)
                        {
                            Wifi_Rx_Buf[j] = Wifi_Rx_Buf[j + i]; // 数据搬移
                        }
                        Rx_Flag = 1;
                    }
                    else
                        Rx_Flag = 0;
                }
            }
        }
    }
}

uint8_t Infrared_Tab[6]; // 红外数据存放数组

void Can_WifiRx_Check(void)
{
    if (Wifi_Rx_flag)
    {
        if (gt_get_sub(canu_wifi_rxtime) == 0)
        {
            if (Wifi_Rx_Buf[0] == 0xFD)
                Send_ZigbeeData_To_Fifo(Wifi_Rx_Buf, (Wifi_Rx_num + 1));
            else if (Wifi_Rx_Buf[0] == 0x55) // 普通WIFI/ZigBee指令
                Normal_data();
            else if (Wifi_Rx_Buf[0] == 0x56) // 自定义数据处理
            {
                HostData_Handler(Wifi_Rx_Buf);
                Rx_Flag = 0; // 跳过后续的数据处理
            }
            else
                Abnormal_data();
            Wifi_Rx_flag = 0;
        }
    }
    if (Rx_Flag == 1)
    {
        if (Wifi_Rx_Buf[1] == 0xAA)
            Process_CommandFromHost(Wifi_Rx_Buf[2]); // 判断指令,执行、置位
        else
            Send_ZigbeeData_To_Fifo(Wifi_Rx_Buf, 8);
        Rx_Flag = 0;
    }
}

/**
函数功能：保存ZigBee数据
参    数: 无
返 回 值：无
*/
void Can_ZigBeeRx_Save(uint8_t res)
{
    /*	if(Zigbee_Rx_flag == 0)
    {
        Zigb_Rx_Buf[Zigbee_Rx_num]=res;
        Zigbee_Rx_num++;
        if(Zigbee_Rx_num > ZIGB_RX_MAX )
        {
            Zigbee_Rx_num = 0;
            Zigbee_Rx_flag = 1;
        }
    } */

    if (Zigbee_Rx_flag == 0)
    {
        canu_zibe_rxtime = gt_get() + 10;
        Zigbee_Rx_num = 0;
        Zigb_Rx_Buf[Zigbee_Rx_num] = res;
        Zigbee_Rx_flag = 1;
    }
    else if (Zigbee_Rx_num < ZIGB_RX_MAX)
    {
        Zigb_Rx_Buf[++Zigbee_Rx_num] = res;
    }
}

/**
函数功能：ZigBee数据监测
参    数：无
返 回 值：无
*/
void Can_ZigBeeRx_Check(void)
{
    if (Zigbee_Rx_flag)
    {
        if (gt_get_sub(canu_zibe_rxtime) == 0)
        {
            // 对收到的ZigBee指令或数据进行处理
            ZigBee_CmdHandler(Zigb_Rx_Buf);
            Zigbee_Rx_flag = 0;
        }
    }
}

/** 暂未使用
函数功能：设置循迹上传更新时间
参    数：无
返 回 值：无
*/
void Canuser_upTrackTime(void)
{
    static uint8_t run_mode = 0;

    if (gt_get() < 1000)
        run_mode = 0;
    else if (run_mode == 0)
    {
        run_mode = 1;
        Host_Set_UpTrack(50);
    }
}

/**
函数功能：CAN查询、发送接收检测
参    数：无
返 回 值：无
*/
void Canuser_main(void)
{
    CanP_Host_Main();
    // CanP_CanTx_Check();				//CAN总线发送数据监测
    CanP_CanTx_Check_fIrq();
}
