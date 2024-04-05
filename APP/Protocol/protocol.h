#if !defined(__PROTOCOL_H_)
#define __PROTOCOL_H_

#include "sys.h"
#include "delay.h"

// 与上位机通信的发送函数指针
extern void (*Send_ToHost)(uint8_t *, uint8_t);

// 定义连接模式
#define WIRED_CONNECTION 0    // 有线连接（串口）
#define WIRELESS_CONNECTION 1 // 无线连接（WIFI）

// 设定连接模式 true无线,false有线
void SetConnectionMode(bool mode);

// 上传数据包的结构
enum
{
    Pack_Header1,  // 帧头1
    Pack_Header2,  // 帧头2
    Pack_MainCmd,  // 主指令
    Pack_SubCmd1,  // 副指令1
    Pack_SubCmd2,  // 副指令2
    Pack_SubCmd3,  // 副指令3
    Pack_CheckSum, // 校验和
    Pack_Ending    // 帧尾
};

// 上位机发送的数据标识
enum
{
    // 官方文档内指令
    FromHost_Stop = 0x01,                       // 停止
    FromHost_Go = 0x02,                         // 前进
    FromHost_Back = 0x03,                       // 后退
    FromHost_TurnLeft = 0x04,                   // 左转
    FromHost_TurnRight = 0x05,                  // 右转
    FromHost_TrackLine = 0x06,                  // 循迹
    FromHost_EncoderClear = 0x07,               // 码盘清零
    FromHost_TurnCountClockWiseToDigree = 0x08, // 左转弯--角度
    FromHost_TurnClockWiseToDigree = 0x09,      // 右转弯--角度
    FromHost_InfraredFrontData = 0x10,          // 红外前三位数据
    FromHost_InfraredBackData = 0x11,           // 红外后三位数据
    FromHost_InfraredSend = 0x12,               // 通知小车单片机发送红外线
    FromHost_TurnningLightControl = 0x20,       // 转向灯控制
    FromHost_Beep = 0x30,                       // 蜂鸣器
    FromHost_NotUsed = 0x40,                    // 暂未使用
    FromHost_InfraredPhotoPrevious = 0x50,      // 红外发射控制相片上翻
    FromHost_InfraredPhotoNext = 0x51,          // 红外发射控制相片下翻
    FromHost_InfraredLightAdd1 = 0x61,          // 红外发射控制光源强度档位加1
    FromHost_InfraredLightAdd2 = 0x62,          // 红外发射控制光源强度档位加2
    FromHost_InfraredLightAdd3 = 0x63,          // 红外发射控制光源强度档位加3
    FromHost_AGVReturnData = 0x80,              // 从车返回数据
    FromHost_VoiceRecognition = 0x90,           // 语音识别
    FromHost_AGVRoad = 0x58,                    // 道闸信息
    FromHost_AGVOpenMvQRCode = 0x92,            // OpenMv二维码识别
    FromHost_AGVStart = 0xD0,                   // 从车启动
    FromHost_AGVRestart = 0xB0,                 // 从车二次启动
    FromHost_LEDDisplaySecomdRow = 0xc1,        // 数码管第二排显示是数据
    FromHost_ReceivePresetHeadTowards = 0x71,   // 接收预案车头设置
    FromHost_Start = 0xA1,                      // 小车启动命令

    FromHost_QRCodeRecognition = 0xA2,  // 二维码识别
    FromHost_PlateRecognition = 0xA3,   // 车牌识别
    FromHost_ShapeRecongnition = 0xA4,  // 图像识别
    FromHost_TrafficLight = 0xA5,       // 交通灯
    FromHost_StreetLight = 0xA6,        // 路灯
    FromHost_AlarmON = 0xA7,            // 报警器开
    FromHost_AlarmOFF = 0xA8,           // 报警器关
    FromHost_PlateData1 = 0x88,         // 车牌信息1
    FromHost_PlateData2 = 0x99,         // 车牌信息2
    FromHost_Garage = 0xB1,             // 立体车库
    FromHost_TFTRecognition = 0xAC,     // TFT识别
    FromHost_TextRecognition = 0xAD,    // Text识别
    FromHost_TraSignRecognition = 0xAE, // 交通标志识别
    FromHost_Completed = 0xB0,          // 安卓识别任务完成

