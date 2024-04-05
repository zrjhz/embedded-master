#ifndef __ROADWAY_H
#define __ROADWAY_H

#include "sys.h"
#include "movement.h"

// 速度设定常量↓

// 循迹速度
#define Track_Speed 50 // 55
// 转弯速度0
#define Turn_Speed 85 // 85
// 任务执行中调整速度
#define Mission_Speed 40 // 慢速防止打滑
// 挪动速度
#define Step_Speed 30

// 角度转换到码盘常量↓

// 顺时针转换倍数
#define ClockWiseDigreeToEncoder 9.0f // 旧赛道 10.1f
// 逆时针转换倍数
#define CountClockWiseDigreeToEncoder 9.30f // 旧 9.83f

// 循迹距离常量↓

// 每厘米的码盘值
#define Centimeter_Value 28.94f // 28.94f
// 线宽
static const int Track_Width = 3.0f; // 3.0f
// X轴中点循迹值
static const int LongTrack_Value = 37.0f; // 37.5f
// Y轴中点循迹值
static const int ShortTrack_Value = 30.0f; // 20.0f
// 到十字路口中心码盘值
static const float ToCrossroadCenter = 14.0f; // 16.5f
// 白卡移动距离
static const int CardTrack_Value = 2.5f; // 2.5f 0.5的倍数
// 两侧车库最大前进值
static const int SidePark_Value = 27.5f; // 27.5f
// 特殊地形长度
static const int Speical_Road_Value = 37.5f; // 37.5f

#define _STOP_WAITING_INTERVAL_ 250

// 每秒行进距离（速度50）
static const float _CentermetersPerSecondAtSpeed50 = 36.0;

// 循迹模式
typedef enum
{
    TrackMode_NONE = 0,           // 没有在动
    TrackMode_NORMAL,             // 红外+矫正循迹
    TrackMode_ENCODER,            // 码盘+矫正循迹
    TrackMode_Turn,               // 转弯
    TrackMode_BACK_TO_CROSS,      // 退回黑线
    TrackMode_Turn_CHECK,         // 转弯矫正
    TrackMode_ENCODER_SpecialRoad, // 过特殊地形
} TrackMode_t;                    // 车子是因为什么在动

// 根据码盘值运动的模式
typedef enum
{
    ENCODER_NONE = 0,
    ENCODER_GO,
    ENCODER_BACK,
    ENCODER_TurnByValue
} Moving_ByEncoder_t;

typedef enum
{
    CONDITION_CROSSROAD,
    CONDITION_CARD_CROSSROAD,
    CONDITION_CARD_STRAIGHT,
    CONDITION_STRAIGHT
} Condition_t;

// 使能循迹信息输出
#define _TRACK_OUTPUT_ 0

// 大于等于此数判定出线/遇到白卡
#define ALL_WHITE 15
#define IS_All_WHITE() (NumberOfWhite >= ALL_WHITE)
// 低于此数判定撞线
#define ALL_BLACK 9
#define IS_ALL_BLACK() (NumberOfWhite <= ALL_BLACK)
//
#define ONE_WHITE 14
#define IS_ONE_WHITE() (NumberOfWhite == ONE_WHITE)

// 循迹灯信息
extern int8_t Q7[7],
    H8[8];
// 循迹灯亮起的个数
extern uint8_t NumberOfWhite;
// 循迹模式
extern uint8_t Track_Mode;
// 定值前后和转向
extern Moving_ByEncoder_t Moving_ByEncoder;
// 定角度转向目标码盘值
extern uint16_t TurnByEncoder_Value;
// 路况
extern Condition_t Road_Condition;

extern uint8_t Turn_check;

extern uint8_t Stop_Flag;
extern int Car_Speed;
extern uint16_t temp_MP;
extern uint8_t Force_Break;
extern float offset;

void Roadway_Flag_clean(void);
void Roadway_mp_syn(void);
uint16_t Roadway_mp_Get(void);
void Roadway_CheckTimInit(uint16_t arr, uint16_t psc);

void Update_MotorSpeed(int LSpeed, int RSpeed);
void Submit_SpeedChanges(void);
void Get_Track(void);
float Get_Offset(void);

#endif
