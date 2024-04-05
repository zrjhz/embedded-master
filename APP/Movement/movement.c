#include "movement.h"
#include "roadway_check.h"
#include "CanP_Hostcom.h"
#include "a_star.h"
#include "pid.h"
#include "route.h"
#include "malloc.h"
#include "debug.h"
#include "independent_task.h"
#include "data_interaction.h"
#include "my_lib.h"
#include "task.h"

uint32_t lastStopStamp = 0;
uint8_t turnLeftOrRight = DIR_NOTSET;
uint8_t route_index;			  // 当前路径下标
uint8_t card_skip = false;		  // 跳过第一个白卡路段(必须跳过)
uint8_t specialroad_skip = false; // 跳过第一个白卡路段(必须跳过)
extern uint8_t Turn_Check;
extern uint8_t Need_Back_To_Cross;
extern uint8_t Execute_Task;

struct RunningStatus_Struct
{
	uint8_t stopFlag;
	uint8_t trackMode;
	int8_t currentSpeed;
	Moving_ByEncoder_t movingByencoder;
	uint16_t remainEncoderValue;
} RunningStatus;

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 循迹 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:Auto_Run
 【参 数】:routeTask:任务路径	taskNumber:任务点个数	current:当前坐标点的指针
 【返 回】:
 【简 例】:
 【说 明】:全自动跑图
 ************************************************************************************************************/
void Auto_Run(RouteSetting_t *routeTask, uint8_t taskNumber, RouteNode_t *current)
{
	Generate_Routetask(routeTask, taskNumber); // 初始化坐标信息
	*current = routeTask[0].node;			   // 设定起始方向和坐标
	card_skip = 0;
	for (uint8_t i = 0; i < taskNumber; i++)
	{
		Auto_DriveBetweenNodes(current, routeTask[i].node);
		print_info("zbd:%s\r\n", ReCoordinate_Convert(CurrentStatus));
		if (routeTask[i].Task != NULL && Execute_Task)
		{
			routeTask[i].Task(); // 任务非空时执行任务
		}
	}
}
/************************************************************************************************************
 【函 数】:Auto_DriveBetweenNodes
 【参 数】:current:当前点的指针		next:下一个点的坐标
 【返 回】:
 【简 例】:
 【说 明】:处理跳点,两个坐标可以跳点
 ************************************************************************************************************/
void Auto_DriveBetweenNodes(RouteNode_t *current, RouteNode_t next)
{
	RouteNode_t *route = malloc(sizeof(RouteNode_t) * 12); // 两点间最多12途径点
	uint8_t routeCount = 0;

	A_Star_GetRouteBetweenNodes(*current, next, route, &routeCount);

	// 跳过第一个点,因为当前就在第一个点。
	for (route_index = 1; route_index < routeCount; route_index++)
	{
		NextStatus = route[route_index];
		Go_ToNextNode(current, route[route_index]);
	}
	free(route);
}
/************************************************************************************************************
 【函 数】:Auto_DriveBetweenNodes
 【参 数】:current:当前点的指针		next:下一个点的坐标
 【返 回】:
 【简 例】:
 【说 明】:真正前往下一点,不能跳点
 ************************************************************************************************************/