    // 从车专有的指令
    FromHost_AGVRouting = 0xE1, // AGV 接收路径设置
    FromHost_AGVSetTask = 0xE2, // AGV 接收任务设置
    FromHost_AGVData1 = 0xEA,   // AGV 数据接口1
    FromHost_AGVData2 = 0xEB,   // AGV 数据接口2
    FromHost_AGVData3 = 0xEC,   // AGV 数据接口3
    FromHost_AGVData4 = 0x39,   // AGV 数据接口4  语音天气温度
    FromHost_AGVData5 = 0x37,   // AGV 数据接口5  语音年份
    FromHost_AGVData6 = 0x38,   // AGV 数据接口6  语音时分
    FromHost_AGVData7 = 0x70,   // AGV 数据接口7  处理之后的答案
    FromHost_AGVData8 = 0xD6,   // AGV 数据接口8  车牌前三位
    FromHost_AGVData9 = 0xD7,   // AGV 数据接口9  车牌后三位
    FromHost_AGVData10 = 0xE4,   // AGV 数据接口9  交通标志
};

// 上位机任务请求ID
typedef enum
{
    TFT_Task_Shape = 1,       // 识别图形
    TFT_Task_License = 2,     // 识别车牌
    TFT_Task_TrafficSign = 3, // 识别路标
    TFT_Task_Mask = 4,        // 识别口罩
} TFT_Task_t;

// 安卓接收的下标 不了解安卓端别改!!!!
typedef enum
{
    DataRequest_NotUsed = 0,            // 未使用
    DataRequest_PlateNumber = 1,        // 车牌号
    DataRequest_QRCode1 = 2,            // 二维码1
    DataRequest_QRCode2 = 3,            // 二维码2
    DataRequest_QRCodeSecondCar = 4,    // 二维码3（从车二维码）
    DataRequest_TrafficLight = 5,       // 交通灯
    DataRequest_ShapeNumber = 6,        // 形状数量
    DataRequest_ColorNumber = 7,        // 颜色数量
    DataRequest_ShapeColorNumber = 8,   // 形状颜色数量
    DataRequest_RFID = 9,               // RFID数据
    DataRequest_TFTInfo = 10,           // TFT信息
    DataRequest_AllColorCount = 11,     // 颜色总和
    DataRequest_AllShapeCount = 12,     // 形状总和
    DataRequest_Preset1 = 13,           // 预设1
    DataRequest_Preset2 = 14,           // 预设2
    DataRequest_Preset3 = 15,           // 预设3
    DataRequest_TrafficSign = 16,       // 交通标志
    DataRequest_CarModel = 17,          // 车辆车型
    DataRequest_TrafficSignNumber = 18, // 交通标志数量
    DataRequest_Text = 19,              // 文本
    DataRequest_GarageCoord = 20,       // 车库位置信息
    DataRequest_TextNum = 21,           // 文字数量
} DataRequest_t;

