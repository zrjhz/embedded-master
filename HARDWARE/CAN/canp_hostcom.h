/*
 * @Author: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @Date: 2024-03-02 08:12:27
 * @LastEditors: error: error: git config user.name & please set dead value or install git && error: git config user.email & please set dead value or install git & please set dead value or install git
 * @LastEditTime: 2024-03-05 16:23:11
 * @FilePath: \模板 - 副本\HARDWARE\CAN\canp_hostcom.h
 * @Description: 
 * 
 * Copyright (c) 2024 by ${git_name_email}, All Rights Reserved. 
 */
#include "stm32f4xx.h"
#ifndef __CANP_HOSTCOM_H__
#define __CANP_HOSTCOM_H__
// #include "fifo_drv.h"
// #include "stm32f10x.h"
#ifdef __CANP_HOSTCOM_C__
#define GLOBAL
#else
#define GLOBAL extern
#endif

#define TRACK_Q7 7	// 前7位寻迹数据
#define TRACK_H8 8	// 后8位寻迹数据
#define TRACK_ALL 0 // 所有寻迹数据

GLOBAL void CanP_Init(void);
GLOBAL void CanP_FifoInit(void);
GLOBAL void CanP_TestFifo(void);

GLOBAL void CanP_Host_Main(void);

GLOBAL void Send_ZigbeeData_To_Fifo(u8 *p, u8 len);
GLOBAL void Send_WifiData_To_Fifo(u8 *p, u8 len);
GLOBAL u16 Get_Host_UpTrack(u8 mode);
GLOBAL void Host_Receive_UpTrack(u8 x1, u8 x2);
GLOBAL void CanP_CanTx_Check(void);
GLOBAL void Send_Electric(u8 x1, u8 x2);
GLOBAL void Send_CodedCnt(void);
GLOBAL void Send_UpMotor(int x1, int x2);
GLOBAL void Send_UpCompass(uint16_t c);
GLOBAL void Send_UpMotor_Four(int L1, int L2, int R1, int R2);
GLOBAL void Host_Close_UpTrack(void);
GLOBAL void Host_Open_UpTrack(u8 time);
GLOBAL void Host_Set_UpTrack(u8 time);
GLOBAL void Send_Navig(int x1, int x2);
GLOBAL void Send_Debug_Info(u8 *s, u8 len);
GLOBAL void Send_ZigBee_Info(u8 *s, u8 len);
GLOBAL void Send_InfoData_To_Fifo(u8 *p, u8 len);
GLOBAL void Set_Track_Pwr(u16 power);
GLOBAL void Set_Track_Yzbj(u8 addr, u16 ydata);
GLOBAL void Set_Track_Init(void);

GLOBAL int16_t CanHost_Mp;
GLOBAL uint16_t CanHost_Navig;

#define CANP_CMD_ID_SIZE 7

#define CANP_CMD_ID_MOTO 0
#define CANP_CMD_ID_CNT 1
#define CANP_CMD_ID_NV 2
#define CANP_CMD_ID_POWER 3
#define CANP_CMD_ID_T0 4
#define CANP_CMD_ID_T1 5
#define CANP_CMD_ID_T2 6

GLOBAL void CanP_Cmd_Init(void);
GLOBAL void CanP_Cmd_Write(uint8_t id, uint8_t *p, uint8_t l, uint32_t sid, uint32_t eid);
GLOBAL int8_t CanP_Cmd_Check(void);
GLOBAL void CanP_Cmd_Send(uint8_t id);
GLOBAL void CanP_CanTx_Check_fIrq(void);

#undef GLOBAL

/////
#include "stdbool.h"
// 接收到循迹信息
extern bool trackInfoReceived;

static inline bool Get_TrackInfoReceived(void)
{
	return trackInfoReceived;
}

static inline void Set_TrackInfoReceived(bool status)
{
	trackInfoReceived = status;
}

#endif //__CANP_DISPCOM_H__