void Go_ToNextNode(RouteNode_t *current, RouteNode_t next)
{
	Force_Break = 0;
	Direction_t finalDir = DIR_NOTSET;
	NextStatus = next; // 下一个坐标信息更新
	finalDir = Get_TowardsByNode(*current, next);
	uint8_t move = false;

	TURN_TO(finalDir);

	Get_Track();
	if (Q7[3] && Q7[2] && Q7[4])
	{
		MOVE(-5);
		move = true;
	}

	if (next.x % 2 == 0 || next.y % 2 == 0)
	{
		if (Find_SpecialRoad)
		{
			if (specialroad_skip)
			{
				Start_Turn_Check();
				Start_Turn_Check();
			}
			else
				specialroad_skip = true;
		}
		else
		{
			Start_Turn_Check();
			Start_Turn_Check();
		}
	}

	if (Find_RFID && card_skip && (next.x % 2 == 0 || next.y % 2 == 0))
	{
		Back_To_Cross();
		Start_Tracking(); // 继续循迹
	}
	else
		card_skip = true;

	if (Find_SpecialRoad)
	{ // 固定码盘冲特殊地形
		float gap = Get_Gap(current, next);
		if (Need_Back_To_Cross)
			gap += ((current->x % 2 == 0 || current->y % 2 == 0) ? 0 : ToCrossroadCenter);

		if (move)
			gap += 5.0f;

		if (Find_RFID)
			Move_By_Step(gap);
		else
		{
			Track_ByEncoder_With_SpecialRoad(Mission_Speed, gap);
			Wait_For_Action(FORBACKCOMPLETE);
		}
	}
	else
	{ // 正常寻路
		if (next.x % 2 == 0)
		{ // X轴为偶数的坐标
			Track_ByEncoder(Track_Speed, ((next.x == 0) || (next.x == 6)) ? SidePark_Value : LongTrack_Value);
			Wait_For_Action(FORBACKCOMPLETE);
		}
		else if (next.y % 2 == 0)
		{ // Y轴为偶数的坐标
			Track_ByEncoder(Track_Speed, ShortTrack_Value);
			Wait_For_Action(FORBACKCOMPLETE);
		}
		else
		{ // 前方十字路口
			Start_Tracking();
			Wait_For_Action(CROSSROAD);
			MOVE(ToCrossroadCenter);
		}
	}
	Stop();
	Submit_SpeedChanges();
	// 更新当前位置信息和方向
	current->x = next.x;
	current->y = next.y;
	current->dir = finalDir;
}
/************************************************************************************************************
 【函 数】:Get_Gap
 【参 数】:current:当前点	next:下一个点
 【返 回】:两个点之间的码盘距离
 【简 例】:
 【说 明】:废弃方案, 但没准能用上
 ************************************************************************************************************/
