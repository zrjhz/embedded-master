#include "independent_task.h"
#include <stdio.h>
#include <string.h>
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "bh1750.h"
#include "voice.h"
#include "roadway_check.h"
#include "tba.h"
#include "uart_a72.h"
#include "Rc522.h"
#include "debug.h"
#include "movement.h"
#include "route.h"
#include "my_lib.h"
#include "Timer.h"
#include "data_interaction.h"
#include "agv.h"
#include "canp_hostcom.h"
#include "stdlib.h"
#include "my_lib.h"
#include "task.h"
#include "protocol.h"
#include "can_user.h"

extern uint16_t h;              // 超声波数据
extern uint8_t *AGV_routeInfo;  // 从车路线
extern uint8_t *AGV_StartPoint; // 从车起点
extern uint8_t plate[6];        // 获取的车牌
extern uint8_t E;               // 第一张张扇区
extern uint8_t S;               // 第二张张扇区
extern uint8_t FN;              // 卡1ABCD频次
extern uint8_t s1;              // 卡1首个数字
extern uint8_t s2;              // 卡1末个数字
extern uint8_t M01;             // 卡2有效字符串

uint8_t card1_first = true; // 第一张卡是否为card1
// 白卡的标志位和指针

uint8_t RFID_Index = 0;
uint8_t success = 0;
// 当前卡信息指针
RFID_Info_t *CurrentRFIDCard = NULL;
// 所有白卡中的数据 长度为白卡数据总数量
uint8_t Current_Card_Data[10][17];
// TODO block = 4 * n + m
Block_Info_t RFID1_Block[2] = {
    {.block = 10, .authMode = PICC_AUTHENT1A, .key = {0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE}},
    {.block = 0, .authMode = PICC_AUTHENT1A, .key = {0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE}},
};
Block_Info_t RFID2_Block[2] = {
    {.block = 10, .authMode = PICC_AUTHENT1A, .key = {0xFE, 0xFF, 0xFF, 0xFF, 0xFF, 0xFE}},
    {.block = 0, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
};
Block_Info_t RFID3_Block[1] = {
    {.block = 37, .authMode = PICC_AUTHENT1A, .key = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}},
};
Block_Info_t RFID4_Block[2] = {
    {.block = 29, .authMode = PICC_AUTHENT1A, .key = {0x18, 0x19, 0xBF, 0xFB, 0xFC, 0xAB}},
    {.block = 34, .authMode = PICC_AUTHENT1A, .key = {0x18, 0x19, 0xBF, 0xFB, 0xFC, 0xAB}},
};

#define DefineRFIDCard(RFIDx)                           \
    RFID_Info_t RFIDx## = {                             \
        .blockInfo = RFIDx##_Block,                     \
        .blockNumber = GET_ARRAY_LENGEH(RFIDx##_Block), \
        .coordinate = {                                 \
            .x = 0,                                     \
            .y = 0,                                     \
            .dir = DIR_NOTSET,                          \
        },                                              \
    }
DefineRFIDCard(RFID1);
DefineRFIDCard(RFID2);
DefineRFIDCard(RFID3);
DefineRFIDCard(RFID4);

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ RFID处理 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/*TODO*******************************************************************************************************
 【函 数】:RFID_Start
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:开始识别rfid卡,里面的RFIDx_Block的内容根据题目修改数据块、模式、密钥
 ************************************************************************************************************/
void RFID_Start(void)
{
    Find_RFID = true;
    RFIDx_Begin(1);
}
/*TODO*******************************************************************************************************
 【函 数】:RFID_End
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:取消识别rfid卡,并根据题目整理路上扫描的数据
 ************************************************************************************************************/
void RFID_End(void)
{
    Find_RFID = false;
    CurrentRFIDCard = NULL;
    // TODO
}
/*TODO*******************************************************************************************************
 【函 数】:RFIDx_Begin
 【参 数】:index:白卡下标
 【返 回】:
 【简 例】:
 【说 明】:选择当前的白卡
*************************************************************************************************************/
void RFIDx_Begin(uint8_t index)
{
    RFID_Index = index;
    print_info("RFID%d start\r\n", RFID_Index);
    delay(1000);
    switch (index)
    {
    case 1:
        CurrentRFIDCard = &RFID1;
        break;
    case 2:
        CurrentRFIDCard = &RFID2;
        break;
    case 3:
        CurrentRFIDCard = &RFID3;
        break;
    case 4:
        CurrentRFIDCard = &RFID4;
        break;
    }
}
/************************************************************************************************************
 【函 数】:UseDefaultKey
 【参 数】:key:密钥地址     keyx:密钥值
 【返 回】:
 【简 例】:
 【说 明】:初始化默认KEY
 ************************************************************************************************************/
void UseDefaultKey(uint8_t *key, uint8_t keyx)
{
    for (uint8_t i = 0; i < 6; i++)
    {
        key[i] = keyx;
    }
}
/************************************************************************************************************
 【函 数】:Block_InfoInit
 【参 数】:blockInfo:块信息在车上的地址     blockNumber:块数量
 【返 回】:
 【简 例】:
 【说 明】:块信息初始化
 ************************************************************************************************************/
void Block_InfoInit(Block_Info_t *blockInfo, uint8_t blockNumber)
{
    memset(blockInfo, 0, sizeof(Block_Info_t) * blockNumber);

    for (uint8_t i = 0; i < blockNumber; i++)
    {
        blockInfo[i].authMode = PICC_AUTHENT1A;
    }
}
/************************************************************************************************************
 【函 数】:Read_RFID
 【参 数】:
 【返 回】:识别结果(失败,成功)
 【简 例】:Read_RFID();
 【说 明】:读取当前卡片信息
 ************************************************************************************************************/
uint8_t Read_RFID(void)
{
    if (CurrentRFIDCard == NULL)
        return 0;
    uint8_t status = 1;
    uint8_t tmpStatus;

    for (uint8_t i = 0; i < CurrentRFIDCard->blockNumber; i++)
    {
        tmpStatus = PICC_ReadBlock(CurrentRFIDCard->blockInfo[i], CurrentRFIDCard->data[i]);
        CurrentRFIDCard->data[i][16] = '\0';
        if (tmpStatus)
        {
            if (RFID_Index == 1)
            {
                switch (i)
                {
                case 0:
                    if (CurrentRFIDCard->data[i] == "ID01")
                    {
                        RFID1_Block[1].block = 4 * E + 1;
                        RFID2_Block[1].block = 4 * S + 1;
                        card1_first = true;
                    }
                    else
                    {
                        RFID1_Block[1].block = 4 * S + 1;
                        RFID2_Block[1].block = 4 * E + 1;
                        card1_first = false;
                    }
                    break;
                case 1:
                    if (card1_first)
                    {
                        FN = countLetter(CurrentRFIDCard->data[i]);
                        s1 = getFirstDigit(CurrentRFIDCard->data[i], 1, 0);
                        s2 = getFirstDigit(CurrentRFIDCard->data[i], 1, 1);
                    }
                    else
                    {
                        extractDigits(CurrentRFIDCard->data[i], M01);
                    }
                    break;
                }
            }
            else
            {
                if (i == 1)
                {
                    if (!card1_first)
                    {
                        FN = countLetter(CurrentRFIDCard->data[i]);
                        s1 = getFirstDigit(CurrentRFIDCard->data[i], 1, 0);
                        s2 = getFirstDigit(CurrentRFIDCard->data[i], 1, 1);                                                                        
                    }
                    else
                    {
                        extractDigits(CurrentRFIDCard->data[i], M01);
                    }
                }
            }
        }
        status &= tmpStatus;
    }
    if (status)
    {
        switch (RFID_Index) // 用switch方便操作
        {
        case 1:
            RFIDx_Begin(2);
            break;
        case 2:
            // RFID_End();
            break;
        }
    }

    delay(100);
    return status;
}
/************************************************************************************************************
 【函 数】:Write_RFID
 【参 数】:RFIDx:卡的指针       data_str:写入的信息
 【返 回】:写入结果(失败,成功)
 【简 例】:
 【说 明】:写入信息到当前卡片
 ************************************************************************************************************/
