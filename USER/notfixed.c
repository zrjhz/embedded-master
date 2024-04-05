#include "sys.h"

// 旧的语音识别指令
enum
{
    VoiceCmd_NotUsed = 0x01,         // 未使用
    VoiceCmd_TurnRignt = 0x02,       // 向右转弯
    VoiceCmd_NOTurnRight = 0x03,     // 禁止右转
    VoiceCmd_DrvingToLeft = 0x04,    // 左侧行驶
    VoiceCmd_NODrivingToLeft = 0x05, // 左行被禁
    VoiceCmd_TurnAround = 0x06       // 原地掉头
};

#define STRING_01 "向右转弯"
#define STRING_02 "禁止右转"
#define STRING_03 "左侧行驶"
#define STRING_04 "左行被禁"
#define STRING_05 "原地掉头"
