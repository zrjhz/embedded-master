#if !defined(_A_STAR_H_)
#define _A_STAR_H_

#include "sys.h"
#include "route.h"

// 生成路径

bool A_Star_GetRouteBetweenNodes(RouteNode_t current, RouteNode_t next, RouteNode_t *finalRoute, uint8_t *routeCount);

// 调整障碍点设置

void A_Star_AdjustBarrier(uint8_t *barrierNodes);
void A_Star_ResetBarrier(void);


#endif // _A_STAR_H_
