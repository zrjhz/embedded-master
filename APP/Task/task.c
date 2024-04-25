#include "task.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "bh1750.h"
#include "voice.h"
#include "can_user.h"
#include "roadway_check.h"
#include "tba.h"
#include "uart_a72.h"
#include "Rc522.h"
#include "malloc.h"
#include "a_star.h"
#include "debug.h"
#include "movement.h"
#include "protocol.h"
#include "route.h"
#include "my_lib.h"
#include "Timer.h"
#include "agv.h"
#include "data_interaction.h"
#include "string.h"
#include "ctype.h"
#include "seven_segment_display.h"
#include <math.h>
#include "calculator.h"
// test
//  * 模式选择
uint8_t Need_Back_To_Cross = true; // 白卡路段退回黑线
uint8_t Turn_Check = true;         // 转弯矫正
uint8_t Execute_Task = true;       // 执行任务
uint8_t Find_SpecialRoad = false;  // 识别特殊地形
uint8_t Find_RFID = false;         // 识别白卡

// * 题目数据 & data_interation
uint8_t AGV_platenumber[15]; // 从车车牌
uint8_t AGV_QRCodeData1[32]; // 从车二维码1
uint8_t AGV_QRCodeData2[32]; // 从车二维码2
uint16_t AGV_dis;            // 从车方向
uint8_t car_flag = 0;        // 避让标识

// * 题目数据 & independent_task
uint8_t *AGV_routeInfo;  // 从车未知路径
uint8_t *AGV_StartPoint; // 从车起点
//  * 					   小地图
//  *     0      1     2     3     4     5      6
//  *     |										|
//  *  0  |------$-----------$-----------$------|  6
//  *     |      |           |           |      |
//  *     |      |           |           |      |             横x纵y
//  *  1  $------$-----------$-----------$------$  5
//  *     |      |           |           |      |
//  *  2  |      |           |           |      |  4
//  *     |      |           |           |      |
//  *  3  $------$-----------$-----------$------$  3
//  *     |      |           |           |      |
//  *  4  |      |           |           |      |  2
//  *     |      |           |           |      |
//  *  5  $------$-----------$-----------$------$  1
//  *     |      |           |           |      |
//  *     |      |           |           |      |
//  *  6  |------$-----------$-----------$------|  0
//  *  y  |										|
// 	*	x A      B     C     D     E     F      G
//  *
// * 做题参数
uint16_t L;             // 距离
uint8_t plate[6];       // 车牌
uint8_t sign;           // 交通标识
uint8_t n;              // 路灯初始挡位
uint8_t fht[6];         // 烽火台
uint8_t T;              // 路灯调整挡位
uint8_t YS[16];         // 红色二维码
uint8_t Green_Code[16]; // 绿色二维码
uint8_t E;              // 第一张张扇区
uint8_t S;              // 第二张张扇区
uint8_t x;              // 首个出现的字母的位置
uint8_t y;              // 首个出现的字母的位置
                        // 白卡数据
uint16_t FN;            // 卡1ABCD频次
uint8_t s1;             // 卡1首个数字
uint8_t s2;             // 卡1末个数字
uint8_t *M01;            // 卡2有效字符串

extern Block_Info_t RFID1_Block[2];
extern Block_Info_t RFID2_Block[2];

void Task_G2()
{
    Start_Task();
}

void Task_F2()
{
    StaticMarker_Task('A');
    uint8_t *red_code = Get_QRCode(DataRequest_QRCode1, 'A');   // D3AC6B68A5
    uint8_t *green_code = Get_QRCode(DataRequest_QRCode2, 'A'); // 010203

    memcpy(YS, red_code, 16);
    memcpy(Green_Code, green_code, 16);

    Dump_Array("red:", YS, 16, 0);
    // Dump_Array("green:", Green_Code, 16, 1);

    for (size_t i = 0; i < 2; i++)
    {
        for (size_t j = 0; j < 6; j++)
        {
            RFID1_Block[i].key[j] = Green_Code[j];
            RFID2_Block[i].key[j] = Green_Code[j];
        }
    }
    // Dump_Array("key:", RFID1_Block[0].key, 6, 1);

    x = getFirstDigit(YS, 0, 0);
    E = hexValue(YS[x]);
    x++;
    y = getFirstDigit(Green_Code, 1, 1);
    S = hexValue(Green_Code[y]);

    print_info("E:%X x:%X S:%X y:%X\r\n", E, x, S, y);
    // L = DistanceMeasure_Task(); // 静态标志物测距
    // delay(1000);

    // TURN_TO(DIR_DOWN); // 转弯矫正
    // delay(1000);
    // Start_Turn_Check();
    // Start_Turn_Check();
    // delay(3000);

    // TrafficLight_Task('A'); // 交通灯识别
    // delay(1000);
}

void Task_F4() {}

void Task_F5()
{
    RFID_Start(); // 白卡开始
}

void Task_F6()
{
    Voice_Recognition(); // 语音
    delay(1000);

    SpecialRoad_Start(); // 特殊地形开始
}

void Task_D6()
{
    SpecialRoad_End(); // 特殊地形结束
}

void Task_D4()
{
    RFID_End(); // 白卡任务结束
    delay(1000);

    TURN_TO(DIR_LEFT); // 路灯 TODO
    delay(1000);
    n = StreetLight_Now();
    delay(1000);
    T = (n ^ 2 + 3 * L) % 4 + 1;
    StreetLight_AdjustTo(T);
    delay(3000);
}

void Task_D2()
{
    TFT_Task(Zigbee_TFT_A, TFT_Task_License, 5); // 识别车牌
    delay(3000);
    memcpy(plate, Get_PlateNumber(Zigbee_TFT_A), 6);
    print_info("plate:%s\r\n", plate);
    delay(1000);

    TURN_TO(DIR_LEFT); // ETC
    delay(1000);
    ETC_Task();
}

void Task_B2()
{
    AGV_Send_None(AGV_CMD_Start);
    delay(3000);

    Wait_GarageToFristLayer('A');
    delay(1000);
    Reverse_Parcking(&CurrentStatus, "B1", 'A');
    delay(1000);
    StereoGarage_ToLayer('A', 3);
    WirelessCharging_ON();
    End_Task(); // 任务结束
}