float Get_Gap(RouteNode_t *current, RouteNode_t next)
{
	// 对应上方小地图
	float gap[11][11] = {
		{0.0, 0.0, 40.0, 0.0, 0.0, 40.0, 0.0, 0.0, 40.0, 0.0, 0.0},
		{0.0, 0.0, 40.0, 0.0, 0.0, 40.0, 0.0, 0.0, 40.0, 0.0, 0.0},
		{47.0, 47.0, 0.0, 36.75, 36.75, 0.0, 36.75, 36.75, 0.0, 47.0, 47.0},
		{0.0, 0.0, 29.25, 0.0, 0.0, 29.25, 0.0, 0.0, 29.25, 0.0, 0.0},
		{0.0, 0.0, 29.25, 0.0, 0.0, 29.25, 0.0, 0.0, 29.25, 0.0, 0.0},
		{47.0, 47.0, 0.0, 36.75, 36.75, 0.0, 36.75, 36.75, 0.0, 47.0, 47.0},
		{0.0, 0.0, 29.25, 0.0, 0.0, 29.25, 0.0, 0.0, 29.25, 0.0, 0.0},
		{0.0, 0.0, 29.25, 0.0, 0.0, 29.25, 0.0, 0.0, 29.25, 0.0, 0.0},
		{47.0, 47.0, 0.0, 36.75, 36.75, 0.0, 36.75, 36.75, 0.0, 47.0, 47.0},
		{0.0, 0.0, 40.0, 0.0, 0.0, 40.0, 0.0, 0.0, 40.0, 0.0, 0.0},
		{0.0, 0.0, 40.0, 0.0, 0.0, 40.0, 0.0, 0.0, 40.0, 0.0, 0.0},
	};
	int8_t kernel[10] = {-1, -1, 0, 0, 1, 1, 2, 2, 3, 3};
	uint8_t y = current->x + 1;
	uint8_t x = 7 - current->y;
	uint8_t dx, dy;
	int8_t x_diff = next.x - y + 1;
	int8_t y_diff = 7 - next.y - x;

	// 下 | 右
	if ((x_diff == 1 && y_diff == 0) || (x_diff == 0 && y_diff == 1))
	{
		dx = kernel[x];
		dy = kernel[y];
	}
	// 上 | 左
	else if ((x_diff == -1 && y_diff == 0) || (x_diff == 0 && y_diff == -1))
	{
		dx = kernel[x + 1];
		dy = kernel[y + 1];
	}

	uint8_t x2 = 7 - next.y + dx;
	uint8_t y2 = next.x + 1 + dy;
	return gap[x2][y2];
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 运动标识控制 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓(和后面两个不同的是,这个是设置标识位,下面才是真正的在运动)
/************************************************************************************************************
 【函 数】:Wait_For_Action
 【参 数】:flag:停止的标识位
 【返 回】:
 【简 例】:Wait_For_Action(CROSSROAD) 等到Stop_flag为CROSSROAD时,退出死循环
 【说 明】:等待某个状态完成, 同时做判断任务
 ************************************************************************************************************/
void Wait_For_Action(StopFlag_t flag)
{
	while (Stop_Flag != flag)
	{
		if (Force_Break)
			break;
		if (Stop_Flag == CARD)
			RFID_Task();
	}
}
/************************************************************************************************************
 【函 数】:Stop
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:让车停止,并清空pid值
 ************************************************************************************************************/
void Stop(void)
{
	Roadway_Flag_clean(); // 清除标志位状态
	Update_MotorSpeed(0, 0);
	PidData_Clear();
	lastStopStamp = millis();
}
/************************************************************************************************************
 【函 数】:Stop_Set_Flag
 【参 数】:flag:车停止的标识位
 【返 回】:
 【简 例】:Stop_Set_Flag(CROSSROAD) 让车因为CROSSROAD而停
 【说 明】:让车停止,清空pid值,同时设置停止标识位
 ************************************************************************************************************/
void Stop_Set_Flag(StopFlag_t flag)
{
	Stop();
	Stop_Flag = flag;
}
/************************************************************************************************************
 【函 数】:Stop_WithoutPIDClear
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:让车停止,但不清空pid值
 ************************************************************************************************************/
void Stop_WithoutPIDClear(void)
{
	Roadway_Flag_clean(); // 清除标志位状态
	Update_MotorSpeed(0, 0);
	lastStopStamp = millis();
}
/************************************************************************************************************
 【函 数】:Start_Tracking
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:让车启动,用红外循迹,别用码盘循迹,设置速度
 ************************************************************************************************************/
void Start_Tracking(void)
{
	Stop_Flag = TRACKING;			 // 告诉车要启动了
	Track_Mode = TrackMode_NORMAL;	 // 告诉车用红外循迹
	Moving_ByEncoder = ENCODER_NONE; // 告诉车别用码盘循迹
	Car_Speed = Track_Speed;		 // 设置车速
}
/************************************************************************************************************
 【函 数】:Start_Tracking
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:让车退回至黑线处(这样做时为了防止一种白卡情况就是,白卡贴在黑线旁边,所有要让车先退回黑线,从黑线处开始扫白卡)
 ************************************************************************************************************/
void Start_Tracking_Back(void)
{
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_BACK_TO_CROSS;
	Moving_ByEncoder = ENCODER_NONE;
	Car_Speed = -40;
}
/************************************************************************************************************
 【函 数】:Start_Turn_Check
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:转弯后开始矫正
 ************************************************************************************************************/
void Start_Turn_Check(void)
{
	if (!Turn_Check)
		return;
	Turn_check = 1;
	Stop_Flag = TRACKING;			   // 告诉车要启动了
	Track_Mode = TrackMode_Turn_CHECK; // 告诉车用红外循迹
	Moving_ByEncoder = ENCODER_NONE;   // 告诉车别用码盘循迹
	Car_Speed = 30;					   // 设置车速
	delay(1000);
	Wait_For_Turn_Check();
}
/************************************************************************************************************
 【函 数】:Turn_ByEncoder
 【参 数】:digree:转弯的角度
 【返 回】:
 【简 例】:
 【说 明】:根据码盘值转任意角度
 ************************************************************************************************************/
void Turn_ByEncoder(int16_t digree)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;
	Moving_ByEncoder = ENCODER_TurnByValue;
	if (digree >= 0)
	{
		Update_MotorSpeed(Turn_Speed, -Turn_Speed);
		TurnByEncoder_Value = digree * ClockWiseDigreeToEncoder;
	}
	else
	{
		Update_MotorSpeed(-Turn_Speed, Turn_Speed);
		TurnByEncoder_Value = -digree * CountClockWiseDigreeToEncoder;
	}
}
/************************************************************************************************************
 【函 数】:Turn_ByTrack
 【参 数】:dir:转弯的角度
 【返 回】:
 【简 例】:
 【说 明】:用红外循迹方式转向
 ************************************************************************************************************/
