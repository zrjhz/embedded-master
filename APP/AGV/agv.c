#include "agv.h"
#include "protocol.h"
#include "string.h"
#include "route.h"
#include "a_star.h"

// 从车ZigBee发送间隔
#define _AGV_ZIGBEE_SEND_INTERVAL_ 80

// 向AGV发送的数据buffer
uint8_t DataToAGV[] = {0x55, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};

#define ClearAGVCmd() memset(&DataToAGV[Pack_MainCmd], 0, 4)
#define SendAGVCmd() Send_ZigBeeData(DataToAGV)
#define SendCmdNTimes(n) Send_ZigBeeDataNTimes(DataToAGV, n, _AGV_ZIGBEE_SEND_INTERVAL_)

// 设定单个路径点
void AGV_SendSinglePoint(uint8_t number, RouteNode_t node)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = AGV_CMD_Routes;
    DataToAGV[Pack_SubCmd1] = number;
    DataToAGV[Pack_SubCmd2] = node.x;
    DataToAGV[Pack_SubCmd3] = node.y;
    SendCmdNTimes(5);
}

// 从任务集合中设定从车任务
void AGV_SetRouteFromTask(RouteNode_t task[], uint8_t length)
{
    for (uint8_t i = 0; i < length; i++)
    {
        AGV_SendSinglePoint(i, task[i]);
    }
}

// 发送字符串格式路径到从车
void AGV_SetRoute(uint8_t *str)
{
    uint8_t length = strlen((char *)str) / 2;
    RouteNode_t tempNode;

    for (uint8_t i = 0; i < length; i++)
    {
        tempNode = Coordinate_Convert(&str[i * 2]);
        AGV_SendSinglePoint(i, tempNode);
    }
}

// 发送多个数据
void AGV_Send_Multi(AGV_CMD_t cmd, uint8_t *data, uint8_t length)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = cmd;
    memcpy(&DataToAGV[Pack_SubCmd1], data, length);
    SendCmdNTimes(10);
}
// 发送单个数据
void AGV_Send_Single(AGV_CMD_t cmd, uint8_t data)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = cmd;
    DataToAGV[Pack_SubCmd1] = data;
    SendCmdNTimes(10);
}
// 不发送数据
void AGV_Send_None(AGV_CMD_t cmd)
{
    ClearAGVCmd();
    DataToAGV[Pack_MainCmd] = cmd;
    SendCmdNTimes(10);
}
