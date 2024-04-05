#if !defined(_INDEPENDENT_TASK_H)
#define _INDEPENDENT_TASK_H

#include "sys.h"
#include "protocol.h"
#include "route.h"
#include "roadway_check.h"

extern RouteNode_t CurrentStatus;
#define Send_ZigBeeData5Times(data) Send_ZigBeeDataNTimes(data, 5, 200)

typedef struct Block_Info_Struct
{
    uint8_t block;     // 块
    uint8_t authMode; // 密钥模式
    uint8_t key[6];   // 密钥
} Block_Info_t;

typedef struct RFID_Info_Struct
{
    uint8_t data[3][17];    // 读出来的数据
    RouteNode_t coordinate; // 坐标点
    Block_Info_t *blockInfo;
    uint8_t blockNumber;
} RFID_Info_t;

extern RFID_Info_t *CurrentRFIDCard;
extern uint8_t RFID_Index;

extern RFID_Info_t RFID1;
extern RFID_Info_t RFID2;
extern RFID_Info_t RFID3;
extern RFID_Info_t RFID4;



void Emergency_Flasher(uint16_t time);
void Resume_RunningStatus(uint16_t encoderChangeValue);
uint8_t Read_RFID(void);
ErrorStatus Write_RFID(RFID_Info_t *RFIDx, uint8_t *data_str);
void RFID_Task(void);
void RFID_Start(void);
void RFID_End(void);
bool Get_SpecialRoadStatus(void);
void SpecialRoad_ReturnStatus(void);
void SpecialRoad_End(void);
void SpecialRoad_Start(void);

void Alarm_ON(uint8_t code[6]);
void AlarmReturn_RescueCoord(void);
void Alarm_ChangeCode(uint8_t code[6]);
void RFIDx_Begin(uint8_t index);
void BarrierGate_Plate(uint8_t plate[6]);
void BarrierGate_AdjustAngle(uint8_t angle);
void BarrierGate_ReturnStatus(void);
void BarrierGate_Task(uint8_t plate[6]);
void BarrierGate_Control(bool status);

void LEDDisplay_DataToFistRow(uint8_t data[3]);
void LEDDisplay_DataToSecondRow(uint8_t data[3]);
void LEDDisplay_TimerMode(TimerMode_t mode);
void LEDDisplay_Distance(uint16_t dis);

void RotationLED_PlateAndCoord(uint8_t plate[6], RouteNode_t coord);
void RotationLED_Distance(uint8_t dis);
void RotationLED_Shape(Shape_t shape);
void RotationLED_Color(Color_t color);
void RotationLED_Default(void);
void RotationLED_TrafficWarningSign(RouteTrafficWarnSign_t TraWarnSign);
void RotationLED_TrafficSign(RouteTrafficSign_t TraSign);
void RotationLED_SetTextColor(uint8_t Red, uint8_t Green, uint8_t Blue);
void RotationLED_TextAccumulation(uint8_t chineseData1, uint8_t chineseData2, uint8_t choice);
void RotationLED_TextClear(uint8_t ClearMode);
void RotationLEDZigBee_TextAccumulation(uint8_t chineseData1, uint8_t chineseData2, uint8_t choice);
void RotationLED_zigdie(uint8_t *chr);
void RotationLEDZigBee_TextClear(uint8_t ClearMode);
void RotationLED_CoordAndDistance(uint16_t distance);
void RotationLED_TextOverlay(uint8_t *chr);

Zigbee_Header Get_TFT_Index(uint8_t index);
void TFT_ShowPicture(uint8_t TFTx, uint8_t picNumber);
void TFT_PicturePrevious(uint8_t TFTx);
void TFT_PictureNext(uint8_t TFTx);
void TFT_PictureAuto(uint8_t TFTx);
void TFT_Plate(uint8_t TFTx, uint8_t plate[6]);
void TFT_Timer(uint8_t TFTx, TimerMode_t mode);
void TFT_HexData(uint8_t TFTx, uint8_t data[3]);
void TFT_Distance(uint8_t TFTx, uint16_t dis);
void TFT_TrafficSign(uint8_t TFTx, RouteTrafficSign_t TraSign);

Zigbee_Header Get_StereoGarage_Index(uint8_t index);
void StereoGarage_ToLayer(uint8_t garage_x, uint8_t layer);
void StereoGarage_ReturnLayer(uint8_t garage_x);
void StereoGarage_ReturnInfraredStatus(uint8_t garage_x);

Zigbee_Header Get_TrafficLight_Index(uint8_t index);
void TrafficLight_RecognitionMode(uint8_t light_x);
void TrafficLight_ConfirmColor(uint8_t light_x, TrafficLightColor_t light);

void VoiceBroadcast_Specific(uint8_t voiceID);
void VoiceBroadcast_Radom(void);
void VoiceRTC_StartData(uint8_t year, uint8_t month, uint8_t day);
void VoiceRTC_CurrentData(void);
void VoiceRTC_StartTime(uint8_t hour, uint8_t minute, uint8_t second);
void VoiceRTC_CurrentTime(void);
void VoiceSet_WeaTem(Weather_t weather, uint8_t temperature);
void VoiceReturn_WeaTem(void);
void VoiceRecognition_Return(uint8_t voiceID);
void Voice_Recognition(void);

Zigbee_Header Get_StaticMarker_Index(uint8_t index);

#define WirelessCharging_ON() Send_ZigBeeDataNTimes(ZigBee_WirelessChargingON, 5, 100)
#define WirelessCharging_OFF() Send_ZigBeeDataNTimes(ZigBee_WirelessChargingOFF, 5, 100)
#define WirelessCharging_CodeON() Send_ZigBeeData(code)

void StreetLight_AdReset(void);
uint8_t StreetLight_Now(void);
uint8_t StreetLight_AdjustTo(uint8_t targetLevel);
uint8_t StreetLight_AdjustTotargetLevel(uint8_t targetLevel);

#define LEDDisplay_Timer(status) LEDDisplay_TimerMode(TimerMode_##status);

void Start_Task(void);
void End_Task(void);
uint16_t DistanceMeasure_Task(void);
void RouteString_Process(DataToAGV_t agvData, uint8_t *buffer);
void ETC_Task(void);
void AGV_Task(DataToAGV_t agvData);
void AGV_Task_With_Msg(DataToAGV_t agvData);
uint8_t *VoiceRuturnTime(void);
uint8_t *VoiceRuturnData(void);
uint8_t *VoiceRuturnWea(void);
void Wait_GarageToFristLayer(uint8_t StereoGarage_x);
void Wait_CarIntoGarage(uint8_t StereoGarage_x, uint8_t targetLayer);
void Reverse_Parcking(RouteNode_t *current, uint8_t targetGarage[3], uint8_t StereoGarage_x);
void RFID_Handler(void);

void BEEP_Triple(void);
void Start_Task(void);
void End_Task(void);

#endif // _INDEPENDENT_TASK_H
