#if !defined(_SEVENN_SEG_H_)
#define _SEVENN_SEG_H_

#include "sys.h"

void SevenSegmentDisplay_PortInit(void);
void SevenSegmentDisplay_Refresh(void);
void SevenSegmentDisplay_Update(uint8_t data);
void SevenSegmentDisplay_UpdateHex(uint8_t data);

void Display_Number(uint8_t number);

#endif // _SEVENN_SEG_H_
