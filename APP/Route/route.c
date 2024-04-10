#include "route.h"
#include "task.h"
#include "string.h"
#include "malloc.h"
#include "debug.h"
#include "my_lib.h"
#include "a_star.h"
#include "independent_task.h"

// 当前位置状态
RouteNode_t CurrentStatus;
// 下一个位置和状态
RouteNode_t NextStatus;
// 错误的字符串坐标
static const char *badCoordinate = "\0\0\0";

// 错误的节点坐标
static const RouteNode_t badNode = {.x = -1, .y = -1, .dir = DIR_NOTSET};
//  任务和路径设定
RouteSetting_t Route_Task[] = {
    {.coordinate = NULL, .Task = NULL, .node.dir = DIR_LEFT},
};

// 任务点个数
uint8_t ROUTE_TASK_NUMBER = GET_ARRAY_LENGEH(Route_Task);

/************************************************************************************************************
 【函 数】:Coordinate_Convert
 【参 数】:coordinate:字符串坐标
 【返 回】:
 【简 例】:
 【说 明】:Coordinate_Convert("D7");
 ************************************************************************************************************/
// 转换字符串到坐标点
RouteNode_t Coordinate_Convert(uint8_t coordinate[3])
{
    RouteNode_t outNode;
    outNode.dir = DIR_NOTSET;

    if ((coordinate[0] >= 'A') && (coordinate[0] <= 'G'))
    {
        outNode.x = coordinate[0] - 'A';
    }
    else // 不合法的x坐标
        return badNode;

    if ((coordinate[1] >= '1') && (coordinate[1] <= '7'))
    {
        outNode.y = '7' - coordinate[1];
    }
    else // 不合法的y坐标
        return badNode;

    return outNode;
}

// 转换坐标点到字符串
uint8_t *ReCoordinate_Convert(RouteNode_t coordinate)
{
    static uint8_t tempCoordinate[3];

    if (coordinate.x >= 0 && coordinate.x <= 6)
    {
        tempCoordinate[0] = 'A' + coordinate.x;
    }
    else // 不合法的x坐标
        return (uint8_t *)badCoordinate;

    if (coordinate.y >= 0 && coordinate.y <= 6)
    {
        tempCoordinate[1] = '7' - coordinate.y;
    }
    else // 不合法的y坐标
        return (uint8_t *)badCoordinate;

    return tempCoordinate;
}

// 获取任务点在路径中出现第n次的位置，返回-1为错误
// warning:此操作不会计算中间点
int8_t Get_TaskNumber(uint8_t coordinate[3], uint8_t *route, uint8_t nTimes)
{
    uint8_t count = 0;

    char *location = NULL;
    char *tempRoute = (char *)route;

    for (;;)
    {
        location = strstr(tempRoute, (char *)coordinate);
        if (location == NULL)
            return -1;
        else
        {
            tempRoute = location + 2; // 指向下一个坐标点的字符
            if (++count >= nTimes)
                return (location - (char *)route) / 2; // 算出第n次出现的任务位置
        }
    }
}
/************************************************************************************************************
 【函 数】:Generate_Routetask
 【参 数】:routeSetting:任务路径    count:任务数量
 【返 回】:
 【简 例】:
 【说 明】:
 【说 明】:将Route_Task[]中的.coordinate转为.node结构体
 ************************************************************************************************************/
// 从设定路径中的字符串生成坐标路径
bool Generate_Routetask(RouteSetting_t routeSetting[], uint8_t count)
{
    RouteNode_t tempNode;

    for (uint8_t i = 0; i < count; i++)
    {
        tempNode = Coordinate_Convert(routeSetting[i].coordinate);
        routeSetting[i].node.x = tempNode.x; // 防止设定的方向被变更
        routeSetting[i].node.y = tempNode.y;

        if (routeSetting[i].node.x == -1 || routeSetting[i].node.y == -1)
            return false; // 无效节点，退出

        print_info((i < count - 1) ? "(%d, %d)->" : "(%d, %d)", routeSetting[i].node.x, routeSetting[i].node.y);
        delay_ms(50);
    }

    print_info("\r\n");

    return true;
}