// TODO ZigBee设备 根据文档更改
typedef enum
{
    ZigBee_MainCar = 0x01,        // 主车
    ZigBee_AGV = 0x02,            // 从车
    Zigbee_BarrierGate = 0x03,    // 道闸
    Zigbee_LEDDisplay = 0x04,     // 显示标志物 (LED)
    Zigbee_StereoGarage_A = 0x0D, // 立体车库A
    Zigbee_StereoGarage_B = 0x05, // 立体车库B
    Zigbee_VoiceBroadcast = 0x06, // 智能公交站
    Zigbee_BeaconTower = 0x07,    // 报警台
    Zigbee_SpecialRoad = 0x10,    // 特殊地形
    Zigbee_RotationLED = 0x11,    // 智能立体显示
    Zigbee_TFT_A = 0x0B,          // 多功能信息显示A (TFT)
    Zigbee_TFT_B = 0x08,          // 多功能信息显示B (TFT)
    Zigbee_TFT_C = 0x12,          // 多功能信息显示C (TFT)
    Zigbee_TrafficLight_A = 0x0E, // 智能交通信号灯A
    Zigbee_TrafficLight_B = 0x0F, // 智能交通信号灯B
    Zigbee_TrafficLight_C = 0x13, // 智能交通信号灯C
    Zigbee_TrafficLight_D = 0x14, // 智能交通信号灯D
    ZigBee_WirelessCharge = 0x0A, // 无线充电
    Zigbee_ETC = 0x0C,            // 智能ETC系统
    ZigBee_AutoJudgement = 0xAF,  // 自动评分系统
    Zigbee_StaticMarker_A = 0x15, // 静态标志物A (自定义)
    Zigbee_StaticMarker_B = 0x09, // 静态标志物B (自定义)

    Task_1 = 0x20, // 安卓任务一
    Task_2 = 0x21, // 安卓任务二
    Task_3 = 0x22, // 安卓任务三
    Task_4 = 0x23, // 安卓任务四
    Task_5 = 0x24, // 安卓任务五
    Task_6 = 0x25, // 安卓任务六
    Task_7 = 0x26, // 安卓任务七
    Task_8 = 0x27, // 安卓任务八
    Task_9 = 0x28, // 安卓任务九
} Zigbee_Header;   // 标志物的第二位帧头指令

// 标志物和请求名称区分
enum
{
    MainCar,        // 主车
    AGV,            // 从车
    BarrierGate,    // 道闸
    LEDDisplay,     // 显示标志物 (LED)
    StereoGarage_A, // 立体车库A
    StereoGarage_B, // 立体车库B
    VoiceBroadcast, // 智能公交站
    BeaconTower,    // 报警台
    SpecialRoad,    // 特殊地形
    RotationLED,    // 智能立体显示
    TFT_A,          // 多功能信息显示A (TFT)
    TFT_B,          // 多功能信息显示B (TFT)
    TFT_C,          // 多功能信息显示C (TFT)
    TrafficLight_A, // 智能交通信号灯A
    TrafficLight_B, // 智能交通信号灯B
    TrafficLight_C, // 智能交通信号灯C
    TrafficLight_D, // 智能交通信号灯D
    WirelessCharge, // 无线充电
    ETC,            // 智能ETC系统
    AutoJudgement,  // 自动评分系统
};

// 通用ZigBee回传的数据状态和时间戳
typedef struct ZigBee_DataStatus_Sturuct
{
    uint8_t isSet;
    uint8_t cmd[8];
    uint32_t timeStamp;
} ZigBee_DataStatus_t;

// AGV预设任务
enum
{
    AGVPreasetTask_AdjustBarrier = 9, // 障碍点设置
    AGVPreasetTask_QRCode = 8,        // 扫描二维码并上传
    AGVPreasetTask_Streetlight = 7,   // 调整路灯档位
    AGVPreasetTask_Distance = 6,      // 上传距离信息
    AGVPreasetTask_SpecialRoad = 5,   // 特殊地形
    AGVPreasetTask_TrafficLight = 4,  // 交通灯识别
    AGVPreasetTask_Image = 3,         // 图像识别
};

// AGV预设数据
enum
{
    AGVPresetData_StreetLight = 0xEF, // 路灯档位
    AGVPresetData_TrafficSign = 0xEA, // 立体显示交通标志
    AGVData_carnum1 = 0xD6,           // 车牌前三位
    AGVData_carnum2 = 0xD7,           // 车牌后三位
};

