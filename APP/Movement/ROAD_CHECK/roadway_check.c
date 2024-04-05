#include "roadway_check.h"
#include "delay.h"
#include "cba.h"
#include "tba.h"
#include "movement.h"
#include "debug.h"
#include "pid.h"
#include "my_lib.h"
#include "canp_hostcom.h"
#include "independent_task.h"
#include "task.h"

#define _ENABLE_TURNING_BY_TRACK_ 1

// 转向是否完成
#define CheckTurnComplete(EncoderValue) \
    if (Mp_Value >= EncoderValue)       \
    {                                   \
        Stop_Set_Flag(TURNCOMPLETE);    \
    }
// 前七个循迹传感器
int8_t Q7[7] = {0};
// 后八个循迹传感器
int8_t H8[8] = {0};
// 所有循迹传感器数据按左到右顺序排列
int8_t ALL_TRACK[15];
// 当前白色数量
uint8_t NumberOfWhite = 0;

// 运行状态
uint8_t Stop_Flag = TRACKING;
// 循迹模式
uint8_t Track_Mode = TrackMode_NONE;
// 定值前后和转向
Moving_ByEncoder_t Moving_ByEncoder = ENCODER_NONE;
// 路况
Condition_t Road_Condition = CONDITION_STRAIGHT;
// 定角度转向目标码盘值
uint16_t TurnByEncoder_Value = 0;

// 左右速度
int LSpeed = 0, RSpeed = 0;
// 循迹时车速
int Car_Speed = 0;
// 目标码盘值
uint16_t temp_MP = 0;
// 当前的码盘值
uint16_t Mp_Value = 0;
// 之前获取的码盘值
int16_t Roadway_cmp;
// 从CAN获取的实时码盘值
extern int16_t CanHost_Mp;
// 误差值
extern float offset = 0;
// 强制退出
extern uint8_t Force_Break = 0;
// 转弯调整
extern uint8_t Turn_check = 0;