ErrorStatus Write_RFID(RFID_Info_t *RFIDx, uint8_t *data_str)
{
    ErrorStatus status = SUCCESS;
    // ErrorStatus tmpStatus;

    // for (uint8_t i = 0; i < RFIDx->blockNumber; i++)
    // {

    //     tmpStatus = PICC_WriteBlock(&RFIDx->blockInfo[i], data_str);

    //     if (tmpStatus)
    //     {
    //         print_info("Block%d success, ", RFIDx->blockInfo[i].block);
    //         print_info("data load success\n");
    //     }
    //     else
    //     {
    //         print_info("Block%d fail\r\n", RFIDx->blockInfo[i].block);
    //     }

    //     status &= tmpStatus;
    //     delay(100);
    // }

    print_info("RFID %s\r\n", status ? "all pass" : "something wrong");

    return status;
}
/************************************************************************************************************
 【函 数】:RFID_Task
 【参 数】:index:第几张卡
 【返 回】:
 【简 例】:RFID(1); 第一张白卡
 【说 明】:识别到白卡并识别,MOVE的值根据地图差异可以改改
 ************************************************************************************************************/
void RFID_Task(void)
{
    float count = 0;
    float step = 0.5f;
    success = 0;

    CurrentRFIDCard->coordinate = NextStatus;

    Get_Track();
    if ((H8[0] + Q7[0] + H8[7] + Q7[6]) < 4 || count != 0)
        count += step;

    MOVE(2.5);
    while (Q7[2] && Q7[3] && Q7[4])
    {
        MOVE(step);
        Get_Track();
        if ((H8[0] + Q7[0] + H8[7] + Q7[6]) < 4 || count != 0)
            count += step;
        if (!success)
            success = Read_RFID();
    }

    for (size_t i = 0; i < 5; i++)
    {
        MOVE(step);
        Get_Track();
        if ((H8[0] + Q7[0] + H8[7] + Q7[6]) < 4 || count != 0)
            count += step;
        if (!success)
            success = Read_RFID();
    }

    if (count == 0) // 未判断到黑线 走到白卡出头的位置
    {
        MOVE(-2.5);
        Start_Tracking(); // 一直前进
    }
    else // 判断到黑线 走到黑线的位置
    {
        MOVE(-count);
        Stop_Set_Flag(CROSSROAD);
        Force_Break = 1;
    }
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 特殊地形处理 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:SpecialRoad_ReturnStatus
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:询问特殊地形状态
 ************************************************************************************************************/
void SpecialRoad_ReturnStatus(void)
{
    ZigBee_SpecialRoadData[Pack_MainCmd] = SpecialRoadMode_ReturnStatus;
    ZigBee_SpecialRoadData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeData(ZigBee_SpecialRoadData);
}
/************************************************************************************************************
 【函 数】:Get_BarrierGateStatus
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取特殊地形状态
 ************************************************************************************************************/
bool Get_SpecialRoadStatus(void)
{
    Reset_ZigBeeReturnStatus(SpecialRoad);
    for (uint8_t i = 0; i < 3; i++)
    {

        SpecialRoad_ReturnStatus();
        WaitZigBeeFlag(SpecialRoad, 300);
        if (Get_ZigBeeReturnStatus(SpecialRoad))
        {
            print_info("status:%X\r\n", Get_ZigBeeReturnData(SpecialRoad)[Pack_SubCmd2]);
            return !(Get_ZigBeeReturnData(SpecialRoad)[Pack_SubCmd2] == 0x33);
        }
    }
    return false;
}
/************************************************************************************************************
 【函 数】:SpecialRoad_End
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取特殊地形状态
 ************************************************************************************************************/
void SpecialRoad_End(void)
{
    Find_SpecialRoad = false;
    delay(1000);
    Need_Back_To_Cross = true;
    delay(1000);
    Back_To_Cross();
    Stop();
    Submit_SpeedChanges();
    delay(1000);
    MOVE(ToCrossroadCenter - 2);
    delay(1000);
}
/************************************************************************************************************
 【函 数】:SpecialRoad_End
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取特殊地形状态
 ************************************************************************************************************/
void SpecialRoad_Start(void)
{
    Find_SpecialRoad = true;
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 烽火台（报警台） ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:Alarm_ON
 【参 数】:code:报警码
 【返 回】:
 【简 例】:
 【说 明】:输入报警码,红外发射给报警台
 ************************************************************************************************************/
void Alarm_ON(uint8_t code[6])
{
    Infrared_Send(code, 6);
}
/************************************************************************************************************
 【函 数】:AlarmReturn_RescueCoord
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:请求回传随机救援坐标点
 ************************************************************************************************************/
void AlarmReturn_RescueCoord(void)
{
    ZigBee_AlarmData[Pack_MainCmd] = Alarm_ReturnResCoord;
    Send_ZigBeeData5Times(ZigBee_AlarmData);
}
/************************************************************************************************************
 【函 数】:Alarm_ChangeCode
 【参 数】:code:报警码
 【返 回】:
 【简 例】:
 【说 明】:报警台更改报警码
 ************************************************************************************************************/
void Alarm_ChangeCode(uint8_t code[6])
{
    ZigBee_AlarmData[Pack_MainCmd] = Alarm_CodeFront3Bytes;
    memcpy(&ZigBee_AlarmData[Pack_SubCmd1], code, 3);
    Send_ZigBeeData5Times(ZigBee_AlarmData);
    //	Send_ZigBeeData(ZigBee_AlarmData);

    ZigBee_AlarmData[Pack_MainCmd] = Alarm_CodeBack3Bytes;
    memcpy(&ZigBee_AlarmData[Pack_SubCmd1], &code[3], 3);
    Send_ZigBeeData5Times(ZigBee_AlarmData);
    Send_ZigBeeData(ZigBee_AlarmData);
    // warning：上面为了防止报警灯响起多次只发送一次,需要的话可发送多次
    //    Send_ZigBeeDataNTimes(ZigBee_AlarmData, 2, 100);
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 道闸 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:BarrierGate_Plate
 【参 数】:plate:车牌信息
 【返 回】:
 【简 例】:
 【说 明】:道闸显示车牌
 ************************************************************************************************************/
void BarrierGate_Plate(uint8_t plate[6])
{
    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_PlateFront3Bytes;
    memcpy(&ZigBee_BarrierGateData[Pack_SubCmd1], plate, 3);
    Send_ZigBeeData5Times(ZigBee_BarrierGateData);

    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_PlateBack3Bytes;
    memcpy(&ZigBee_BarrierGateData[Pack_SubCmd1], &plate[3], 3);
    Send_ZigBeeData5Times(ZigBee_BarrierGateData);
}
/************************************************************************************************************
 【函 数】:BarrierGate_AdjustAngle
 【参 数】:Gate_UP 上升 / Gate_Down 下降
 【返 回】:
 【简 例】:
 【说 明】:道闸调节初始角度
 ************************************************************************************************************/
void BarrierGate_AdjustAngle(uint8_t angle)
{
    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_GateInitialAngle;
    ZigBee_BarrierGateData[Pack_SubCmd1] = angle;
    Send_ZigBeeData(ZigBee_BarrierGateData);
}
/************************************************************************************************************
 【函 数】:BarrierGate_Control
 【参 数】:status:开关
 【返 回】:
 【简 例】:
 【说 明】:道闸控制
 ************************************************************************************************************/
void BarrierGate_Control(bool status)
{
    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_Control;
    ZigBee_BarrierGateData[Pack_SubCmd1] = status ? 0x01 : 0x02;
    Send_ZigBeeData5Times(ZigBee_BarrierGateData);
}
/************************************************************************************************************
 【函 数】:BarrierGate_ReturnStatus
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:道闸回传状态
 ************************************************************************************************************/
void BarrierGate_ReturnStatus(void)
{
    ZigBee_BarrierGateData[Pack_MainCmd] = BarrierGateMode_ReturnStatus;
    ZigBee_BarrierGateData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeData(ZigBee_BarrierGateData);
}
/************************************************************************************************************
 【函 数】:BarrierGate_Task
 【参 数】:plate:车牌
 【返 回】:
 【简 例】:
 【说 明】:道闸任务; 显示车牌并打开道闸
 ************************************************************************************************************/
void BarrierGate_Task(uint8_t plate[6])
{
    for (uint8_t i = 0; i < 3; i++)
    {
        if (plate != NULL)
        {
            BarrierGate_Plate(plate);
        }
        else
        {
            BarrierGate_Control(0x01);
        }
        if (Get_BarrierGateStatus() == true)
            break;
    }
}
/************************************************************************************************************
 【函 数】:Get_BarrierGateStatus
 【参 数】:
 【返 回】:true：开启 false：关闭或超时
 【简 例】:
 【说 明】:获取道闸状态
 ************************************************************************************************************/
bool Get_BarrierGateStatus(void)
{
    Reset_ZigBeeReturnStatus(BarrierGate);
    for (uint8_t i = 0; i < 3; i++)
    {

        BarrierGate_ReturnStatus();
        WaitZigBeeFlag(BarrierGate, 300);
        if (Get_ZigBeeReturnStatus(BarrierGate))
        {
            return (Get_ZigBeeReturnData(BarrierGate)[Pack_SubCmd2] == 0x05);
        }
    }
    return false;
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ ETC ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:ETC_GateAngleAdjust
 【参 数】:angle:角度
 【返 回】:
 【简 例】:
 【说 明】:闸门初始角度调节
 ************************************************************************************************************/
void ETC_GateAngleAdjust(uint8_t angle)
{
    ZigBee_ETCData[Pack_MainCmd] = ETCMode_GateInitialAngle;
    ZigBee_BarrierGateData[Pack_SubCmd1] = angle; // 左侧闸门
    ZigBee_BarrierGateData[Pack_SubCmd2] = angle; // 右侧闸门
    Send_ZigBeeData(ZigBee_BarrierGateData);
}
/************************************************************************************************************
 【函 数】:ETC_Task
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:ETC任务
 ************************************************************************************************************/
void ETC_Task(void)
{
    for (uint8_t i = 0; i < 10; i++) // 调整10次,不开直接走
    {
        // 六秒前的数据作废
        if ((ETC_Status.isSet == SET) && (!IsTimeOut(Get_ZigBeeReturnStamp(ETC), 6 * 1000)))
            break;
        MOVE(-6);
        MOVE(6);
    }
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ LED数码管显示 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:LEDDisplay_DataToFistRow
 【参 数】:data:数据
 【返 回】:
 【简 例】:
 【说 明】:LED显示标志物第一行显示数据
 ************************************************************************************************************/
void LEDDisplay_DataToFistRow(uint8_t data[3])
{
    ZigBee_LEDDisplayData[Pack_MainCmd] = LEDDisplayMainCmd_DataToFirstRow;
    memcpy(&ZigBee_LEDDisplayData[Pack_SubCmd1], data, 3);
    Send_ZigBeeData5Times(ZigBee_LEDDisplayData);
}
/************************************************************************************************************
 【函 数】:LEDDisplay_DataToSecondRow
 【参 数】:data:数据
 【返 回】:
 【简 例】:
 【说 明】:LED显示标志物第二行显示数据
 ************************************************************************************************************/
void LEDDisplay_DataToSecondRow(uint8_t data[3])
{
    ZigBee_LEDDisplayData[Pack_MainCmd] = LEDDisplayMainCmd_DataToSecondRow;
    memcpy(&ZigBee_LEDDisplayData[Pack_SubCmd1], data, 3);
    Send_ZigBeeData5Times(ZigBee_LEDDisplayData);
}
/************************************************************************************************************
 【函 数】:LEDDisplay_TimerMode
 【参 数】:mode:TimerMode_t模式
 【返 回】:
 【简 例】:
 【说 明】:LED显示标志物进更改计时模式
 ************************************************************************************************************/
void LEDDisplay_TimerMode(TimerMode_t mode)
{
    ZigBee_LEDDisplayData[Pack_MainCmd] = LEDDisplayMainCmd_TimerMode;
    ZigBee_LEDDisplayData[Pack_SubCmd1] = (uint8_t)mode;
    Send_ZigBeeData5Times(ZigBee_LEDDisplayData);
}
/************************************************************************************************************
 【函 数】:LEDDisplay_Distance
 【参 数】:dis:距离值
 【返 回】:
 【简 例】:
 【说 明】:LED显示标志物显示距离
 ************************************************************************************************************/
void LEDDisplay_Distance(uint16_t dis)
{
    ZigBee_LEDDisplayData[Pack_MainCmd] = LEDDisplayMainCmd_ShowDistance;
    ZigBee_LEDDisplayData[Pack_SubCmd2] = HEX2BCD(dis / 100);
    ZigBee_LEDDisplayData[Pack_SubCmd3] = HEX2BCD(dis % 100);
    Send_ZigBeeData5Times(ZigBee_LEDDisplayData);
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 立体显示（旋转LED） ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:RotationLED_PlateAndCoord
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED显示车牌和坐标
 ************************************************************************************************************/
void RotationLED_PlateAndCoord(uint8_t plate[6], RouteNode_t coord)
{
    uint8_t *stringCoord = ReCoordinate_Convert(coord);
    Infrared_RotationLEDData[1] = RotationLEDMode_PlateFront4BytesData;
    memcpy(&Infrared_RotationLEDData[2], plate, 4);
    Infrared_Send_A(Infrared_RotationLEDData);
    delay_ms(600);

    Infrared_RotationLEDData[1] = RotationLEDMode_PlateBack2BytesAndCoordInfo;
    memcpy(&Infrared_RotationLEDData[2], &plate[4], 2);
    memcpy(&Infrared_RotationLEDData[4], stringCoord, 2);
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_Distance
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED显示距离 warning: 只能显示两位数
 ************************************************************************************************************/
void RotationLED_Distance(uint8_t dis)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_Distance;
    Infrared_RotationLEDData[2] = ((dis % 100) / 10) + 0x30;
    Infrared_RotationLEDData[3] = (dis % 10) + 0x30;
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_Shape
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED显示图形
 ************************************************************************************************************/
void RotationLED_Shape(Shape_t shape)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_Shape;
    Infrared_RotationLEDData[2] = (uint8_t)shape;
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_Color
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED显示颜色
 ************************************************************************************************************/
void RotationLED_Color(Color_t color)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_Color;
    Infrared_RotationLEDData[2] = (uint8_t)color;
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_Default
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED显示默认数据
 ************************************************************************************************************/
void RotationLED_Default(void)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_Default;
    Infrared_RotationLEDData[2] = 0x01;
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_TrafficWarningSign
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED显示交通警示牌
 ************************************************************************************************************/
void RotationLED_TrafficWarningSign(RouteTrafficWarnSign_t TraWarnSign)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_TrafficWarnSign;
    Infrared_RotationLEDData[2] = (uint8_t)TraWarnSign;
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_TrafficSign
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED显示交通标志
 ************************************************************************************************************/
void RotationLED_TrafficSign(RouteTrafficSign_t TraSign)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_TrafficSign;
    Infrared_RotationLEDData[2] = (uint8_t)TraSign;
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_SetTextColor
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED设置文字颜色
 ************************************************************************************************************/
void RotationLED_SetTextColor(uint8_t Red, uint8_t Green, uint8_t Blue)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_SetTextColor;
    Infrared_RotationLEDData[2] = 0x01;
    Infrared_RotationLEDData[3] = Red;
    Infrared_RotationLEDData[4] = Green;
    Infrared_RotationLEDData[5] = Blue;
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_TextOverlay
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED自定义文本累加显示
 ************************************************************************************************************/
void RotationLED_TextOverlay(uint8_t *chr)
{
    uint8_t i = 0;
    for (i = 0; chr[i + 2] != '\0'; i += 2)
    {
        RotationLED_TextAccumulation(chr[i], chr[i + 1], 0);
        delay_ms(10);
    }
    RotationLED_TextAccumulation(chr[i], chr[i + 1], 1);
}
/************************************************************************************************************
 【函 数】:RotationLED_TextAccumulation
 【参 数】:choice:0 累加,1 结束
 【返 回】:
 【简 例】:
 【说 明】:旋转LED自定义文本累加显示(红外协议)
 ************************************************************************************************************/
void RotationLED_TextAccumulation(uint8_t chineseData1, uint8_t chineseData2, uint8_t choice)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_TextAccumulation;
    Infrared_RotationLEDData[2] = chineseData1;
    Infrared_RotationLEDData[3] = chineseData2;
    if (choice == 0)
        Infrared_RotationLEDData[4] = 0x00;
    else
        Infrared_RotationLEDData[4] = 0x55;
    Infrared_Send_A(Infrared_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_TextClear
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED自定义文本清空显示
 ************************************************************************************************************/
void RotationLED_TextClear(uint8_t ClearMode)
{
    Infrared_RotationLEDData[1] = RotationLEDMode_TextClear;
    Infrared_RotationLEDData[2] = ClearMode;
}
/************************************************************************************************************
 【函 数】:RotationLEDZigBee_TextAccumulation
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED自定义文本累加显示（ZigBee协议）
 ************************************************************************************************************/
void RotationLEDZigBee_TextAccumulation(uint8_t chineseData1, uint8_t chineseData2, uint8_t choice)
{
    ZigBee_RotationLEDData[Pack_MainCmd] = RotationLEDMode_TextAccumulation;
    ZigBee_RotationLEDData[Pack_SubCmd1] = chineseData1;
    ZigBee_RotationLEDData[Pack_SubCmd2] = chineseData2;
    if (choice == 0)
        ZigBee_RotationLEDData[Pack_SubCmd3] = 0x00;
    else
        ZigBee_RotationLEDData[Pack_SubCmd3] = 0x55;
    Send_ZigBeeData(ZigBee_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_zigdie
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED自定义文本累加显示(杨工说：“表达13要0x31 0x00 0x33 0x00.”)
 ************************************************************************************************************/
void RotationLED_zigdie(uint8_t *chr)
{
    uint8_t i = 0;
    for (i = 0; chr[i + 2] != '\0'; i += 2)
    {
        RotationLEDZigBee_TextAccumulation(chr[i], chr[i + 1], 0);
        delay_ms(10);
    }
    RotationLEDZigBee_TextAccumulation(chr[i], chr[i + 1], 1);
}
/************************************************************************************************************
 【函 数】:RotationLEDZigBee_TextClear
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED自定义文本清空显示（ZigBee协议）
 ************************************************************************************************************/
void RotationLEDZigBee_TextClear(uint8_t ClearMode)
{
    ZigBee_RotationLEDData[Pack_MainCmd] = RotationLEDMode_TextClear;
    ZigBee_RotationLEDData[Pack_SubCmd1] = ClearMode;
    Send_ZigBeeData(ZigBee_RotationLEDData);
}
/************************************************************************************************************
 【函 数】:RotationLED_CoordAndDistance
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:旋转LED显示坐标和距离
 ************************************************************************************************************/
void RotationLED_CoordAndDistance(uint16_t distance)
{
    uint8_t coord[6];
    memcpy(coord, ReCoordinate_Convert(RFID1.coordinate), 2);
    memcpy(&coord[2], ReCoordinate_Convert(RFID2.coordinate), 2);
    memcpy(&coord[4], ReCoordinate_Convert(RFID3.coordinate), 2);

    distance /= 10;

    uint8_t distanceStr[3];
    distanceStr[0] = ((distance % 100) / 10) + 0x30;
    distanceStr[1] = (distance % 10) + 0x30;

    Infrared_RotationLEDData[1] = RotationLEDMode_PlateFront4BytesData;
    memcpy(&Infrared_RotationLEDData[2], coord, 4);
    Infrared_Send_A(Infrared_RotationLEDData);
    delay_ms(600);

    Infrared_RotationLEDData[1] = RotationLEDMode_PlateBack2BytesAndCoordInfo;
    memcpy(&Infrared_RotationLEDData[2], &coord[4], 2);
    memcpy(&Infrared_RotationLEDData[4], distanceStr, 2);
    Infrared_Send_A(Infrared_RotationLEDData);
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ TFT显示屏 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/*TODO*******************************************************************************************************
 【函 数】:Get_TFT_Index
 【参 数】:index:'A' 'B' 'C'
 【返 回】:Get_TFT_Index('A')
 【简 例】:
 【说 明】:通过字符返回zigbeeID
 ************************************************************************************************************/
Zigbee_Header Get_TFT_Index(uint8_t index)
{
    Zigbee_Header requestID;
    switch (index)
    {
    case 'A':
        requestID = Zigbee_TFT_A;
        break;
    case 'B':
        requestID = Zigbee_TFT_B;
        break;
    case 'C':
        requestID = Zigbee_TFT_C;
        break;
    }
    return requestID;
}
/************************************************************************************************************
 【函 数】:TFT_ShowPicture
 【参 数】:TFTx:'A' 'B' 'C'     picNumber:第几张图片
 【返 回】:
 【简 例】:
 【说 明】:TFT显示编号图片 picNumber最多20张
 ************************************************************************************************************/
void TFT_ShowPicture(uint8_t TFTx, uint8_t picNumber)
{
    if (picNumber > 20 || picNumber < 1)
        return;

    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Picture;
    ZigBee_TFTData[Pack_SubCmd1] = 0x00;
    ZigBee_TFTData[Pack_SubCmd2] = picNumber;
    Send_ZigBeeData5Times(ZigBee_TFTData);
}
/************************************************************************************************************
 【函 数】:TFT_PicturePrevious
 【参 数】:TFTx:'A' 'B' 'C'
 【返 回】:
 【简 例】:
 【说 明】:TFT上一张图片
 ************************************************************************************************************/
void TFT_PicturePrevious(uint8_t TFTx)
{
    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Picture;
    ZigBee_TFTData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeDataNTimes(ZigBee_TFTData, 2, 200);
    //	Send_ZigBeeData(ZigBee_TFTData);
}
/************************************************************************************************************
 【函 数】:TFT_PictureNext
 【参 数】:TFTx:'A' 'B' 'C'
 【返 回】:
 【简 例】:
 【说 明】:TFT下一张图片
 ************************************************************************************************************/
void TFT_PictureNext(uint8_t TFTx)
{
    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Picture;
    ZigBee_TFTData[Pack_SubCmd1] = 0x02;
    Send_ZigBeeDataNTimes(ZigBee_TFTData, 2, 200);
    //	Send_ZigBeeData(ZigBee_TFTData);
}
/************************************************************************************************************
 【函 数】:TFT_PictureAuto
 【参 数】:TFTx:'A' 'B' 'C'
 【返 回】:
 【简 例】:
 【说 明】:TFT图片自动翻页
 ************************************************************************************************************/
void TFT_PictureAuto(uint8_t TFTx)
{
    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Picture;
    ZigBee_TFTData[Pack_SubCmd1] = 0x03;
    Send_ZigBeeData5Times(ZigBee_TFTData);
}
/************************************************************************************************************
 【函 数】:TFT_Plate
 【参 数】:TFTx:'A' 'B' 'C'
 【返 回】:
 【简 例】:
 【说 明】:TFT显示车牌
 ************************************************************************************************************/
void TFT_Plate(uint8_t TFTx, uint8_t plate[6])
{
    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_PlateDataA;
    memcpy(&ZigBee_TFTData[Pack_SubCmd1], plate, 3);
    Send_ZigBeeData5Times(ZigBee_TFTData);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_PlateDataB;
    memcpy(&ZigBee_TFTData[Pack_SubCmd1], &plate[3], 3);
    Send_ZigBeeData5Times(ZigBee_TFTData);
}
/************************************************************************************************************
 【函 数】:TFT_Timer
 【参 数】:TFTx:'A' 'B' 'C'
 【返 回】:
 【简 例】:
 【说 明】:TFT计时模式控制
 ************************************************************************************************************/
void TFT_Timer(uint8_t TFTx, TimerMode_t mode)
{
    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Timer;
    ZigBee_TFTData[Pack_SubCmd1] = (uint8_t)mode;
    Send_ZigBeeData5Times(ZigBee_TFTData);
}
/************************************************************************************************************
 【函 数】:TFT_HexData
 【参 数】:TFTx:'A' 'B' 'C'
 【返 回】:
 【简 例】:
 【说 明】:TFT六位数据显示模式（HEX）
 ************************************************************************************************************/
void TFT_HexData(uint8_t TFTx, uint8_t data[3])
{
    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Hex;
    memcpy(&ZigBee_TFTData[Pack_SubCmd1], data, 3);
    Send_ZigBeeData5Times(ZigBee_TFTData);
}
/************************************************************************************************************
 【函 数】:TFT_Distance
 【参 数】:TFTx:'A' 'B' 'C'
 【返 回】:
 【简 例】:
 【说 明】:TFT显示距离
 ************************************************************************************************************/
void TFT_Distance(uint8_t TFTx, uint16_t dis)
{
    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_Distance;
    ZigBee_TFTData[Pack_SubCmd2] = HEX2BCD(dis / 100);
    ZigBee_TFTData[Pack_SubCmd3] = HEX2BCD(dis % 100);
    Send_ZigBeeData5Times(ZigBee_TFTData);
}
/************************************************************************************************************
 【函 数】:TFT_TrafficSign
 【参 数】:TFTx:'A' 'B' 'C'
 【返 回】:
 【简 例】:
 【说 明】:TFT显示交通标志
 ************************************************************************************************************/
void TFT_TrafficSign(uint8_t TFTx, RouteTrafficSign_t TraSign)
{
    ZigBee_TFTData[Pack_Header2] = Get_TFT_Index(TFTx);
    ZigBee_TFTData[Pack_MainCmd] = TFTMode_TrafficSign;
    ZigBee_TFTData[Pack_SubCmd1] = TraSign;
    Send_ZigBeeData5Times(ZigBee_TFTData);
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 立体车库 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/*TODO*******************************************************************************************************
 【函 数】:Get_TFT_Index
 【参 数】:index:'A' 'B'
 【返 回】:Get_TFT_Index('A')
 【简 例】:
 【说 明】:通过字符返回zigbeeID
 ************************************************************************************************************/
Zigbee_Header Get_StereoGarage_Index(uint8_t index)
{
    Zigbee_Header requestID;
    switch (index)
    {
    case 'A':
        requestID = Zigbee_StereoGarage_A;
        break;
    case 'B':
        requestID = Zigbee_StereoGarage_B;
        break;
    }
    return requestID;
}
/************************************************************************************************************
 【函 数】:StereoGarage_ToLayer
 【参 数】:garage_x:'A' 'B'
 【返 回】:
 【简 例】:
 【说 明】:立体车库到达第X层
 ************************************************************************************************************/
void StereoGarage_ToLayer(uint8_t garage_x, uint8_t layer)
{
    if (layer > 4 || layer < 1)
        return;

    ZigBee_StereoGarageData[Pack_Header2] = Get_StereoGarage_Index(garage_x);
    ZigBee_StereoGarageData[Pack_MainCmd] = StereoGarage_Control;
    ZigBee_StereoGarageData[Pack_SubCmd1] = layer;
    Send_ZigBeeData5Times(ZigBee_StereoGarageData);
}
/************************************************************************************************************
 【函 数】:StereoGarage_ReturnLayer
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:立体车库返回位于第几层
 ************************************************************************************************************/
void StereoGarage_ReturnLayer(uint8_t garage_x)
{
    ZigBee_StereoGarageData[Pack_Header2] = Get_StereoGarage_Index(garage_x);
    ZigBee_StereoGarageData[Pack_MainCmd] = StereoGarage_Return;
    ZigBee_StereoGarageData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeData(ZigBee_StereoGarageData);
}
/************************************************************************************************************
 【函 数】:StereoGarage_ReturnInfraredStatus
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:立体车库返回前后红外信息
 ************************************************************************************************************/
void StereoGarage_ReturnInfraredStatus(uint8_t garage_x)
{
    ZigBee_StereoGarageData[Pack_Header2] = Get_StereoGarage_Index(garage_x);
    ZigBee_StereoGarageData[Pack_MainCmd] = StereoGarage_Return;
    ZigBee_StereoGarageData[Pack_SubCmd1] = 0x02;
    Send_ZigBeeData(ZigBee_StereoGarageData);
}
/************************************************************************************************************
 【函 数】:Wait_GarageToFristLayer
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:等待车库下降到第一层
 ************************************************************************************************************/
void Wait_GarageToFristLayer(uint8_t garage_x)
{
    uint8_t times = 7;
    while (Get_StereoGrageLayer(garage_x) != 1 && times > 0)
    {
        times--;
        StereoGarage_ToLayer(garage_x, 1);
        print_info("StereoGarage_A:%d\r\n", Get_StereoGrageLayer(garage_x));
        delay(2000);
    }
}
/************************************************************************************************************
 【函 数】:Wait_CarIntoGarage
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:等待车完全进入车库
 ************************************************************************************************************/
void Wait_CarIntoGarage(uint8_t StereoGarage_x, uint8_t targetLayer)
{
    for (int i = 0; i < 10; i++)
    {
        if (Get_StereoGrageInfraredStatus(StereoGarage_x)[1] == 0)
        {
            StereoGarage_ToLayer(StereoGarage_x, targetLayer);
            break;
        }
        else if (Get_StereoGrageInfraredStatus(StereoGarage_x)[1] == 1)
        {
            MOVE(-2);
        }
    }
}
/************************************************************************************************************
 【函 数】:Reverse_Parcking
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:自动判断方向倒车入库, 当前在车库入口时使用
 ************************************************************************************************************/
void Reverse_Parcking(RouteNode_t *current, uint8_t targetGarage[3], uint8_t StereoGarage_x)
{
    uint8_t *currentStr = ReCoordinate_Convert(*current);
    Direction_t dir = Get_Towards(targetGarage, currentStr);

    // warning:码盘转向
    TURN_TO(dir);

    Start_Turn_Check();
    Track_ByEncoder(Track_Speed, 25); // 根据码盘值前进25cm

    delay(1500);

    Move_ByEncoder(30, -80);

    // bool garage_front_flag = false;
    // bool garage_back_flag = false;

    // while (!(garage_front_flag && garage_back_flag))
    // {
    //     if (!garage_front_flag && !Get_StereoGrageInfraredStatus(StereoGarage_x)[0])
    //         garage_front_flag = true;
    //     if (!garage_back_flag && !Get_StereoGrageInfraredStatus(StereoGarage_x)[1])
    //         garage_back_flag = true;
    // }

    MOVE(5);
    Stop();
}
/************************************************************************************************************
 【函 数】:Get_StereoGrageLayer
 【参 数】:
 【返 回】:0为错误
 【简 例】:
 【说 明】:获取立体车库当前层数
 ************************************************************************************************************/
uint8_t Get_StereoGrageLayer(uint8_t garage_x)
{
    ZigBee_DataStatus_t *StereoGarage_Status;
    StereoGarage_Status = (garage_x == 'A') ? (&StereoGarage_A_Status) : (&StereoGarage_B_Status);

    StereoGarage_Status->isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        StereoGarage_ReturnLayer(garage_x);
        WaitForFlagInMs(StereoGarage_Status->isSet, SET, 300);
        if (StereoGarage_Status->isSet == SET)
        {
            if (StereoGarage_Status->cmd[Pack_SubCmd1] == 0x01)
                return StereoGarage_Status->cmd[Pack_SubCmd2];
            else
                return 0;
        }
    }
    return 0;
}
/************************************************************************************************************
 【函 数】:Get_StereoGrageInfraredStatus
 【参 数】:[0] 前侧 [1] 后侧
 【返 回】:0未触发 1触发（无障碍时触发）
 【简 例】:
 【说 明】:获取立体车库前后红外状态
 ************************************************************************************************************/
uint8_t *Get_StereoGrageInfraredStatus(uint8_t garage_x)
{
    static uint8_t IRStatus[2];

    ZigBee_DataStatus_t *StereoGarage_Status;
    StereoGarage_Status = (garage_x == StereoGarage_A) ? (&StereoGarage_A_Status) : (&StereoGarage_B_Status);

    StereoGarage_Status->isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        StereoGarage_ReturnInfraredStatus(garage_x);
        WaitForFlagInMs(StereoGarage_Status->isSet, SET, 300);

        if (StereoGarage_Status->isSet == SET)
        {
            if (StereoGarage_Status->cmd[Pack_SubCmd1] == 0x02)
                memcpy(IRStatus, &StereoGarage_Status->cmd[Pack_SubCmd2], 2);
            else
                memset(IRStatus, 0, sizeof(IRStatus));
            break;
        }
    }
    for (uint8_t i = 0; i < 2; i++)
    {
        if (IRStatus[i] == 0x02)
            IRStatus[i] = 0;
    }
    return IRStatus;
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 交通灯 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/*TODO*******************************************************************************************************
 【函 数】:Get_TrafficLight_Index
 【参 数】:index:'A' 'B' 'C' 'D'
 【返 回】:Get_TrafficLight_Index('A')
 【简 例】:
 【说 明】:通过字符返回zigbeeID
 ************************************************************************************************************/
Zigbee_Header Get_TrafficLight_Index(uint8_t index)
{
    Zigbee_Header requestID;
    switch (index)
    {
    case 'A':
        requestID = Zigbee_TrafficLight_A;
        break;
    case 'B':
        requestID = Zigbee_TrafficLight_B;
        break;
    case 'C':
        requestID = Zigbee_TrafficLight_C;
        break;
    case 'D':
        requestID = Zigbee_TrafficLight_D;
        break;
    }
    return requestID;
}
/************************************************************************************************************
 【函 数】:TrafficLight_RecognitionMode
 【参 数】:light_X: 'A' 'B' 'C' 'D'
 【返 回】:
 【简 例】:
 【说 明】:交通灯进入识别状态
 ************************************************************************************************************/
void TrafficLight_RecognitionMode(uint8_t light_x)
{
    ZigBee_TrafficLightData[Pack_Header2] = Get_TrafficLight_Index(light_x);
    ZigBee_TrafficLightData[Pack_MainCmd] = TrafficLight_Recognition;
    ZigBee_TrafficLightData[Pack_SubCmd1] = 0x00;
    Send_ZigBeeDataNTimes(ZigBee_TrafficLightData, 2, 150);
}
/************************************************************************************************************
 【函 数】:TrafficLight_ConfirmColor
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:交通灯确认识别结果
 ************************************************************************************************************/
void TrafficLight_ConfirmColor(uint8_t light_x, TrafficLightColor_t light)
{
    uint8_t requestID;
    switch (light_x)
    {
    case 'A':
        requestID = Zigbee_TrafficLight_A;
        break;
    case 'B':
        requestID = Zigbee_TrafficLight_B;
        break;
    case 'C':
        requestID = Zigbee_TrafficLight_C;
        break;
    case 'D':
        requestID = Zigbee_TrafficLight_D;
        break;
    }
    ZigBee_TrafficLightData[Pack_Header2] = requestID;
    ZigBee_TrafficLightData[Pack_MainCmd] = TrafficLight_Confirm;
    ZigBee_TrafficLightData[Pack_SubCmd1] = (uint8_t)light;
    Send_ZigBeeData5Times(ZigBee_TrafficLightData);
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 语音识别 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:VoiceBroadcast_Specific
 【参 数】:voiceID:语音编号
 【返 回】:
 【简 例】:
 【说 明】:语音播报特定编号语音
 ************************************************************************************************************/
void VoiceBroadcast_Specific(uint8_t voiceID)
{
    if (voiceID > 6 || voiceID < 1)
        return;

    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_Specific;
    ZigBee_VoiceData[Pack_SubCmd1] = voiceID;
    Send_ZigBeeDataNTimes(ZigBee_VoiceData, 2, 100);
}
/************************************************************************************************************
 【函 数】:VoiceBroadcast_Radom
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:语音随机播报语音
 ************************************************************************************************************/
void VoiceBroadcast_Radom(void)
{
    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_Random;
    ZigBee_VoiceData[Pack_SubCmd1] = 0x01;
    Send_ZigbeeData_To_Fifo(ZigBee_VoiceData, 8);
}
/************************************************************************************************************
 【函 数】:VoiceRTC_StartData
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:设置RTC（实时时钟）起始日期
 ************************************************************************************************************/
void VoiceRTC_StartData(uint8_t year, uint8_t month, uint8_t day)
{
    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_SetStartData;
    ZigBee_VoiceData[Pack_SubCmd1] = year;
    ZigBee_VoiceData[Pack_SubCmd2] = month;
    ZigBee_VoiceData[Pack_SubCmd3] = day;
    Send_ZigBeeDataNTimes(ZigBee_VoiceData, 2, 100);
}
/************************************************************************************************************
 【函 数】:VoiceRTC_CurrentData
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:查询RTC当前日期
 ************************************************************************************************************/
void VoiceRTC_CurrentData(void)
{
    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_QueryCurrentData;
    ZigBee_VoiceData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeDataNTimes(ZigBee_VoiceData, 2, 100);
}
/************************************************************************************************************
 【函 数】:VoiceRTC_StartTime
 【参 数】:hour:小时    minute:分   second:秒
 【返 回】:
 【简 例】:
 【说 明】:设置RTC起始时间
 ************************************************************************************************************/
void VoiceRTC_StartTime(uint8_t hour, uint8_t minute, uint8_t second)
{
    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_SetStartTime;
    ZigBee_VoiceData[Pack_SubCmd1] = hour;
    ZigBee_VoiceData[Pack_SubCmd2] = minute;
    ZigBee_VoiceData[Pack_SubCmd3] = second;
    Send_ZigBeeDataNTimes(ZigBee_VoiceData, 2, 100);
}
/************************************************************************************************************
 【函 数】:VoiceRTC_CurrentTime
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:查询RTC当前时间
 ************************************************************************************************************/
void VoiceRTC_CurrentTime(void)
{
    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_QueryCurrentTime;
    ZigBee_VoiceData[Pack_SubCmd1] = 0x01;
    Send_ZigBeeDataNTimes(ZigBee_VoiceData, 2, 100);
}
/************************************************************************************************************
 【函 数】:VoiceSet_WeaTem
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:设置天气数据与温度数据 温度：输入为十六进制,显示为十进制
 ************************************************************************************************************/
void VoiceSet_WeaTem(Weather_t weather, uint8_t temperature)
{
    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_SetWeaTem;
    ZigBee_VoiceData[Pack_SubCmd1] = weather;
    ZigBee_VoiceData[Pack_SubCmd2] = temperature;
    Send_ZigBeeDataNTimes(ZigBee_VoiceData, 2, 100);
}
/************************************************************************************************************
 【函 数】:VoiceReturn_WeaTem
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:请求回传天气数据与温度数据
 ************************************************************************************************************/
void VoiceReturn_WeaTem(void)
{
    ZigBee_VoiceData[Pack_MainCmd] = VoiceCmd_RequestWeaTemData;
    Send_ZigBeeDataNTimes(ZigBee_VoiceData, 2, 100);
}
/************************************************************************************************************
 【函 数】:VoiceRecognition_Return
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:返回语音识别结果到自动评分系统
 ************************************************************************************************************/
void VoiceRecognition_Return(uint8_t voiceID)
{
    print_info("voiceID:%d", voiceID);
    ZigBee_VoiceReturnData[2] = voiceID;
    print_info("voice zigbee:%d", ZigBee_VoiceReturnData);
    for (uint8_t i = 0; i < 3; i++)
    {
        Send_ZigbeeData_To_Fifo(ZigBee_VoiceReturnData, 8);
        delay_ms(100);
    }
    // 语音返回有固定码,不能校验
}
/*TODO*******************************************************************************************************
 【函 数】:Voice_Recognition
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:语音识别任务
 ************************************************************************************************************/
void Voice_Recognition(void)
{
    VoiceRecognition_Return(Start_VoiceCommandRecognition(5)); // 3指语音播报标志物随机播报次数,可更改
}
/************************************************************************************************************
 【函 数】:VoiceRuturnTime
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取语音识别标志物返回的时间
 ************************************************************************************************************/
uint8_t *VoiceRuturnTime(void)
{
    static uint8_t RuturnTime[3];

    ZigBee_DataStatus_t *Voice_Status;

    Voice_Status = &VoiceBroadcast_Status;

    Voice_Status->isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        VoiceRTC_CurrentTime();
        WaitForFlagInMs(Voice_Status->isSet, SET, 300);

        if (Voice_Status->isSet == SET)
        {
            if (Voice_Status->cmd[Pack_MainCmd] == 0x03)
                memcpy(RuturnTime, &Voice_Status->cmd[Pack_SubCmd1], 3);
            else
                memset(RuturnTime, 0, sizeof(RuturnTime));
        }
    }
    return RuturnTime;
}
/************************************************************************************************************
 【函 数】:VoiceRuturnData
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取语音识别标志物返回的日期
 ************************************************************************************************************/
uint8_t *VoiceRuturnData(void)
{
    static uint8_t RuturnData[3];

    ZigBee_DataStatus_t *Voice_Status;

    Voice_Status = &VoiceBroadcast_Status;

    Voice_Status->isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        VoiceRTC_CurrentData();
        WaitForFlagInMs(Voice_Status->isSet, SET, 300);

        if (Voice_Status->isSet == SET)
        {
            if (Voice_Status->cmd[Pack_MainCmd] == 0x02)
                memcpy(RuturnData, &Voice_Status->cmd[Pack_SubCmd1], 3);
            else
                memset(RuturnData, 0, sizeof(RuturnData));
        }
    }
    return RuturnData;
}
/************************************************************************************************************
 【函 数】:VoiceRuturnWea
 【参 数】:
 【返 回】:返回的 [天气,温度]
 【简 例】:
 【说 明】:获取语音识别标志物
 ************************************************************************************************************/
uint8_t *VoiceRuturnWea(void)
{
    static uint8_t RuturnWea[2];

    ZigBee_DataStatus_t *Voice_Status;

    Voice_Status = &VoiceBroadcast_Status;

    Voice_Status->isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        VoiceReturn_WeaTem();
        WaitForFlagInMs(Voice_Status->isSet, SET, 300);

        if (Voice_Status->isSet == SET)
        {
            if (Voice_Status->cmd[Pack_MainCmd] == 0x04)
                memcpy(RuturnWea, &Voice_Status->cmd[Pack_SubCmd1], 3);
            else
                memset(RuturnWea, 0, sizeof(RuturnWea));
        }
    }
    return RuturnWea;
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 智能路灯 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:StreetLight_AdReset
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:路灯挡位复位为最初始挡位(一档)
 ************************************************************************************************************/
void StreetLight_AdReset(void)
{
    uint8_t temp_val[4];

    for (int8_t i = 0; i < 4; i++)
    {
        temp_val[i] = BH1750_GetAverage(10);
        Infrared_Send_A(Infrared_LightAdd1);
        Beep(2);
        delay(1000);
    }

    uint8_t reset_error = BackMINSubscript_2(temp_val, GET_ARRAY_LENGEH(temp_val));
    switch (reset_error)
    {
    case 0:
        break;
    case 1:
        Infrared_Send_A(Infrared_LightAdd1);
        break;
    case 2:
        Infrared_Send_A(Infrared_LightAdd2);
        break;
    default:
        Infrared_Send_A(Infrared_LightAdd3);
        break;
    } // 恢复默认挡位
    delay(1000);
}
/************************************************************************************************************
 【函 数】:StreetLight_AdjustTotargetLevel
 【参 数】:targetLevel:目标挡位
 【返 回】:
 【简 例】:
 【说 明】:复位调节灯至目标挡位返回调节后挡位; 路灯调节至目标挡位后又复位到初始挡位,返回目标挡位
 ************************************************************************************************************/
uint8_t StreetLight_AdjustTotargetLevel(uint8_t targetLevel)
{
    StreetLight_AdReset();

    if ((targetLevel > 0) && (targetLevel < 5))
    {
        switch (targetLevel - 1)
        {
        case 0:
            break;
        case 1:
            Infrared_Send_A(Infrared_LightAdd1);
            break;
        case 2:
            Infrared_Send_A(Infrared_LightAdd2);
            break;
        default:
            Infrared_Send_A(Infrared_LightAdd3);
            break;
        } // 恢复默认挡位
        return targetLevel;
    }
    else
    {
        Infrared_Send_A(Infrared_LightAdd2);
        return 2;
    }
}
/************************************************************************************************************
 【函 数】:StreetLight_Now
 【参 数】:
 【返 回】:返回初始挡位
 【简 例】:
 【说 明】:路灯挡位不调节
 ************************************************************************************************************/
uint8_t StreetLight_Now(void)
{
    uint16_t temp_val[4];

    for (int8_t i = 0; i < 4; i++)
    {
        temp_val[i] = BH1750_GetAverage(10);
        Infrared_Send_A(Infrared_LightAdd1);
        Beep(2);
        delay(1000);
    }

    uint16_t currentBrightness = temp_val[0];
    uint8_t currentLevel;

    // 对获得数据排序可算出当前档位
    bubble_sort(temp_val, 4);

    for (int8_t i = 0; i < 4; i++)
    {
        if (currentBrightness == temp_val[i])
        {
            currentLevel = i + 1;
            break;
        }
    }
    return currentLevel;
}
/************************************************************************************************************
 【函 数】:StreetLight_AdjustTo
 【参 数】:targetLevel:目标挡位
 【返 回】:返回初始档位; 0为不调节档位
 【简 例】:
 【说 明】:路灯档位调节
 ************************************************************************************************************/
uint8_t StreetLight_AdjustTo(uint8_t targetLevel)
{
    uint16_t temp_val[4];

    for (int8_t i = 0; i < 4; i++)
    {
        temp_val[i] = BH1750_GetAverage(10);
        Infrared_Send_A(Infrared_LightAdd1);
        Beep(2);
        delay(1000);
    }

    uint16_t currentBrightness = temp_val[0];
    uint8_t currentLevel;

    // 对获得数据排序可算出当前档位
    bubble_sort(temp_val, 4);

    for (int8_t i = 0; i < 4; i++)
    {
        if (currentBrightness == temp_val[i])
        {
            currentLevel = i + 1;
            break;
        }
    }

    // 目标档位不合法时不调节
    if ((targetLevel > 0) && (targetLevel < 5))
    {
        int8_t error = targetLevel - currentLevel;
        int errorArray[] = {error, error - 4, error + 4};

        int minimumError = MinimumAbsOf(errorArray, GET_ARRAY_LENGEH(errorArray));
        int length = abs(minimumError);

        for (int i = 0; i < length; i++)
        {
            Infrared_Send_A((minimumError >= 0) ? Infrared_LightAdd1 : Infrared_LightAdd3);
            delay(1000);
        }
    }
    return currentLevel;
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 静态标志物 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/*TODO*******************************************************************************************************
 【函 数】:Get_StaticMarker_Index
 【参 数】:index:'A' 'B'
 【返 回】:Get_StaticMarker_Index('A')
 【简 例】:
 【说 明】:通过字符返回zigbeeID
 ************************************************************************************************************/
Zigbee_Header Get_StaticMarker_Index(uint8_t index)
{
    Zigbee_Header requestID;
    switch (index)
    {
    case 'A':
        requestID = Zigbee_StaticMarker_A;
        break;
    case 'B':
        requestID = Zigbee_StaticMarker_B;
        break;
    }
    return requestID;
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 任务版 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:Beep
 【参 数】:Ntimes:次数
 【返 回】:
 【简 例】:
 【说 明】:蜂鸣器
 ************************************************************************************************************/
void Beep(uint8_t Ntimes)
{
    for (uint8_t i = 0; i < Ntimes; i++)
    {
        MP_SPK = 1;
        delay_ms(70);
        MP_SPK = 0;
        delay_ms(30);
    }
}
/************************************************************************************************************
 【函 数】:BEEP_Triple
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:蜂鸣三次,不要问我为什么搞一个函数,学长留的!
 ************************************************************************************************************/
void BEEP_Triple(void)
{
    Beep(3);
}
/************************************************************************************************************
 【函 数】:Emergency_Flasher
 【参 数】:time:延迟
 【返 回】:
 【简 例】:
 【说 明】:双闪灯闪烁
 ************************************************************************************************************/
void Emergency_Flasher(uint16_t time)
{
    Set_tba_WheelLED(L_LED, SET);
    Set_tba_WheelLED(R_LED, SET);
    Can_ZigBeeRx_Check();
    if (time > 0)
        delay(time);

    Set_tba_WheelLED(L_LED, RESET);
    Set_tba_WheelLED(R_LED, RESET);
}
/************************************************************************************************************
 【函 数】:DistanceMeasure_Task
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:测距任务
 ************************************************************************************************************/
uint16_t DistanceMeasure_Task(void)
{
    uint16_t distance = Ultrasonic_GetAverage(20);
    delay(2000);
    return distance;
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 从车 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:AGV_Task
 【参 数】:agvData:从车信息集
 【返 回】:
 【简 例】:
 【说 明】:AGV启动
 ************************************************************************************************************/
void AGV_Task(DataToAGV_t agvData)
{
    uint8_t agvRoute[120];

    // 坐标、起始点、坐标处理
    RouteString_Process(agvData, agvRoute);
    print_info("AGV_Route:%s\r\n", agvRoute);
    AGV_SetRoute(agvRoute);
    AGV_Send_None(AGV_CMD_Start);
    print_info("AGV_Start\r\n");
}
/************************************************************************************************************
 【函 数】:AGV_Task_With_Msg
 【参 数】:agvData:从车信息集
 【返 回】:
 【简 例】:
 【说 明】:AGV任务携带其他数据
 ************************************************************************************************************/
void AGV_Task_With_Msg(DataToAGV_t agvData)
{
    uint8_t agvRoute[20];

    // 坐标、起始点、坐标处理
    RouteString_Process(agvData, agvRoute);
    print_info("AGV_Route:%s\r\n", agvRoute);
    AGV_SetRoute(agvRoute);

    // 方向设定
    if (agvData.direction != DIR_NOTSET)
    {
        print_info("AGV_Dir:%d\r\n", agvData.direction);
        AGV_Send_Single(AGV_CMD_Towards, agvData.direction);
    }
    // 报警码
    if (agvData.alarmData != NULL)
    {
        Dump_Array("AGV_Alarm:\r\n", agvData.alarmData, 6, 1);
        AGV_Send_Multi(AGV_CMD_AlarmFront, &agvData.alarmData[0], 3);
        delay_ms(200);
        AGV_Send_Multi(AGV_CMD_AlarmBack, &agvData.alarmData[3], 3);
    }
    // 车牌信息
    if (agvData.carnum != NULL)
    {
        print_info("%s\r\n", agvData.carnum);
        AGV_Send_Multi(AGV_CMD_PlateFront, &agvData.carnum[0], 3);
        delay_ms(200);
        AGV_Send_Multi(AGV_CMD_PlateBack, &agvData.carnum[3], 3);
    }
    // 路灯档位
    if (agvData.streetLightLevel > 0 && agvData.streetLightLevel <= 4)
    {
        print_info("LightLevel:%d\r\n", agvData.streetLightLevel);
        AGV_Send_Single(AGV_CMD_StreetLight, agvData.streetLightLevel);
    }
    // 交通信号
    if (agvData.trafficSign != NULL)
    {
        print_info("trafficSign:d\r\n", agvData.trafficSign);
        AGV_Send_Single(AGV_CMD_TrafficSign, agvData.trafficSign);
    }
    delay(2000);
    // 启动
    AGV_Send_None(AGV_CMD_Start);
    print_info("AGV_Start");
}
/************************************************************************************************************
 【函 数】:RouteString_Process
 【参 数】:agvData:从车信息集,buffer返回的路径
 【返 回】:
 【简 例】:
 【说 明】:处理含有路径信息的字符串，去除无效信息
 ************************************************************************************************************/
void RouteString_Process(DataToAGV_t agvData, uint8_t *buffer)
{
    uint16_t startPointLen = strlen((char *)agvData.startPoint);
    uint16_t beforeRouteLen = strlen((char *)agvData.beforeRouteInfo);
    uint16_t RouteLen = strlen((char *)agvData.routeInfo);
    uint16_t afterRouteLen = strlen((char *)agvData.afterRouteInfo);
    uint16_t endPointLen = strlen((char *)agvData.endPoint);

    uint8_t *tempbuffer = (uint8_t *)malloc(sizeof(uint8_t) * (startPointLen + beforeRouteLen + RouteLen + afterRouteLen + endPointLen + 1));

    if (tempbuffer == NULL)
        return;

    // 拷贝前缀和原始信息
    memcpy(tempbuffer, agvData.startPoint, startPointLen);
    memcpy(tempbuffer + startPointLen, agvData.beforeRouteInfo, beforeRouteLen);
    memcpy(tempbuffer + startPointLen + beforeRouteLen, agvData.routeInfo, RouteLen);
    memcpy(tempbuffer + startPointLen + beforeRouteLen + RouteLen, agvData.afterRouteInfo, afterRouteLen);
    memcpy(tempbuffer + startPointLen + beforeRouteLen + RouteLen + afterRouteLen, agvData.endPoint, endPointLen);

    uint8_t *pstr = buffer;

    // 查找符合条件的字符串并拷贝到buffer中
    for (uint16_t i = 0; i < (startPointLen + beforeRouteLen + RouteLen + afterRouteLen + endPointLen); i++)
    {
        if ((tempbuffer[i] >= 'A') && (tempbuffer[i] <= 'G'))
        {
            if ((tempbuffer[i + 1] >= '1') && (tempbuffer[i + 1] <= '7'))
            {
                pstr[0] = tempbuffer[i];
                pstr[1] = tempbuffer[i + 1];
                pstr += 2;

                // 匹配则跳过下一个字符
                i++;
            }
        }
    }
    pstr[0] = '\0';

    free(tempbuffer);

    return;
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 通用 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:Start_Task
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:通用起始任务
 ************************************************************************************************************/
void Start_Task(void)
{
    Start_Turn_Check();
    Emergency_Flasher(1500);
    BEEP_Triple();
    LEDDisplay_TimerMode(TimerMode_ON);
    //	TFT_HexData(TFT_A,dat);
}
/************************************************************************************************************
 【函 数】:End_Task
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:通用终止任务
 ************************************************************************************************************/
void End_Task(void)
{
    LEDDisplay_TimerMode(TimerMode_OFF);
    Emergency_Flasher(5000);
}