// 从车上传数据结构
enum
{
    AGVUploadData_Header1 = 0,  // 包头1
    AGVUploadData_Header2 = 1,  // 包头2
    AGVUploadData_DataType = 2, // 数据类型
};

// 从车上传数据类型
enum
{
    AGVUploadType_Ultrasonic = 0x0B, // 超声波
    AGVUploadType_Brightness = 0x0A, // 光照度
    AGVUploadType_Alarm1 = 0x10,     // 烽火台
    AGVUploadType_carflag = 0xB0,    // 临时避让二次启动
    AGVUploadType_QRCodeData = 0x92, // 二维码数据
    AGVUploadType_MisonComplete = 0xFF,
    AGVUploadType_platenum = 0x15, // 车牌
    AGV_Bfloor = 0x2B,             // B车库初始
    AGV_Afloor = 0x2A,             // A车库初始
};

// 从车RFID数据结构
enum
{
    AGVRFID_Route = 1,      // 从车路径
    AGVRFID_Coordinate = 2, // 从车初始坐标
};

// 数据 发送/请求/返回 的包结构

// 发送 [0][1] 包头 [2] 发送ID [3] 数据长度 [4+] 数据区域
// 请求 [0][1] 包头 [2] 请求ID [3+] 附加参数区（用途/区分）
// 返回 [0][1] 包头 [2] 返回ID [3+] 数据区域
enum
{
    // 共用
    Data_Header1 = 0, // 头1
    Data_Header2 = 1, // 头2
    Data_ID = 2,      // ID

    // 非共用         // 定义-------|请求|发送|返回|
    Data_Use = 3,     // 数据用途位 | Y  |    |    |
    Data_Length = 3,  // 长度定义位 |    | Y  | Y  |
    Data_Content = 4, // 数据发送位 |    | Y  |    |
};

// 数据发送ID
typedef enum
{
    DataSend_QRCode = 0,     // 二维码
    DataSend_RFID = 1,       // RFID1
    DataSend_RFID2 = 2,      // RFID2
    DataSend_RFID3 = 3,      // RFID3
    DataSend_Calculator = 4, // 计算公式
    DataSend_Var1 = 5,       // 变量1
    DataSend_Var2 = 6,       // 变量2
    DataSend_Var3 = 7,       // 变量3
    DataSend_Var4 = 8,       // 变量4
    DataSend_QText = 9,      // 文本
} Datasend_t;

// 数据请求储存格式
typedef struct DataSetting_Struct
{
    uint8_t *buffer;     // 数据buffer
    uint8_t Data_Length; // 数据长度信息
    uint8_t isSet;       // 接收标志位
} DataSetting_t;

// 标志物协议的命令和功能字节

// 特殊地形标志物
enum
{
    SpecialRoadMode_ReturnStatus = 0x10, // 请求回传特殊地形通信状态
};

// 报警灯标志物
enum
{
    Alarm_ReturnResCoord = 0x09,  // 请求回传随机救援坐标点
    Alarm_CodeFront3Bytes = 0x10, // 报警码前三位
    Alarm_CodeBack3Bytes = 0x11   // 报警码后三位
};

// 道闸标志物
enum
{
    BarrierGateMode_Control = 0x01,          // 控制开关
    BarrierGateMode_GateInitialAngle = 0x09, //  闸门初始角度调节
    BarrierGateMode_PlateFront3Bytes = 0x10, // 车牌前三位
    BarrierGateMode_PlateBack3Bytes = 0x11,  // 车牌后三位
    BarrierGateMode_ReturnStatus = 0x20      // 状态返回
};

// ETC 标志物
enum
{
    ETCMode_GateInitialAngle = 0x08, //  闸门初始角度调节
};

// LED显示标志物
enum
{
    LEDDisplayMainCmd_DataToFirstRow = 0x01, // 第一行显示数据
    LEDDisplayMainCmd_DataToSecondRow,       // 第二行显示数据
    LEDDisplayMainCmd_TimerMode,             // 计时模式
    LEDDisplayMainCmd_ShowDistance           // 显示距离
};

