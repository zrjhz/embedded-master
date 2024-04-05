/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-03-02 08:12:27
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-03-05 12:58:51
 * @FilePath: \模板 - 副本\APP\Debug\debug.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#if !defined(__DEBUG_H_)
#define __DEBUG_H_

#include "sys.h"

// 调试引脚定义

#define DEBUG_PIN_1 PFout(8)
#define DEBUG_PIN_2 PHout(9)
#define DEBUG_PIN_3 PFout(9)

#define DEBUG_PIN_1_SET() DEBUG_PIN_1 = 1
#define DEBUG_PIN_1_RESET() DEBUG_PIN_1 = 0
#define DEBUG_PIN_1_TOGGLE() DEBUG_PIN_1 = !DEBUG_PIN_1

#define DEBUG_PIN_2_SET() DEBUG_PIN_2 = 1
#define DEBUG_PIN_2_RESET() DEBUG_PIN_2 = 0
#define DEBUG_PIN_2_TOGGLE() DEBUG_PIN_2 = !DEBUG_PIN_2

#define DEBUG_PIN_3_SET() DEBUG_PIN_3 = 1
#define DEBUG_PIN_3_RESET() DEBUG_PIN_3 = 0
#define DEBUG_PIN_3_TOGGLE() DEBUG_PIN_3 = !DEBUG_PIN_3

void DebugTimer_Init(uint16_t arr, uint16_t psc);
void DebugPin_Init(void);
void print_info(char *str, ...);
void Dump_Array(uint8_t *name, uint8_t *array, uint8_t length,uint8_t choose);

// 显示数据

#define dump_array(array, length) Dump_Array(#array, array, length)
#define print_var(var) print_info("%s = %d\r\n", #var, var)
#define print_str(str) print_info("%s:%s\r\n", #str, str)

void ZigBee_Test(uint16_t count ,uint16_t interval);

#endif // __DEBUG_H_