void Turn_ByTrack(Direction_t dir)
{
	if ((dir != DIR_RIGHT) && (dir != DIR_LEFT))
	{
		print_info("Turn Dir ERROR\r\n");
		return;
	}
	turnLeftOrRight = dir;

	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_Turn;
	Moving_ByEncoder = ENCODER_NONE;

	if (dir == DIR_RIGHT)
	{
		Update_MotorSpeed(Turn_Speed, -Turn_Speed);
	}
	else if (dir == DIR_LEFT)
	{
		Update_MotorSpeed(-Turn_Speed, Turn_Speed);
	}
}
/************************************************************************************************************
 【函 数】:Resume_RunningStatus
 【参 数】:encoderChangeValue: 前后设定码盘差值
 【返 回】:
 【简 例】:
 【说 明】:恢复状态遇到白卡前储存的状态
 ************************************************************************************************************/
void Resume_RunningStatus(uint16_t encoderChangeValue)
{
	extern uint16_t Mp_Value;
	uint16_t Roadway_mp_Get(void);

	Roadway_mp_syn(); // 同步码盘
	Mp_Value = Roadway_mp_Get();

	Moving_ByEncoder = RunningStatus.movingByencoder;
	Stop_Flag = RunningStatus.stopFlag;
	Track_Mode = RunningStatus.trackMode;
	// 循迹信息已清空,需要重新计算并减去执行中的行进值
	temp_MP = Mp_Value + RunningStatus.remainEncoderValue;
	int8_t currentSpeed = RunningStatus.currentSpeed;
	Update_MotorSpeed(currentSpeed, currentSpeed);
	Submit_SpeedChanges();
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 前后运动 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:Back_To_Cross
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:倒车至黑线,用于短距离调整,不能循迹
 ************************************************************************************************************/
void Back_To_Cross(void)
{
	if (!Need_Back_To_Cross)
		return;

	Start_Tracking_Back();		// 开始倒退
	Wait_For_Action(CROSSROAD); // 直至到黑线
	MOVE(2);					// 向前2厘米防止判成十字路口
}
/************************************************************************************************************
 【函 数】:Back_To_Cross
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:行驶至黑线,用于短距离调整,不能循迹
 ************************************************************************************************************/
void Go_To_Cross(void)
{
	delay(1000);
	Start_Tracking();
	delay(1000);
	Wait_For_Action(FORBACKCOMPLETE);
	delay(1000);
	MOVE(ToCrossroadCenter);
}
/************************************************************************************************************
 【函 数】:Move_ByEncoder
 【参 数】:speed:车速	distance:距离
 【返 回】:
 【简 例】:
 【说 明】:前后移动 单位厘米 正负方向 (不会矫正循迹, 垂直运动)
 ************************************************************************************************************/
void Move_ByEncoder(int speed, float distance)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;
	Track_Mode = TrackMode_NONE;

	if (distance > 0)
	{
		temp_MP = distance * Centimeter_Value;
		Moving_ByEncoder = ENCODER_GO;
		Update_MotorSpeed(speed, speed);
	}
	else
	{
		temp_MP = -distance * Centimeter_Value;
		Moving_ByEncoder = ENCODER_BACK;
		Update_MotorSpeed(-speed, -speed);
	}
}
/************************************************************************************************************
 【函 数】:Track_ByEncoder
 【参 数】:speed:车速	distance:距离
 【返 回】:
 【简 例】:
 【说 明】:根据码盘设定值循迹 (会矫正循迹)
 ************************************************************************************************************/