// 立体显示标志物
enum
{
    RotationLEDMode_PlateFront4BytesData = 0x20,        // 接收前四位车牌信息
    RotationLEDMode_PlateBack2BytesAndCoordInfo = 0x10, // 接收后两位车牌和两位坐标信息并显示
    RotationLEDMode_Distance = 0x11,                    // 显示距离
    RotationLEDMode_Shape = 0x12,                       // 显示图形
    RotationLEDMode_Color = 0x13,                       // 显示颜色
    RotationLEDMode_TrafficWarnSign = 0x14,             // 显示交通警示牌信息
    RotationLEDMode_TrafficSign = 0x15,                 // 显示交通标志
    RotationLEDMode_Default = 0x16,                     // 显示默认
    RotationLEDMode_SetTextColor = 0x17,                // 设置文字显示颜色
    RotationLEDMode_TextAccumulation = 0x31,            // 自定义文本累加显示
    RotationLEDMode_TextClear = 0x32,                   // 自定义文本清空显示
};

// TFT显示器标志物
enum
{
    TFTMode_Picture = 0x10,    // 图片
    TFTMode_PlateDataA = 0x20, // 车牌数据A
    TFTMode_PlateDataB = 0x21, // 车牌数据B
    TFTMode_Timer = 0x30,      // 计时
    TFTMode_Hex = 0x40,        // HEX显示
    TFTMode_Distance = 0x50,   // 距离显示（十进制）
    TFTMode_TrafficSign = 0x60 // 交通标志
};

// 立体车库标志物
enum
{
    StereoGarage_Control = 0x01, // 控制
    StereoGarage_Return = 0x02   // 返回
};

// 交通灯标志物
enum
{
    TrafficLight_Recognition = 0x01, // 进入识别模式
    TrafficLight_Confirm = 0x02      // 确认识别结果
};

// 语音播报标志物
enum
{
    VoiceCmd_Specific = 0x10,          // 特定语音
    VoiceCmd_Random = 0x20,            // 随机语音
    VoiceCmd_SetStartData = 0x30,      // 设置RTC起始日期
    VoiceCmd_QueryCurrentData = 0x31,  // 查询RTC当前日期
    VoiceCmd_SetStartTime = 0x40,      // 设置RTC起始时间
    VoiceCmd_QueryCurrentTime = 0x41,  // 查询RTC当前时间
    VoiceCmd_SetWeaTem = 0x42,         // 设置天气数据与温度数据
    VoiceCmd_RequestWeaTemData = 0x43, // 请求回传天气数据与温度数据
};

// 计时控制
typedef enum
{
    TimerMode_OFF = 0x00, // 计时关
    TimerMode_ON,         // 计时开
    TimerMode_Clear       // 计时清零
} TimerMode_t;

// 交通灯定义
typedef enum
{
    TrafficLightColor_Red = 0x01,   // 红灯
    TrafficLightColor_Green = 0x02, // 绿灯
    TrafficLightColor_Yellow = 0x03 // 黄灯
} TrafficLightColor_t;

// 形状定义
typedef enum
{
    Shape_NotDefined = 0, // 未定义
    Shape_Rectangle,      // 矩形
    Shape_Circle,         // 圆形
    Shape_Triangle,       // 三角形
    Shape_Diamond,        // 菱形
    Shape_Trapezoid,      // 梯形
    Shape_Pie,            // 饼图
    Shape_Traget,         // 靶图
    Shape_Bar,            // 条形图
    Shape_Pentagram,      // 五角星
} Shape_t;

// 颜色定义
typedef enum
{
    Color_NotDefined = 0, // 未定义
    Color_Red,            // 红
    Color_Green,          // 绿
    Color_Blue,           // 蓝
    Color_Yellow,         // 黄
    Color_PurpleMagenta,  // 紫（品）
    Color_Cyan,           // 青
    Color_Black,          // 黑
    Color_White,          // 白
} Color_t;

