#include "protocol.h"
#include "my_lib.h"
#include "debug.h"
#include "uart_drv.h"
#include "uart_a72.h"
#include "Timer.h"
#include "canp_hostcom.h"

// 定义每种数据的长度
enum
{
    DataLength_PlateNumber = 6,       // 车牌号
    DataLength_QRcode1 = 16,          // 二维码1
    DataLength_QRcode2 = 16,          // 二维码2
    DataLength_QRCodeSecondCar = 8,   // 二维码3
    DataLength_TrafficLight = 1,      // 交通灯
    DataLength_ShapeNumber = 1,       // 形状数量
    DataLength_ColorNumber = 1,       // 颜色数量
    DataLength_ShapeColorNumber = 1,  // 形状颜色数量
    DataLength_RFID = 16,             // RFID数据
    DataLength_TFTInfo = 8,           // TFT信息
    DataLength_AllColorCount = 1,     // 颜色总和
    DataLength_AllShapeCount = 1,     // 形状总和
    DataLength_Preset1 = 16,          // 预设1
    DataLength_Preset2 = 16,          // 预设2
    DataLength_Preset3 = 16,          // 预设3
    DataLength_TrafficSign = 1,       // 交通标志
    DataLength_CarModel = 1,          // 车辆车型
    DataLength_TrafficSignNumber = 1, // 交通标志数量
    DataLength_Text = 20,             // 文本
    DataLength_GarageCoord = 2,       // 车库位置信息
    DataLength_TextNum = 1,           // 文字数量
};                                    // 与头文件中的数据请求和返回ID对应

// 与上位机通信的发送函数指针
void (*Send_ToHost)(uint8_t *, uint8_t) = NULL;

// 定义数据buffer
#define DefineBuffer(X) uint8_t Data_##X[DataLength_##X]
// 在结构体中给出buffer指针和长度
#define DataAndLength(X)                \
    {                                   \
        Data_##X, DataLength_##X, RESET \
    }

// 请求数据的buffer
DefineBuffer(PlateNumber);
DefineBuffer(QRcode1);
DefineBuffer(QRcode2);
DefineBuffer(QRCodeSecondCar);
DefineBuffer(TrafficLight);
DefineBuffer(ShapeNumber);
DefineBuffer(ColorNumber);
DefineBuffer(ShapeColorNumber);
DefineBuffer(RFID);
DefineBuffer(TFTInfo);
DefineBuffer(AllColorCount);
DefineBuffer(AllShapeCount);
DefineBuffer(Preset1);
DefineBuffer(Preset2);
DefineBuffer(Preset3);
DefineBuffer(TrafficSign);
DefineBuffer(CarModel);
DefineBuffer(TrafficSignNumber);
DefineBuffer(Text);
DefineBuffer(GarageCoord);
DefineBuffer(TextNum);

// 储存buffer指针/长度/状态的结构体数组
DataSetting_t DataBuffer[] = {
    {(uint8_t *)NULL, 0, 0},
    DataAndLength(PlateNumber),
    DataAndLength(QRcode1),
    DataAndLength(QRcode2),
    DataAndLength(QRCodeSecondCar),
    DataAndLength(TrafficLight),
    DataAndLength(ShapeNumber),
    DataAndLength(ColorNumber),
    DataAndLength(ShapeColorNumber),
    DataAndLength(RFID),
    DataAndLength(TFTInfo),
    DataAndLength(AllColorCount),
    DataAndLength(AllShapeCount),
    DataAndLength(Preset1),
    DataAndLength(Preset2),
    DataAndLength(Preset3),
    DataAndLength(TrafficSign),
    DataAndLength(TrafficSignNumber),
    DataAndLength(CarModel),
    DataAndLength(Text),
    DataAndLength(GarageCoord),
    DataAndLength(TextNum),
};
// 数据请求命令个数
uint8_t DATA_REQUEST_NUMBER = GET_ARRAY_LENGEH(DataBuffer);

// 上位机指令接收状态
uint8_t CommandFlagStatus[0xFF] = {0};

#if 0
// C中没有泛型,有些函数不容易实现,所以这里宏定义实现
void ExcuteNTimes(void(Task *)(void), N, delay)
{
    for (uint8_t i = 0; i < N; i++)
    {
        Task();
        delay_ms(delay);
    }
}
#endif

// 单次发送,带校验（八位）
void Send_ZigBeeData(uint8_t *data)
{
    Check_Sum(data);
    Send_ZigbeeData_To_Fifo(data, 8);
}

// 多次发送,带校验（八位）
void Send_ZigBeeDataNTimes(uint8_t *data, uint8_t ntimes, uint16_t delay)
{
    Check_Sum(data);
    for (uint8_t i = 0; i < ntimes; i++)
    {
        Send_ZigbeeData_To_Fifo(data, 8);
        if (delay > 0)
        {
            delay_ms(delay);
        }
    }
}

// 发送数据到串口(A72开发板)
void Send_DataToUsart(uint8_t *buf, uint8_t length)
{
    if (length == 0)
        return;

    uint32_t timeStamp;

    UartA72_TxClear();
    UartA72_TxAddStr(buf, length);
    UartA72_TxStart();

    // 等待发送完成
    timeStamp = millis();
    while (!UartTx_EndCheck(PUART_A72TX))
    {
        if (IsTimeOut(timeStamp, 10))
            break;
    }
}

// 将校验和填入cmd[Pack_CheckSum]中
void Check_Sum(uint8_t *cmd)
{
    uint16_t temp = cmd[2] + cmd[3] + cmd[4] + cmd[5];
    cmd[Pack_CheckSum] = (uint8_t)(temp % 256);
}

// 设定连接模式
void SetConnectionMode(bool mode)
{
    Send_ToHost = mode ? Send_WifiData_To_Fifo : Send_DataToUsart;
    print_info("\r\n%s\r\n", mode ? "WIRELESS" : "WIRED");
}
