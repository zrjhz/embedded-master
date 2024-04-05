#ifndef __SYN7318_H
#define __SYN7318_H
#include "sys.h"



void SYN7318_Init(void);
void SYN_TTS(uint8_t *str);
void SYN7318_Test(void);
bool VoiceComand_Process(uint8_t *cmd);
uint8_t Start_VoiceCommandRecognition(uint8_t retryTimes);
void USART6_SendString(uint8_t *str, uint16_t len);
	
#endif
