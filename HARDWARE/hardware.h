#if !defined(_HARDWARE_H_)
#define _HARDWARE_H_

#include "sys.h"
#include <stdio.h>
#include "delay.h"
#include "infrared.h"
#include "cba.h"
#include "ultrasonic.h"
#include "canp_hostcom.h"
#include "hard_can.h"
#include "bh1750.h"
#include "power_check.h"
#include "can_user.h"
#include "roadway_check.h"
#include "tba.h"
#include "Can_check.h"
#include "delay.h"
#include "can_user.h"
#include "Timer.h"
#include "Rc522.h"
#include "debug.h"
#include "movement.h"
#include "can_timer.h"
#include "data_interaction.h"
#include "voice.h"
#include "malloc.h"
#include "independent_task.h"
#include "my_lib.h"
#include "uart_a72.h"
#include "seven_segment_display.h"

void Hardware_Init(void);

#endif // _HARDWARE_H_
