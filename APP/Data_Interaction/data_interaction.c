#include "data_interaction.h"
#include "canP_HostCom.h"
#include "delay.h"
#include "roadway_check.h"
#include "can_user.h"
#include "tba.h"
#include "cba.h"
#include "infrared.h"
#include <string.h>
#include "Timer.h"
#include "agv.h"
#include "debug.h"
#include "independent_task.h"
#include "ultrasonic.h"

// 与ZigBee设备的数据交互↓
extern uint8_t fht[6];              // 烽火台报警
extern uint8_t AGV_platenumber[15]; // 从车车牌
extern uint16_t AGV_dis;
extern uint8_t car_flag;            // 临时避让标志位
extern uint8_t AGV_QRCodeData1[32]; // 从车二维码1
extern uint8_t AGV_QRCodeData2[32]; // 从车二维码2

// 除从车之外其它设备的消息长度都是8位,使用ZigBee_DataStatus_t数据类型

typedef struct AGVUploadData_Struct
{
    uint8_t isSet;
    uint16_t data;
} AGVUploadData_t;

// AGV数据定义
AGVUploadData_t AGV_Ultrasonic = {.isSet = RESET, .data = 0};
AGVUploadData_t AGV_Brightness = {.isSet = RESET, .data = 0};
AGVUploadData_t AGV_Alar = {.isSet = RESET, .data = 0};
uint8_t AGV_QRCodeCount;
uint8_t AGV_platenumLength; // 从车车牌
uint8_t light_Level;
uint8_t adjust_Level;
uint8_t AGV_QRCodeLength = 0;
uint8_t AGV_QRCodeIsReceived = false;
uint8_t AGV_Bfloor1; // 从车发的B车库初始层数
uint8_t AGV_Afloor1; // 从车发的A车库初始层数
bool AGV_MissionComplete = false;

// 返回ID对应的Buffer
#define ReturnBuffer(requestID) return DataBuffer[requestID].buffer
// 自动填充buf长度（buf不能为指针！）
#define ResetRquestWait(requestID, buf) Reset_Rquest_Wait(requestID, buf, sizeof(buf))

// 标志物数据声明、定义和处理操作的结构一样所以使用宏定义简化操作

// 定义变量
#define DefineDataStatus(name) ZigBee_DataStatus_t name##_Status = {0, {0}, 0}

