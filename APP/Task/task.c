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
//test
// * 模式选择
uint8_t Need_Back_To_Cross = true; // 白卡路段退回黑线
uint8_t Turn_Check = true;		   // 转弯矫正
uint8_t Execute_Task = true;	   // 执行任务
uint8_t Find_SpecialRoad = false;  // 识别特殊地形
uint8_t Find_RFID = false;		   // 识别白卡

// * 题目数据 & data_interation
uint8_t AGV_platenumber[15]; // 从车车牌
uint8_t AGV_QRCodeData1[32]; // 从车二维码1
uint8_t AGV_QRCodeData2[32]; // 从车二维码2
uint16_t AGV_dis;			 // 从车方向
uint8_t car_flag = 0;		 // 避让标识

// * 题目数据 & independent_task
uint8_t *AGV_routeInfo;	 // 从车未知路径
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
uint16_t L; // 距离
uint8_t plate[6];
uint8_t sign;
uint8_t n;		// 路灯初始挡位
uint8_t fht[6]; // 烽火台
uint8_t T;		// 路灯调整挡位
extern Block_Info_t RFID1_Block[2];
extern Block_Info_t RFID2_Block[2];