// 判断坐标是否在路径上
// 若在返回步数，否则返回-1
// warning: -1为真值，不能判断真假
int8_t Is_ContainCoordinate(uint8_t *stringRoute, uint8_t coord[3])
{
    uint8_t length = strlen((char *)stringRoute) / 2;

    RouteNode_t *route = malloc(sizeof(RouteNode_t) * length);
    if (route == NULL)
        return -1;

    for (uint8_t i = 0; i < length; i++)
    {
        route[i] = Coordinate_Convert(&stringRoute[i * 2]);
    }

    RouteNode_t *tempRoute = malloc(sizeof(RouteNode_t) * 12); // 两点间最多12途径点
    if (tempRoute == NULL)
    {
        free(route);
        return -1;
    }

    uint8_t routeCount;
    uint8_t allRouteCount = 0;
    RouteNode_t coordinate = Coordinate_Convert(coord);

    for (uint8_t i = 0; i < length - 1; i++)
    {
        A_Star_GetRouteBetweenNodes(route[i], route[i + 1], tempRoute, &routeCount);

        for (uint8_t j = 0; j < routeCount; j++)
        {
            if ((tempRoute[j].x == coordinate.x) && (tempRoute[j].y == coordinate.y))
            {
                free(tempRoute);
                free(route);
                return allRouteCount + j;
            }
        }
        // 记录路径个数，去掉一个起始点
        allRouteCount += (routeCount - 1);
    }

    free(tempRoute);
    free(route);

    return -1;
}

// 根据当前坐标和朝向的坐标获取车头朝向（节点参数）
Direction_t Get_TowardsByNode(RouteNode_t currentNode, RouteNode_t towardsNode)
{
    int8_t dx = towardsNode.x - currentNode.x;
    int8_t dy = towardsNode.y - currentNode.y;

    if ((dx > 1 || dy > 1 || dx < -1 || dy < -1) || (dx != 0 && dy != 0))
    {
        print_info("illegal Node!!\r\n");
        return DIR_NOTSET;
    }

    if (dx > 0)
        return DIR_RIGHT;

    else if (dx < 0)
        return DIR_LEFT;

    else if (dy > 0)
        return DIR_UP;

    else if (dy < 0)
        return DIR_DOWN;

    else
        return DIR_NOTSET;
}

// 根据当前坐标和朝向的坐标获取车头朝向
// 不检查字符串中信息正确性，请确保输入正确坐标
Direction_t Get_Towards(uint8_t current[3], uint8_t towards[3])
{
    RouteNode_t currentNode = Coordinate_Convert(current);
    RouteNode_t towardsNode = Coordinate_Convert(towards);

    return Get_TowardsByNode(currentNode, towardsNode);
}