// 处理ZigBee返回数据
#define ProcessZigBeeReturnData(X)   \
    X##_Status.isSet = SET;          \
    X##_Status.timeStamp = millis(); \
    memcpy(X##_Status.cmd, cmd, 8)

// 标志物ZigBee数据处理
#define CaseProcess(name)                \
    case Zigbee_##name:                  \
        ProcessZigBeeReturnData(##name); \
        break;

DefineDataStatus(SpecialRoad);
DefineDataStatus(BarrierGate);
DefineDataStatus(ETC);
DefineDataStatus(TrafficLight_A);
DefineDataStatus(TrafficLight_B);
DefineDataStatus(StereoGarage_A);
DefineDataStatus(StereoGarage_B);
DefineDataStatus(AGV);
DefineDataStatus(VoiceBroadcast);

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 处理zigbee ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:ZigBee_CmdHandler
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:ZigBee指令、数据处理
 ************************************************************************************************************/
void ZigBee_CmdHandler(uint8_t *cmd)
{
    uint8_t discan[5];
    switch (cmd[1])
    {
        CaseProcess(ETC);
        CaseProcess(BarrierGate);
        CaseProcess(TrafficLight_A);
        CaseProcess(TrafficLight_B);
        CaseProcess(StereoGarage_A);
        CaseProcess(StereoGarage_B);
        CaseProcess(VoiceBroadcast);
        CaseProcess(SpecialRoad);
    case ZigBee_AGV:
        AGV_Status.isSet = SET;
        AGV_Status.timeStamp = millis();
        switch (cmd[AGVUploadData_DataType])
        {
        case AGVUploadType_Ultrasonic: // 从车超声波
            memcpy(discan, &cmd[3], 3);
            AGV_dis = discan[0] * 100 + discan[1] * 10 + discan[2];
            break;
        case AGVUploadType_Alarm1: // 烽火台
            AGV_Alar.isSet = SET;
            memcpy(fht, &cmd[3], 6);
            break;
        case AGVUploadType_carflag: // 临时停车点
            car_flag = 1;
            break;
        case AGVUploadType_Brightness: // 从车光照度
            light_Level = cmd[3];
            adjust_Level = cmd[4];
            break;
        case AGVUploadType_MisonComplete: // 从车任务完成
            AGV_MissionComplete = true;
            break;
        case AGVUploadType_platenum:
            AGV_platenumLength = 6;
            memcpy(AGV_platenumber, &cmd[3], AGV_platenumLength);
            break;
        case AGV_Bfloor:
            AGV_Bfloor1 = cmd[3];
            break;
        case AGV_Afloor:
            AGV_Afloor1 = cmd[3];
            break;
        case AGVUploadType_QRCodeData: // 从车二维码数据
            AGV_QRCodeIsReceived = true;
            AGV_QRCodeLength = cmd[3];
            // 从车二维码组收集处理
            if (AGV_QRCodeCount == 0)
                memcpy(AGV_QRCodeData1, &cmd[4], AGV_QRCodeLength);
            if (AGV_QRCodeCount == 1)
                memcpy(AGV_QRCodeData2, &cmd[4], AGV_QRCodeLength);
            Send_QRCodeData(AGV_QRCodeData1, AGV_QRCodeLength);
            if (AGV_QRCodeCount < 2)
                AGV_QRCodeCount++;
            else
                AGV_QRCodeCount = 0;
            break;
        }
        break;
    }
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 交互函数 ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:RequestToHost_Task
 【参 数】:request Zigbee_Header类型
 【返 回】:
 【简 例】:RequestToHost_Task(Zigbee_TrafficLight_A); 识别交通灯任务
 【说 明】:向上位机请求任务
 ************************************************************************************************************/
void RequestToHost_Task(uint8_t request)
{
    uint8_t requestTaskArray[] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};
    requestTaskArray[Pack_MainCmd] = request;
    Send_ToHost(requestTaskArray, 8);
}
/************************************************************************************************************
 【函 数】:RequestToHost_WithData_Task
 【参 数】:Zigbee_Header类型
 【返 回】:
 【简 例】:RequestToHost_WithData_Task()
 【说 明】:向上位机请求识别任务,并发送数据
 ************************************************************************************************************/
void RequestToHost_WithData_Task(Zigbee_Header request, TFT_Task_t task)
{
    uint8_t requestTaskArray[] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};
    requestTaskArray[Pack_MainCmd] = request;
    requestTaskArray[Pack_SubCmd1] = task;
    Send_ToHost(requestTaskArray, 8);
}
/************************************************************************************************************
 【函 数】:Process_CommandFromHost
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:处理上位机的指令,与官方主指令对应
 ************************************************************************************************************/
void Process_CommandFromHost(uint8_t mainCmd)
{
    SetCmdFlag(mainCmd);
}
/************************************************************************************************************
 【函 数】:HostData_Handler
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:处理上位机返回的数据 自定义的数据接收,长度在DataBuffer结构体中定义
 ************************************************************************************************************/
void HostData_Handler(uint8_t *buf)
{
    uint8_t requestID = buf[Data_ID];

    if (requestID > 0 && requestID <= DATA_REQUEST_NUMBER) // 确认命令是否在设定范围
    {
        // 结构体数组 DataBuffer 中取出ID对应的指针,从ID号之后开始,拷贝相应的ID字节数
        memcpy(DataBuffer[requestID].buffer, &buf[Data_Length], DataBuffer[requestID].Data_Length);
        // 标记已接收
        DataBuffer[requestID].isSet = SET;
    }
}
/************************************************************************************************************
 【函 数】:HostData_Request
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:向上位机请求识别结果 Send_ToHost = mode ? Send_WifiData_To_Fifo : Send_DataToUsart;
 ************************************************************************************************************/
void HostData_Request(uint8_t requestID, uint8_t *param, uint8_t paramLen)
{
    // 数据请求头
    static uint8_t dataRequestHeader[3] = {0x56, 0x66, 0x00};
    dataRequestHeader[Data_ID] = requestID;
    Send_ToHost(dataRequestHeader, 3);
    Send_ToHost(param, paramLen);
}
/************************************************************************************************************
 【函 数】:Reset_Rquest_Wait
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:置位后请求并等待
 ************************************************************************************************************/
void Reset_Rquest_Wait(uint8_t requestID, uint8_t *buf, uint8_t buflen)
{
    // 清空标志位
    DataBuffer[requestID].isSet = RESET;

    for (uint8_t i = 0; i < 3; i++)
    {
        // 发送请求
        HostData_Request(requestID, buf, buflen);
        WaitForFlagInMs(DataBuffer[requestID].isSet, SET, 10000);

        // 判断返回状态
        if (DataBuffer[requestID].isSet == SET)
            break;
    }
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 发送数据post ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:Send_QRCodeData
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:发送二维码数据
 ************************************************************************************************************/
void Send_QRCodeData(uint8_t *QRData, uint8_t length)
{
    uint8_t dataSendHeader[6] = {0x56, 0x76, 0x00, 0x00};
    dataSendHeader[Data_ID] = DataSend_QRCode;
    dataSendHeader[Data_Length] = length;
    Send_ToHost(dataSendHeader, 4);
    Send_ToHost(QRData, length);
}
/************************************************************************************************************
 【函 数】:Send_Calculator
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:发送公式和变量
 ************************************************************************************************************/
void Send_Calculator(Datasend_t data, uint8_t *QRData, uint8_t length)
{
    uint8_t dataSendHeader[6] = {0x56, 0x76, 0x00, 0x00};
    dataSendHeader[Data_ID] = data;
    dataSendHeader[Data_Length] = length;
    Send_ToHost(dataSendHeader, 4);
    Send_ToHost(QRData, length);
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 发送请求get ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:TrafficLight_Task
 【参 数】:index: 'A' 'B' 'C' 'D'
 【返 回】:
 【简 例】:
 【说 明】:交通灯识别
 ************************************************************************************************************/
void TrafficLight_Task(uint8_t index)
{
    ResetCmdFlag(FromHost_TrafficLight);
    TrafficLight_RecognitionMode(index);                               // 交通灯变色
    delay_ms(2000);                                                    // 延迟
    RequestToHost_Task(Get_TrafficLight_Index(index));                 // 识别交通灯
    WaitForFlagInMs(GetCmdFlag(FromHost_TrafficLight), SET, 9 * 1000); // 等待识别完成
    Beep(2);
}
/************************************************************************************************************
 【函 数】:TFT_Task
 【参 数】:index:'A' 'B' 'C'    task:TFT_Task_t中的任务     second:等待时间
 【返 回】:
 【简 例】:TFT_Task('A', TFT_Task_License, 5);  对TFT_A做车牌识别任务并等待5秒
 【说 明】:TFT图形图像识别
 ************************************************************************************************************/
void TFT_Task(Zigbee_Header TFTx, TFT_Task_t task, uint8_t second)
{
    ResetCmdFlag(FromHost_TFTRecognition);
    uint16_t TFTdis;
    TFTdis = Ultrasonic_GetAverage(5);
    if (TFTdis > 100)
        MOVE((TFTdis - 100) / 10);                                        // 移动至对应位置
    RequestToHost_WithData_Task(TFTx, task);                              // 请求识别TFT内容
    WaitForFlagInMs(GetCmdFlag(FromHost_TFTRecognition), SET, 10 * 1000); // 等待识别完成
    Beep(2);
    delay(second * 1000);
    MOVE(-((TFTdis - 100) / 10));
}
/************************************************************************************************************
 【函 数】:StaticMarker_Task
 【参 数】:index:'A' 'B'
 【返 回】:
 【简 例】:StaticMarker_Task('A')
 【说 明】:静态标志物识别
 ************************************************************************************************************/
void StaticMarker_Task(uint8_t index)
{
    ResetCmdFlag(FromHost_QRCodeRecognition);
    uint16_t Staticdis;
    Staticdis = Ultrasonic_GetAverage(5);
    if (Staticdis > 150)
        MOVE((Staticdis - 150) / 10);
    RequestToHost_Task(Get_StaticMarker_Index(index));
    WaitForFlagInMs(GetCmdFlag(FromHost_QRCodeRecognition), SET, 15 * 1000);
    Beep(2);
    MOVE(-((Staticdis - 150) / 10));
}
/************************************************************************************************************
 【函 数】:Any_Task
 【参 数】:index:Task_1
 【返 回】:
 【简 例】:Any_Task(Task_1)
 【说 明】:自定义任务
 ************************************************************************************************************/
void Any_Task(FromHost_t index)
{
    ResetCmdFlag(FromHost_Completed);
    RequestToHost_Task(index);
    WaitForFlagInMs(GetCmdFlag(FromHost_Completed), SET, 30 * 1000);
    Beep(2);
}

// * ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓ 获取数据get ↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓↓
/************************************************************************************************************
 【函 数】:Get_PlateNumber
 【参 数】:TFTx:Zigbee_Header
 【返 回】:
 【简 例】:
 【说 明】:获取车牌号（字符串）
 ************************************************************************************************************/
uint8_t *Get_PlateNumber(Zigbee_Header TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_PlateNumber, buf);
    ReturnBuffer(DataRequest_PlateNumber);
}
/************************************************************************************************************
 【函 数】:Get_QRCode
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取二维码（字符串）//QRCode_x为向上位机请求的ID,use为从哪个标志物上识别到的二维码
 ************************************************************************************************************/
uint8_t *Get_QRCode(uint8_t QRCode_x, DataRequest_t use)
{
    uint8_t QRID = (QRCode_x == 1) ? DataRequest_QRCode1 : DataRequest_QRCode2;
    uint8_t buf[] = {use};
    ResetRquestWait(QRID, buf);
    ReturnBuffer(QRID);
}
/************************************************************************************************************
 【函 数】:Get_TrafficLight
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取交通灯状态
 ************************************************************************************************************/
uint8_t Get_TrafficLight(uint8_t light_x)
{
    uint8_t buf[] = {light_x};
    ResetRquestWait(DataRequest_TrafficLight, buf);
    ReturnBuffer(DataRequest_TrafficLight)[0];
}
/************************************************************************************************************
 【函 数】:Get_ShapeNumber
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取某个形状的图形个数
 ************************************************************************************************************/
uint8_t Get_ShapeNumber(uint8_t TFTx, Shape_t Shape)
{
    uint8_t buf[] = {TFTx, Shape};
    ResetRquestWait(DataRequest_ShapeNumber, buf);
    ReturnBuffer(DataRequest_ShapeNumber)[0];
}
/************************************************************************************************************
 【函 数】:Get_ColorNumber
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取某个颜色的图形个数
 ************************************************************************************************************/
uint8_t Get_ColorNumber(uint8_t TFTx, Color_t Color)
{
    uint8_t buf[] = {TFTx, Color};
    ResetRquestWait(DataRequest_ColorNumber, buf);
    ReturnBuffer(DataRequest_ColorNumber)[0];
}
/************************************************************************************************************
 【函 数】:Get_ShapeColorNumber
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取某个特定形状颜色的图形个数
 ************************************************************************************************************/
uint8_t Get_ShapeColorNumber(uint8_t TFTx, Shape_t Shape, Color_t Color)
{
    uint8_t buf[] = {TFTx, Shape, Color};
    ResetRquestWait(DataRequest_ShapeColorNumber, buf);
    ReturnBuffer(DataRequest_ShapeColorNumber)[0];
}
/************************************************************************************************************
 【函 数】:Get_RFIDInfo
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取RFID处理结果
 ************************************************************************************************************/
uint8_t *Get_RFIDInfo(uint8_t RFIDx)
{
    uint8_t buf[] = {RFIDx};
    ResetRquestWait(DataRequest_RFID, buf);
    ReturnBuffer(DataRequest_RFID);
}
/************************************************************************************************************
 【函 数】:Get_TFTInfo
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取TFT信息（HEX,3bytes）
 ************************************************************************************************************/
uint8_t *Get_TFTInfo(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_TFTInfo, buf);
    ReturnBuffer(DataRequest_TFTInfo);
}
/************************************************************************************************************
 【函 数】:Get_AllColorCount
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取所有出现的颜色数量
 ************************************************************************************************************/
uint8_t Get_AllColorCount(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_AllColorCount, buf);
    ReturnBuffer(DataRequest_AllColorCount)[0];
}
/************************************************************************************************************
 【函 数】:Get_AllShapeCount
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取所有出现的形状数量
 ************************************************************************************************************/