// 码盘同步
void Roadway_mp_syn(void)
{
    Roadway_cmp = CanHost_Mp;
}
// 码盘获取
uint16_t Roadway_mp_Get(void)
{
    uint32_t ct;
    if (CanHost_Mp > Roadway_cmp)
        ct = CanHost_Mp - Roadway_cmp;
    else
        ct = Roadway_cmp - CanHost_Mp;
    if (ct > 0x8000)
        ct = 0xffff - ct;

    return ct;
}
// 清除标志位
void Roadway_Flag_clean(void)
{
    Stop_Flag = TRACKING;
    temp_MP = 0;
    Track_Mode = TrackMode_NONE;
    Moving_ByEncoder = ENCODER_NONE;
}
// 前后,转弯监测
void Moving_ByEncoderCheck(void)
{
    switch (Moving_ByEncoder)
    {
    case ENCODER_GO:
        if (temp_MP <= Mp_Value)
            Stop_Set_Flag(FORBACKCOMPLETE);
        break;
    case ENCODER_BACK:
        if (temp_MP <= Mp_Value)
            Stop_Set_Flag(FORBACKCOMPLETE);
        break;
    case ENCODER_TurnByValue:
        CheckTurnComplete(TurnByEncoder_Value);
    default:
        break;
    }
}
// 更新速度值（速度区间 -100 ~ 100)
void Update_MotorSpeed(int L_Speed, int R_Speed)
{
    // 速度限幅
    LSpeed = constrain_int(L_Speed, -100, 100);
    RSpeed = constrain_int(R_Speed, -100, 100);
}
// 电机控制,提交电机速度更改
void Submit_SpeedChanges(void)
{
    static int preLSpeed, preRSpeed;  // 上次的速度数据
    static uint32_t preSpeedChanging; // 上次变更速度的时间戳

    // 速度值改变则上传数据
    if (LSpeed != preLSpeed || RSpeed != preRSpeed)
    {
        preLSpeed = LSpeed;
        preRSpeed = RSpeed;
        Send_UpMotor(LSpeed, RSpeed);
        preSpeedChanging = millis(); // 更新时间戳
    }
    else
    {
        // 间隔一定时间后发送一次速度信息
        // 防止数据丢失造成匀速行驶时的严重错误
        if (IsTimeOut(preSpeedChanging, 150))
        {
            Send_UpMotor(LSpeed, RSpeed);
            preSpeedChanging = millis();
        }
    }
}
// 获取循迹信息,计算白色的数量
void Get_Track(void)
{
    if (!Get_TrackInfoReceived())
        return;
    else
    {
        Set_TrackInfoReceived(false);
    }

    uint16_t tmp = Get_Host_UpTrack(TRACK_ALL);
    // 清空循迹灯亮起个数
    NumberOfWhite = 0;

    // 获取循迹灯信息和循迹灯亮灯数量
    for (uint8_t i = 0; i < 7; i++)
    {
        Q7[i] = (tmp >> i) & 0x01;
        H8[i] = (tmp >> (8 + i)) & 0x01;

        NumberOfWhite += Q7[i] ? 1 : 0;
        NumberOfWhite += H8[i] ? 1 : 0;

        ALL_TRACK[i * 2] = H8[i];
        ALL_TRACK[i * 2 + 1] = Q7[i];
    }
    H8[7] = (tmp >> (15)) & 0x01;
    NumberOfWhite += H8[7] ? 1 : 0;
    ALL_TRACK[14] = H8[7];
}
// 计算偏差值
float Get_Offset(void)
{
    // 计算各个点与临近点的和
    int8_t all_weights[15] = {0};
    for (uint8_t i = 1; i < 14; i++)
    {
        for (uint8_t j = i - 1; j <= i + 1; j++)
        {
            all_weights[i] += ALL_TRACK[j];
        }
    }

    // 遍历找到最小值
    int8_t minimum = 3;
    for (uint8_t i = 1; i < 14; i++)
    {
        if (minimum > all_weights[i])
            minimum = all_weights[i];
    }

    // 获取最小值正续和倒序编号（两个最小值情况）
    // 丢弃第一位和最后一位,没有数据
    int8_t errorValue1 = 0, errorValue2 = 14;
    for (;;)
    {
        if (all_weights[++errorValue1] == minimum)
            break;
    }
    for (;;)
    {
        if (all_weights[--errorValue2] == minimum)
            break;
    }

    // 平均之后计算误差值
    float errorValue = 7.0 - (errorValue1 + errorValue2) / 2.0;

// 滤波
#define FILTER_ARRAY_LENGTH 3
    static float filterArray[FILTER_ARRAY_LENGTH] = {0};
    static uint8_t currentNumber = 0;

    filterArray[currentNumber++] = errorValue;
    if (currentNumber >= FILTER_ARRAY_LENGTH)
        currentNumber = 0;

    errorValue = 0;
    for (uint8_t i = 0; i < FILTER_ARRAY_LENGTH; i++)
    {
        errorValue += filterArray[i];
    }
    errorValue /= (float)FILTER_ARRAY_LENGTH;

    return errorValue;
}
// 循迹
void TRACK_LINE(void)
{
    // 没有接收到循迹信息不进行运算
    if (Get_TrackInfoReceived())
    {
        Get_Track();
        offset = Get_Offset();
        Calculate_pid(offset);
    }
    switch (Track_Mode)
    {
    case TrackMode_ENCODER: // 码盘循迹
        if (temp_MP <= Mp_Value)
        {
            Stop_WithoutPIDClear();
            Stop_Flag = FORBACKCOMPLETE;
            return;
        }
        break;
    case TrackMode_ENCODER_SpecialRoad: // 特殊地形
        if (temp_MP <= Mp_Value)
        {
            Stop_WithoutPIDClear();
            Stop_Flag = FORBACKCOMPLETE;
            return;
        }
        if (Q7[0] && Q7[1] && Q7[2] && !Q7[3] && Q7[4] && Q7[5] && Q7[6])
            Update_MotorSpeed(Car_Speed + PID_value, Car_Speed - PID_value);
        else
            Update_MotorSpeed(Car_Speed, Car_Speed);
        return;
    case TrackMode_Turn_CHECK: // 矫正
        if (!Q7[3] && Q7[2] && Q7[4])
            Stop_Set_Flag(TURNCHECK);
        else
            Update_MotorSpeed(Car_Speed, Car_Speed);
        return;
    case TrackMode_BACK_TO_CROSS: // 回到黑线
        Update_MotorSpeed(Car_Speed, Car_Speed);
        if ((H8[0] + Q7[0] + H8[7] + Q7[6]) < 3 || IS_ALL_BLACK())
            Stop_Set_Flag(CROSSROAD);
        return;
    }
    
    if (Q7[2] && Q7[3] && Q7[4] && Find_RFID)
        Stop_Set_Flag(CARD);
    else if ((H8[0] + Q7[0] + H8[7] + Q7[6]) < 3 || IS_ALL_BLACK())
    {
        Stop_Set_Flag(CROSSROAD);
        Force_Break = 1;
    }
    else
        Update_MotorSpeed(Car_Speed + PID_value, Car_Speed - PID_value);
}

// 运动控制
void Roadway_Check(void)
{
    if (Track_Mode != TrackMode_NONE) // 红外循迹
        TRACK_LINE();
    if (Moving_ByEncoder != ENCODER_NONE) // 码盘循迹
        Moving_ByEncoderCheck();

    Submit_SpeedChanges();
}

// 路况检测（TIM9）
void Roadway_CheckTimInit(uint16_t arr, uint16_t psc)
{
    TIM_TimeBaseInitTypeDef TIM_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

    RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM9, ENABLE);

    TIM_InitStructure.TIM_Period = arr;
    TIM_InitStructure.TIM_Prescaler = psc;
    TIM_InitStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_InitStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitStructure.TIM_RepetitionCounter = 0;
    TIM_TimeBaseInit(TIM9, &TIM_InitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM1_BRK_TIM9_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 1;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);

    TIM_ITConfig(TIM9, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM9, ENABLE);
}

void TIM1_BRK_TIM9_IRQHandler(void)
{
    extern uint32_t lastStopStamp;

    if (TIM_GetITStatus(TIM9, TIM_IT_Update) == SET)
    {
        // DEBUG_PIN_2_SET();

        // 上一次停止时间未等待足够时间则不进行下一个动作,防止打滑
        if (IsTimeOut(lastStopStamp, _STOP_WAITING_INTERVAL_))
        {
            DEBUG_PIN_2_SET();

            Mp_Value = Roadway_mp_Get();
            Roadway_Check();

            DEBUG_PIN_2_RESET();
        }

        // DEBUG_PIN_2_RESET();
    }
    TIM_ClearITPendingBit(TIM9, TIM_IT_Update);
}
