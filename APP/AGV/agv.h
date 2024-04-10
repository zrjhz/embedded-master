#if !defined(_AGV_H_)
#define _AGV_H_

#include "sys.h"
#include "protocol.h"
#include "route.h"

void AGV_SetRouteFromTask(RouteNode_t task[], uint8_t length);
void AGV_SetRoute(uint8_t *str);

void AGV_Send_Multi(AGV_CMD_t cmd, uint8_t *data, uint8_t length);
void AGV_Send_Single(AGV_CMD_t cmd, uint8_t data);
void AGV_Send_None(AGV_CMD_t cmd);
#endif // _AGV_H_