//	交通警示牌
typedef enum
{
    RouteTrafficWarnSign_plate_2 = 0x01, // 前方学校 减速慢行
    RouteTrafficWarnSign_Plate2 = 0x02,  // 前方施工 注意安全
    RouteTrafficWarnSign_Plate3 = 0x03,  // 塌方路段 注意安全
    RouteTrafficWarnSign_Plate4 = 0x04,  // 追尾危险 保持车距
    RouteTrafficWarnSign_Plate5 = 0x05,  // 严禁 酒后驾车！
    RouteTrafficWarnSign_Plate6 = 0x06,  // 严禁 乱扔垃圾！
} RouteTrafficWarnSign_t;

// 交通标志定义
typedef enum
{
    Number_NotDefined = 0, // 未定义
    Number_Straight,       // 直行
    Number_TurnLeft,       // 左转
    Number_TurnRight,      // 右转
    Number_TurnRound,      // 掉头
    Number_ForbidStraight, // 禁止直行
    Number_ForbidPass,     // 禁止通行
} RouteTrafficSign_t;

// 车辆车型定义
typedef enum
{
    Model_NotDefined = 0, // 未定义
    Model_Bicycle,        // 自行车
    Model_Motorcycle,     // 摩托车
    Model_Car,            // 小轿车
    Model_Truck,          // 货车
} CarModel_t;

// 天气数据
typedef enum
{
    Weather_Gale = 0,      // 大风
    Weather_Cloudy = 1,    // 多云
    Weather_Clear = 2,     // 晴
    Weather_LightSnow = 3, // 小雪
    Weather_LightRain = 4, // 小雨
    Weather_Clouds = 5,    // 阴云
} Weather_t;

// 下面的数据是基本不需要操作的固定数据,可直接发送

// 红外指令
static uint8_t Infrared_LightAdd1[4] = {0x00, 0xFF, 0x0C, ~(0x0C)};         // 光源档位加1
static uint8_t Infrared_LightAdd2[4] = {0x00, 0xFF, 0x18, ~(0x18)};         // 光源档位加2
static uint8_t Infrared_LightAdd3[4] = {0x00, 0xFF, 0x5E, ~(0x5E)};         // 光源档位加3
static uint8_t Infrared_AlarmON[6] = {0x03, 0x05, 0x14, 0x45, 0xDE, 0x92};  // 报警器打开
static uint8_t Infrared_AlarmOFF[6] = {0x67, 0x34, 0x78, 0xA2, 0xFD, 0x27}; // 报警器关闭

// 交通灯
static uint8_t ZigBee_TrafficLightStartRecognition[8] = {0x55, 0x0E, 0x01, 0x00, 0x00, 0x00, 0x01, 0xBB}; // 进入识别模式

// 无线充电
static uint8_t ZigBee_WirelessChargingON[8] = {0x55, 0x0a, 0x01, 0x01, 0x00, 0x00, 0x02, 0xBB};  // 开启无线充电站
static uint8_t ZigBee_WirelessChargingOFF[8] = {0x55, 0x0A, 0x01, 0x02, 0x00, 0x00, 0x03, 0xBB}; // 关闭无线充电站