// 使用传入函数转到特定方向并更新当前方向
// void (*Turn_Once)(Direction_t) 为左/右转向90度使用的函数
void Turn_ToDirection(int8_t *current, Direction_t target, void (*Turn_Once)(Direction_t))
{
    // 非常规方向
    if (target > DIR_RIGHT || target == DIR_NOTSET)
        return;

    switch (*current)
    {
    case DIR_UP:
        switch (target)
        {
        case DIR_UP:
            break;
        case DIR_DOWN:
            Turn_Once(DIR_RIGHT);
            Turn_Once(DIR_RIGHT);
            break;
        case DIR_LEFT:
            Turn_Once(DIR_LEFT);
            break;
        case DIR_RIGHT:
            Turn_Once(DIR_RIGHT);
            break;
        default:
            break;
        }
        break;

    case DIR_DOWN:
        switch (target)
        {
        case DIR_UP:
            Turn_Once(DIR_RIGHT);
            Turn_Once(DIR_RIGHT);
            break;
        case DIR_DOWN:
            break;
        case DIR_LEFT:
            Turn_Once(DIR_RIGHT);
            break;
        case DIR_RIGHT:
            Turn_Once(DIR_LEFT);
            break;
        default:
            break;
        }
        break;

    case DIR_LEFT:
        switch (target)
        {
        case DIR_UP:
            Turn_Once(DIR_RIGHT);
            break;
        case DIR_DOWN:
            Turn_Once(DIR_LEFT);
            break;
        case DIR_LEFT:
            break;
        case DIR_RIGHT:
            Turn_Once(DIR_RIGHT);
            Turn_Once(DIR_RIGHT);
            break;
        default:
            break;
        }
        break;

    case DIR_RIGHT:
        switch (target)
        {
        case DIR_UP:
            Turn_Once(DIR_LEFT);
            break;
        case DIR_DOWN:
            Turn_Once(DIR_RIGHT);
            break;
        case DIR_LEFT:
            Turn_Once(DIR_RIGHT);
            Turn_Once(DIR_RIGHT);
            break;
        case DIR_RIGHT:
            break;
        default:
            break;
        }
        break;

    case DIR_LEFT_UP:
        switch (target)
        {
        case DIR_UP:
            Turn_Once(DIR_RIGHT);
            break;
        case DIR_LEFT:
            Turn_Once(DIR_LEFT);
            break;
        default:
            break;
        }
        break;

    case DIR_RIGHT_UP:
        switch (target)
        {
        case DIR_UP:
            Turn_Once(DIR_LEFT);
            break;
        case DIR_RIGHT:
            Turn_Once(DIR_RIGHT);
            break;
        default:
            break;
        }
        break;

    case DIR_LEFT_DOWN:
        switch (target)
        {
        case DIR_DOWN:
            Turn_Once(DIR_LEFT);
            break;
        case DIR_LEFT:
            Turn_Once(DIR_RIGHT);
            break;
        default:
            break;
        }
        break;

    case DIR_RIGHT_DOWN:
        switch (target)
        {
        case DIR_DOWN:
            Turn_Once(DIR_RIGHT);
            break;
        case DIR_RIGHT:
            Turn_Once(DIR_LEFT);
            break;
        default:
            break;
        }
        break;

    default:
        break;
    }
    *current = target;
}

// 计算当前点和朝向计算出朝向的坐标点
RouteNode_t Get_TowardsCoordinate(RouteNode_t center, uint8_t towards)
{
    switch (towards)
    {
    case DIR_UP:
        center.y++;
        break;
    case DIR_LEFT:
        center.x--;
        break;
    case DIR_DOWN:
        center.y--;
        break;
    case DIR_RIGHT:
        center.x++;
        break;
    case DIR_LEFT_UP:
        center.x--;
        center.y++;
        break;
    case DIR_LEFT_DOWN:
        center.x--;
        center.y--;
        break;
    case DIR_RIGHT_UP:
        center.x++;
        center.y++;
        break;
    case DIR_RIGHT_DOWN:
        center.x++;
        center.y--;
        break;

    default:
        center = badNode;
        break;
    }
    return center;
}

// 获取相反的方向
Direction_t Get_OppositeDirection(Direction_t dir)
{
    // 非常规方向
    if (dir > DIR_RIGHT || dir == DIR_NOTSET)
        return DIR_NOTSET;

    Direction_t oppositeDir = DIR_NOTSET;

    switch (dir)
    {
    case DIR_UP:
        oppositeDir = DIR_DOWN;
        break;
    case DIR_LEFT:
        oppositeDir = DIR_RIGHT;
        break;
    case DIR_DOWN:
        oppositeDir = DIR_UP;
        break;
    case DIR_RIGHT:
        oppositeDir = DIR_LEFT;
        break;

    default:
        break;
    }
    return oppositeDir;
}