void Track_ByEncoder(int speed, float distance)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;				   // 告诉车开始动
	Track_Mode = TrackMode_ENCODER;		   // 告诉车用码盘循迹
	temp_MP = distance * Centimeter_Value; // 行驶的距离(厘米)
	Car_Speed = speed;
}
/************************************************************************************************************
 【函 数】:Track_ByEncoder
 【参 数】:speed:车速	distance:距离
 【返 回】:
 【简 例】:
 【说 明】:根据码盘设定值循迹 (会矫正循迹)
 ************************************************************************************************************/
void Track_ByEncoder_With_SpecialRoad(int speed, float distance)
{
	Roadway_mp_syn();
	Stop_Flag = TRACKING;						// 告诉车开始动
	Track_Mode = TrackMode_ENCODER_SpecialRoad; // 告诉车用码盘循迹
	temp_MP = distance * Centimeter_Value;		// 行驶的距离(厘米)
	Car_Speed = speed;
}
/*TODO*******************************************************************************************************
 【函 数】:Move_By_Step
 【参 数】:distance:距离 一般都是个位数
 【返 回】:
 【简 例】:
 【说 明】:一步一步挪动,用于白卡扫描
 ************************************************************************************************************/
void Move_By_Step(float distance)
{
	for (size_t i = 0; i < distance / 2; i++)
	{
		Track_ByEncoder_With_SpecialRoad(Step_Speed, 2);
		Read_RFID();
	}
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 原地运动 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:TurnOnce_TrackMethod
 【参 数】:dir:转向角度
 【返 回】:
 【简 例】:
 【说 明】:左右转,循迹线模式
 ************************************************************************************************************/
void TurnOnce_TrackMethod(Direction_t dir)
{
	ExcuteAndWait(Turn_ByTrack(dir), Stop_Flag, TURNCOMPLETE);
}
/************************************************************************************************************
 【函 数】:TurnOnce_EncoderMethod
 【参 数】:dir:转向角度
 【返 回】:
 【简 例】:
 【说 明】:左右转,码盘值模式
 ************************************************************************************************************/
void TurnOnce_EncoderMethod(Direction_t dir)
{
	TURN((dir == DIR_LEFT) ? -90 : 90);
}
/************************************************************************************************************
 【函 数】:Wait_For_Turn_Check
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:转向后矫正姿态
 ************************************************************************************************************/
void Wait_For_Turn_Check(void)
{
	Wait_For_Action(TURNCHECK);
	Turn_check = 0;
}
/************************************************************************************************************
 【函 数】:Turn_And_Check
 【参 数】:dir为地图方向 而非小车方向
 【返 回】:
 【简 例】:
 【说 明】:转向后矫正姿态
 ************************************************************************************************************/
void Turn_And_Check(Direction_t dir)
{
	TURN_TO(dir);
	delay(2000);
	Start_Turn_Check();
	delay(1000);
	Start_Turn_Check();
	delay(1000);
}
