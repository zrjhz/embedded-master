/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-03-02 08:12:27
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-03-18 11:16:08
 * @FilePath: \模板\APP\Movement\movement.h
 * @Description:
 *
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved.
 */
#if !defined(_MOVEMENT_H_)
#define _MOVEMENT_H_

#include "sys.h"
#include "a_star.h"
#include "Timer.h"
#include "route.h"

// Stop_Flag的枚举
typedef enum
{
	TRACKING = 0x00,		// 循迹状态
	CROSSROAD = 0x01,		// 十字路口
	TURNCOMPLETE = 0x02,	// 转弯完成
	FORBACKCOMPLETE = 0x03, // 前进后退完成
	CARD = 0x04,			// 找到白卡
	TURNCHECK = 0x05,		// 转弯检测
} StopFlag_t;				// 车子是因为什么而停下来的

// 定义转向模式
typedef enum TurnMethod_Struct
{
	TurnMethod_Track = 0,
	TurnMethod_Encoder = 1
} TurnMethod_t;

// 等待某个标志位。注意：此操作没有超时处理机制
#define WaitForFlag(flag, waitStatus) \
	do                                \
	{                                 \
		while (flag != waitStatus)    \
		{                             \
		};                            \
	} while (0)

// 等待动作完成,用于转向循迹
#define ExcuteAndWait(action, flag, waitStatus) \
	do                                          \
	{                                           \
		action;                                 \
		WaitForFlag(flag, waitStatus);          \
		Stop();                                 \
	} while (0)

// 快速动作宏定义

extern uint8_t Turn_Check;
extern uint8_t Need_Back_To_Cross;

// 通过码盘转向,不自动记录
#define TURN(digree) ExcuteAndWait(Turn_ByEncoder(digree), Stop_Flag, TURNCOMPLETE)
// 前后移动
#define MOVE(distance) ExcuteAndWait(Move_ByEncoder(Mission_Speed, distance), Stop_Flag, FORBACKCOMPLETE)
// 前后挪动
#define MOVE_BY_STEP(distance) ExcuteAndWait(Move_ByEncoder(Step_Speed, distance), Stop_Flag, FORBACKCOMPLETE)
// 根据循迹线转到某个方向,自动记录方向变化
#define TURN_TO(target) Turn_ToDirection(&CurrentStatus.dir, target, TurnOnce_EncoderMethod)

// 基本运动控制
void Stop(void);
void Stop_Set_Flag(StopFlag_t flag);
void Move_ByEncoder(int speed, float distance);
void Start_Tracking(void);
void Start_Tracking_Back(void);
void Back_To_Cross(void);
void Go_To_Cross(void);
float Get_Gap(RouteNode_t *current, RouteNode_t next);
void Stop_WithoutPIDClear(void);
void Track_ByEncoder(int speed, float distance);
void Move_By_Step(float distance);
void Wait_For_Action(StopFlag_t flag);
void Start_Turn_Check(void);
void Track_ByEncoder_With_SpecialRoad(int speed, float distance);
void Resume_RunningStatus(uint16_t encoderChangeValue);
void Turn_And_Check(Direction_t dir);
void Turn_ByEncoder(int16_t digree);
void TurnOnce_TrackMethod(Direction_t dir);
void TurnOnce_EncoderMethod(Direction_t dir);
void Wait_For_Turn_Check(void);
// 循迹线转弯
void Turn_ByTrack(Direction_t dir);

// 自动执行
void Go_ToNextNode(RouteNode_t *current, RouteNode_t next);
void Auto_DriveBetweenNodes(RouteNode_t *current, RouteNode_t next);
void Auto_Run(RouteSetting_t *routeTask, uint8_t taskNumber, RouteNode_t *current);

#endif // _MOVEMENT_H_
