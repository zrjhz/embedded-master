#if !defined(_ROUTE_H_)
#define _ROUTE_H_

#include "sys.h"

// 坐标点信息
typedef struct RouteNode_Struct
{
	int8_t x;
	int8_t y;
	int8_t dir; // 用到不多,需要精简
} RouteNode_t;


typedef struct taskCoordinate
{
    uint8_t taskID;
    uint8_t *coord;
} taskCoord_t;



// 方向定义
typedef enum
{
	DIR_NOTSET = 0,		// 未设定
	DIR_UP = 1,			// 上
	DIR_LEFT = 2,		// 左
	DIR_DOWN = 3,		// 下
	DIR_RIGHT = 4,		// 右
	DIR_LEFT_UP = 5,	// 左上
	DIR_LEFT_DOWN = 6,  // 左下
	DIR_RIGHT_UP = 7,   // 右上
	DIR_RIGHT_DOWN = 8, // 右下
} Direction_t;

typedef struct DataToAGV_Strusct
{
    uint8_t *startPoint;    // 起点
    uint8_t *beforeRouteInfo; // 未知路径前
    uint8_t *routeInfo;       // 未知路径
    uint8_t *afterRouteInfo;  // 未知路径后
    uint8_t *endPoint;         // 终点

    uint8_t direction;        // 方向
    uint8_t *alarmData;        // 报警台数据
    taskCoord_t *taskCoord;    // 任务路径
    uint8_t taskNumber;        // 任务数量
    uint8_t *avoidGarage;      // 道闸避让
    uint8_t *avoidGarage2;     // 道闸避让2
    uint8_t routeLength;
    uint8_t streetLightLevel;
    uint8_t trafficSign;
    uint8_t *AGV_data;
    uint8_t *carnum;
} DataToAGV_t;

// 设定任务点（转换字符串生成坐标信息）
typedef struct RouteSetting_Struct
{
	uint8_t coordinate[3]; // 字符串坐标
	RouteNode_t node;	  // 坐标点数据
	void (*Task)(void);	// 任务函数指针
} RouteSetting_t;

// 任务设定和任务个数
extern RouteSetting_t Route_Task[];
extern uint8_t ROUTE_TASK_NUMBER;

// 寻卡测试用路径
extern RouteSetting_t RFID_TestRoute[];
extern uint8_t RFID_TESTROUTE_NUMBER;

extern RouteNode_t CurrentStatus;
extern RouteNode_t NextStatus;

// 坐标字符串转换

RouteNode_t Coordinate_Convert(uint8_t str[3]);
uint8_t *ReCoordinate_Convert(RouteNode_t coordinate);

// 坐标生成

bool Generate_Routetask(RouteSetting_t routeSetting[], uint8_t count);

// 坐标信息提取和处理

int8_t Get_TaskNumber(uint8_t coordinate[3], uint8_t *route, uint8_t nTimes);
int8_t Is_ContainCoordinate(uint8_t *stringRoute, uint8_t coord[3]);
Direction_t Get_TowardsByNode(RouteNode_t currentNode, RouteNode_t towardsNode);
Direction_t Get_Towards(uint8_t current[3], uint8_t towards[3]);

RouteNode_t Get_TowardsCoordinate(RouteNode_t center, uint8_t towards);
Direction_t Get_OppositeDirection(Direction_t dir);

// 根据当前方向转到指定方向

void Turn_ToDirection(int8_t *current, Direction_t target, void (*Turn_Once)(Direction_t));

#endif // _ROUTE_H_