uint8_t Get_AllShapeCount(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_AllShapeCount, buf);
    ReturnBuffer(DataRequest_AllShapeCount)[0];
}
/************************************************************************************************************
 【函 数】:Get_PresetData
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取预设接口的数据
 ************************************************************************************************************/
uint8_t *Get_PresetData(uint8_t preset_x)
{
    //    preset_x += DataRequest_Preset1 - 1;
    Reset_Rquest_Wait(preset_x, NULL, 0);
    ReturnBuffer(preset_x);
}
/************************************************************************************************************
 【函 数】:Get_TrafficSign
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取某个交通标志
 ************************************************************************************************************/
uint8_t Get_TrafficSign(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_TrafficSign, buf);
    ReturnBuffer(DataRequest_TrafficSign)[0];
}
/************************************************************************************************************
 【函 数】:Get_TrafficSignNumber
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取有效交通标志数量
 ************************************************************************************************************/
uint8_t Get_TrafficSignNumber(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_TrafficSignNumber, buf);
    ReturnBuffer(DataRequest_TrafficSignNumber)[0];
}
/************************************************************************************************************
 【函 数】:Get_CarModel
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取某个车辆车型
 ************************************************************************************************************/
uint8_t Get_CarModel(uint8_t TFTx)
{
    uint8_t buf[] = {TFTx};
    ResetRquestWait(DataRequest_CarModel, buf);
    ReturnBuffer(DataRequest_CarModel)[0];
}
/************************************************************************************************************
 【函 数】:Get_Text
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取文本（字符串）
 ************************************************************************************************************/
uint8_t *Get_Text(uint8_t Text_x)
{
    uint8_t buf[] = {Text_x};
    ResetRquestWait(DataRequest_Text, buf);
    ReturnBuffer(DataRequest_Text);
}
/************************************************************************************************************
 【函 数】:Get_GarageCoord
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取车库位置信息（字符串）
 ************************************************************************************************************/
uint8_t *Get_GarageCoord(void)
{
    uint8_t buf[1];
    ResetRquestWait(DataRequest_GarageCoord, buf);
    ReturnBuffer(DataRequest_GarageCoord);
}
/************************************************************************************************************
 【函 数】:Get_TextNum
 【参 数】:
 【返 回】:
 【简 例】:
 【说 明】:获取数字
 ************************************************************************************************************/
uint8_t Get_TextNum(uint8_t Text_x)
{
    uint8_t buf[] = {Text_x};
    ResetRquestWait(DataRequest_TextNum, buf);
    ReturnBuffer(DataRequest_TextNum)[0];
}