// 语音播报指令
static uint8_t ZigBee_VoiceRandom[8] = {0x55, 0x06, 0x20, 0x01, 0x00, 0x00, 0x00, 0xBB}; // 随机播报语音指令
// 下面时标志物默认指令,基本不用
static uint8_t ZigBee_VoiceTurnRight[8] = {0x55, 0x06, 0x10, 0x02, 0x00, 0x00, 0x12, 0xBB};      // 向右转弯
static uint8_t ZigBee_VoiceNOTurnRight[8] = {0x55, 0x06, 0x10, 0x03, 0x00, 0x00, 0x13, 0xBB};    // 禁止右转
static uint8_t ZigBee_VoiceDriveLeft[8] = {0x55, 0x06, 0x10, 0x04, 0x00, 0x00, 0x14, 0xBB};      // 左侧行驶
static uint8_t ZigBee_VoiceNODriveLeft[8] = {0x55, 0x06, 0x10, 0x05, 0x00, 0x00, 0x15, 0xBB};    // 左行被禁
static uint8_t ZigBee_VoiceTurnAround[8] = {0x55, 0x06, 0x10, 0x06, 0x00, 0x00, 0x16, 0xBB};     // 原地掉头
static uint8_t ZigBee_VoiceDriveAssistant[8] = {0x55, 0x06, 0x10, 0x01, 0x00, 0x00, 0x11, 0xBB}; // 驾驶助手

// 各个标志物的指令根据下面的模板填充后发送
// 标志物ZigBee包头为0xAA(固定)和0xXX(ZigBee编号),包尾为0xBB
// 除自动评分终端外,其它ZigBee数据都需要校验后发送

// 基本格式：8字节,除自动评分不校验之外其它均遵循此规则
// [0] 0x55 包头 [1] ZigBee设备ID [2] 主指令 [3-5] 副指令 [6] 校验和 [7] 0xBB 包尾

static uint8_t ZigBee_AlarmData[8] = {0x55, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};        // 报警灯
static uint8_t ZigBee_BarrierGateData[8] = {0x55, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};  // 道闸
static uint8_t ZigBee_ETCData[8] = {0x55, 0x0C, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};          // 	ETC
static uint8_t ZigBee_LEDDisplayData[8] = {0x55, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};   // LED显示
static uint8_t Infrared_RotationLEDData[6] = {0xFF, 0x00, 0x00, 0x00, 0x00, 0x00};            // 旋转LED
static uint8_t ZigBee_RotationLEDData[8] = {0x55, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};  // 旋转LED Zigbee
static uint8_t ZigBee_TFTData[8] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};          // TFT显示器（A/B）
static uint8_t ZigBee_StereoGarageData[8] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB}; // 立体车库（A/B）
static uint8_t ZigBee_TrafficLightData[8] = {0x55, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB}; // 智能交通灯（A/B）
static uint8_t ZigBee_VoiceData[8] = {0x55, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};        // 语音播报
static uint8_t ZigBee_VoiceReturnData[8] = {0xAF, 0x06, 0x00, 0x02, 0x00, 0x00, 0x01, 0xBB};  // 语音返回自动评分终端
static uint8_t ZigBee_SpecialRoadData[8] = {0x55, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0xBB};  // 特殊地形

// 当前指令状态和数据内容存放(指令不连续和标志位使用造成的空间浪费暂时未解决)
extern uint8_t CommandFlagStatus[0xFF];

// 数据请求编号数量
extern uint8_t DATA_REQUEST_NUMBER;
extern DataSetting_t DataBuffer[];

#define GetCmdFlag(id) CommandFlagStatus[id]
#define SetCmdFlag(id) CommandFlagStatus[id] = SET
#define ResetCmdFlag(id) CommandFlagStatus[id] = RESET

// 执行N次,带延迟
#define ExcuteNTimes(Task, N, delay)    \
    do                                  \
    {                                   \
        for (uint8_t i = 0; i < N; i++) \
        {                               \
            Task;                       \
            delay_ms(delay);            \
        }                               \
    } while (0)

// 自动判断数据长度
// warning: 传入参数不可为指针！
#define Infrared_Send_A(infraredData) Infrared_Send(infraredData, sizeof(infraredData))

void Send_ZigBeeData(uint8_t *data);
void Send_ZigBeeDataNTimes(uint8_t *data, uint8_t ntimes, uint16_t delay);
void Send_DataToUsart(uint8_t *buf, uint8_t length);
void Check_Sum(uint8_t *cmd);

#endif // __PROTOCOL_H_
