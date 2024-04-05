#include <stdint.h>
#include "stm32f4xx.h"
#define __MY_LIB_C__
#include "my_lib.h"
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "debug.h"
#include "stdlib.h"
uint32_t U8ToU32(uint8_t *datas)
{
	// return ((u32)datas[0])|((u32)datas[1]<<8 )|((u32)datas[2]<<16)|((u32)datas[3]<<24);
	uint32_t temp = 0;
	temp = datas[3];
	temp = (temp << 8) | datas[2];
	temp = (temp << 8) | datas[1];
	temp = (temp << 8) | datas[0];
	return temp;
}

void U32ToU8(uint8_t *Buf, uint32_t Datas)
{
	Buf[0] = Datas;
	Buf[1] = Datas >> 8;
	Buf[2] = Datas >> 16;
	Buf[3] = Datas >> 24;
}

uint16_t U8ToU16(uint8_t *datas)
{
	uint32_t temp = 0;
	temp = datas[1];
	temp = (temp << 8) | datas[0];
	return temp;
}

uint16_t U8ToU16_Big(uint8_t *datas)
{
	uint32_t temp = 0;
	temp = datas[0];
	temp = (temp << 8) | datas[1];
	return temp;
}

void U16ToU8(uint8_t *Buf, uint16_t Datas)
{
	Buf[0] = Datas;
	Buf[1] = Datas >> 8;
}

uint8_t MLib_GetSum(uint8_t *p, uint32_t l)
{
	uint8_t Rt = 0;
	while (l--)
		Rt += *p++;
	return Rt;
}

uint16_t MLib_GetShortSum(uint16_t *p, uint32_t dot)
{
	uint16_t Rt = 0;
	while (dot--)
		Rt += *p++;
	return Rt;
}

uint8_t MLib_FindFastBit(uint32_t d)
{
	uint8_t i;
	uint32_t f = 0x01;
	for (i = 0; i < 32; i++, f <<= 1)
	{
		if (d & f)
			break;
	}
	return i;
}

#if 0
void MLib_Gpio_Bits_Write(GPIO_TypeDef* GPIOx,u16 bits,u16 data)
{
	//GPIO_SetBits(GPIOx,bits & data);
	GPIOx->BSRRL = bits & data;
	//GPIO_ResetBits(GPIOx,bits & (~data));
	GPIOx->BSRRH = bits & (~data);
}
#endif

uint32_t MLib_GetDataSub(uint32_t d1, uint32_t d2)
{
	return (d1 > d2) ? d1 - d2 : d2 - d1;
}

void MLib_memcpy(void *d, void *s, uint32_t lb)
{
	uint8_t *r, *w;
	if (lb)
	{
		if (d < s)
		{
			r = (uint8_t *)s;
			w = (uint8_t *)d;
			while (lb--)
				*w++ = *r++;
		}
		else
		{
			r = ((uint8_t *)s) + lb - 1;
			w = ((uint8_t *)d) + lb - 1;
			while (lb--)
				*w-- = *r--;
		}
	}
}

void MLib_memset(void *b, uint8_t d, uint32_t lb)
{
	uint8_t *p = (uint8_t *)b;
	while (lb--)
		*p++ = d;
}

void MLib_memint(void *b, uint8_t d, uint32_t lb)
{
	uint8_t *p = (uint8_t *)b;
	while (lb--)
		*p++ = d++;
}

int8_t MLib_memcmp(void *b1, void *b2, uint32_t lb)
{
	int8_t Rt = 0;
	uint8_t *s1 = (uint8_t *)b1;
	uint8_t *s2 = (uint8_t *)b2;
	while (lb--)
	{
		if (*s1 != *s2)
		{
			if (*s1 < *s2)
				Rt = -1;
			else
				Rt = 1;
			break;
		}
		s1++;
		s2++;
	}
	return Rt;
}

int constrain_int(int x, int a, int b)
{
	if ((x >= a) && (x <= b))
	{
		return x;
	}
	else
	{
		return (x < a) ? a : b;
	}
}

float constrain_float(float x, float a, float b)
{
	if ((x >= a) && (x <= b))
	{
		return x;
	}
	else
	{
		return (x < a) ? a : b;
	}
}

#define LongToBin(n)      \
	(((n >> 21) & 0x80) | \
	 ((n >> 18) & 0x40) | \
	 ((n >> 15) & 0x20) | \
	 ((n >> 12) & 0x10) | \
	 ((n >> 9) & 0x08) |  \
	 ((n >> 6) & 0x04) |  \
	 ((n >> 3) & 0x02) |  \
	 ((n) & 0x01))

#define Bin(n) LongToBin(0x##n##l)

// BCD转HEX
uint8_t BCD2HEX(uint8_t bcd_data)
{
	uint8_t temp;
	temp = ((bcd_data >> 8) * 100) | ((bcd_data >> 4) * 10) | (bcd_data & 0x0f);
	return temp;
}

// HEX转BCD
uint8_t HEX2BCD(uint8_t hex_data)
{
	uint8_t bcd_data;
	uint8_t temp;
	temp = hex_data % 100;
	bcd_data = ((uint8_t)hex_data) / 100 << 8;
	bcd_data = bcd_data | temp / 10 << 4;
	bcd_data = bcd_data | temp % 10;
	return bcd_data;
}

// 冒泡排序 升序
void bubble_sort(uint16_t arr[], uint16_t len)
{
	uint16_t i, j, temp;
	for (i = 0; i < len - 1; i++)
		for (j = 0; j < len - 1 - i; j++)
			if (arr[j] > arr[j + 1])
			{
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
}

// 冒泡升序
void bubble_sort_2(uint8_t arr[], uint8_t len)
{
	uint8_t i, j, temp;
	for (i = 0; i < len - 1; i++)
		for (j = 0; j < len - 1 - i; j++)
			if (arr[j] > arr[j + 1])
			{
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
}

// TFT冒泡排序
void TFT_bubble(uint8_t arr[], uint8_t len)
{
	uint8_t i, j, temp;
	uint8_t TFT_arr[len];
	for (i = 0; i < len - 1; i++)
	{
		for (j = 0; j < len - 1 - i; j++)
		{
			// 右移四位去除字母后排序
			TFT_arr[j] = arr[j] << 4;
			TFT_arr[j + 1] = arr[j + 1] << 4;
			if (TFT_arr[j] > TFT_arr[j + 1])
			{
				temp = arr[j];
				arr[j] = arr[j + 1];
				arr[j + 1] = temp;
			}
			else if (TFT_arr[j] == TFT_arr[j + 1]) // 数字相等 再作字母判断
			{
				if (arr[j] > arr[j + 1])
				{
					temp = arr[j];
					arr[j] = arr[j + 1];
					arr[j + 1] = temp;
				}
			}
		}
	}
}

// RFID截取从车路径
// 返回0则出错
uint8_t *AGV_RFID(uint8_t route[], uint8_t len, uint8_t flag)
{
	static uint8_t agvroute[20] = {0};
	static uint8_t agvcoord[20] = {0};
	for (int i = 0; i < len; i++)
	{
		if (route[i] == '|' && flag == 1)
		{
			for (int j = 0; j < i; j++)
			{
				agvroute[j] = route[j];
			}
			return agvroute;
		}
		else if (route[i] == '|' && flag == 2)
		{
			agvcoord[0] = route[i + 1];
			agvcoord[1] = route[i + 2];
			return agvcoord;
		}
	}
	return 0;
}

// 无限循环
void infinity_loop(void)
{
	for (;;)
	{
	}
}

// 返回绝对值最小的元素
int MinimumAbsOf(int *array, uint16_t length)
{
	int minimum = abs(array[0]);
	uint16_t miniumID = 0;
	int temp;
	for (uint16_t i = 0; i < length; i++)
	{
		temp = abs(array[i]);
		if (minimum > temp)
		{
			minimum = temp;
			miniumID = i;
		}
	}
	return array[miniumID];
}

// 返回字符串中数字最小值下标
uint8_t BackMinSubscript(uint8_t *array, uint16_t length)
{
	int temp = 1;
	int MinSubscript = 0;
	for (uint8_t i = 0; i < length; i++)
	{
		if ('0' <= array[i] && array[i] <= '9')
		{
			if (temp)
			{
				MinSubscript = i;
				temp = 0;
			}
			if (array[i] < array[MinSubscript])
			{
				MinSubscript = i;
			}
		}
	}
	return MinSubscript;
}

// 返回字符串中数字最大值下标
uint8_t BackMaxSubscript(uint8_t *array, uint16_t length)
{
	int temp = 1;
	int MaxSubscript = 0;
	for (uint8_t i = 0; i < length; i++)
	{
		if ('0' <= array[i] && array[i] <= '9')
		{
			if (temp)
			{
				MaxSubscript = i;
				temp = 0;
			}
			if (array[i] > array[MaxSubscript])
			{
				MaxSubscript = i;
			}
		}
	}
	return MaxSubscript;
}

// 返回不同数据中最小下标
uint8_t BackMINSubscript_2(uint8_t *array, uint16_t length)
{
	int MinSubscript = 0;
	for (uint8_t i = 0; i < length; i++)
	{
		if (array[i] < array[MinSubscript])
		{
			MinSubscript = i;
		}
	}
	return MinSubscript;
}

// 字符串中十六进制提取chen例如D3->D*16+3

// 提取数字
uint8_t BackNumberFromStr(uint8_t str[3])
{
	uint8_t Number = 0;
	Number = (uint8_t)str[0] * 16 + (uint8_t)str[1];
	return Number;
}
uint16_t Extract_number(uint8_t num[], uint8_t len)
{
	uint16_t Number;
	uint8_t NumGroup[5] = {0};
	uint8_t j = 0;
	uint8_t b = 0;
	uint8_t a = 0;
	for (int i = 0; i < len; i++)
	{
		if (num[i] > '0' && num[i] <= '9')
		{
			NumGroup[j] = num[i];
			j++;
		}
	}

	b = strlen((char *)NumGroup);
	bubble_sort_2(NumGroup, b);
	for (int t = 0; t < b; t++)
	{
		NumGroup[t] = NumGroup[t] - 0x30;
	}
	while (b)
	{
		b--;
		Number = Number * 10 + NumGroup[a];
		a++;
	}
	return Number;
}

uint8_t *get_data(uint8_t num[], uint8_t len)
{
	uint8_t *NumGroup = malloc(sizeof(uint8_t) * 5);
	int j = 0;
	for (int i = 0; i < len; i++)
	{
		if ((num[i] > '0' && num[i] <= '9') || (num[i] >= 'A' && num[i] <= 'Z'))
		{
			NumGroup[j++] = num[i];
		}
	}
	bubble_sort_2(NumGroup, j);
	return NumGroup;
}

void get_by_char(uint8_t arr[], uint8_t *res)
{
	const uint8_t *start = arr;
	while (*start && *start != '[')
		start++;

	const uint8_t *end = start + 1;
	while (*end && *end != ']')
		end++;

	uint8_t index = 0;
	for (const uint8_t *ptr = start + 1; ptr < end; ptr++)
	{
		if (*ptr >= '0' && *ptr <= '9')
		{
			res[index++] = hexValue(*ptr);
		}
	}
}

bool get_by_char_2(const uint8_t *arr, char *res)
{
	int count = 0;
	bool inside_braces = false;

	while (*arr != '\0')
	{
		if (*arr == '{')
		{
			inside_braces = true;
		}
		else if (*arr == '}')
		{
			inside_braces = false;
			break; // 结束循环，因为我们只需要第一个花括号内的字符串
		}
		else if (inside_braces)
		{
			res[count] = (char)*arr;
			count++;
		}

		arr++;
	}

	res[count] = '\0'; // 在结果字符串末尾添加 NULL 终止符

	return inside_braces; // 返回是否成功提取了花括号内的字符串
}
//提取数字和字母
void get_by_char_3(uint8_t arr[], uint8_t *res)
{
	const uint8_t *start = arr;

	const uint8_t *end = start + 1;
	while (*end && *end != '\0')
		end++;

	uint8_t index = 0;
	for (const uint8_t *ptr = start + 1; ptr < end; ptr++)
	{
		if ((*ptr >= '0' && *ptr <= '9') || (*ptr >= 'A' && *ptr <= 'F'))
		{
			res[index++] = hexValue(*ptr);
		}
	}
}

char binaryToChar(const char *binaryString)
{
	char result = 0;
	int i;
	for (i = 0; binaryString[i] != '\0'; i++)
	{
		result = (result << 1) + (binaryString[i] - '0');
	}
	return result;
}

void copyArrayRange(const uint8_t *inputArray, uint8_t start, uint8_t end, uint8_t *outputArray)
{
	uint8_t i, j;
	j = 0;

	for (i = start; i <= end; i++)
	{
		outputArray[j] = inputArray[i];
		j++;
	}
	outputArray[j++] = '\0';
}

char *ITA2(uint8_t value[])
{
	char *c[] = {
		"A",
		"B",
		"C",
		"D",
		"E",
		"F",
		"G",
		"H",
		"I",
		"J",
		"K",
		"L",
		"M",
		"N",
		"O",
		"P",
		"Q",
		"R",
		"S",
		"T",
		"U",
		"V",
		"W",
		"X",
		"Y",
		"Z",
		"0",
		"1",
		"2",
		"3",
		"4",
		"5",
		"6",
		"7",
		"8",
		"9",
		"0",
	};
	char *s[] = {
		"1100001",
		"1000110",
		"1010100",
		"1000010",
		"1000010",
		"1010010",
		"1100100",
		"1000001",
		"1010000",
		"1100011",
		"1011000",
		"1001000",
		"1101000",
		"1011100",
		"1001110",
		"1001101",
		"1100010",
		"1011010",
		"1001010",
		"1100000",
		"1011001",
		"1101110",
		"1011011",
		"1101010",
		"1001100",
		"1011101",
		"1111111",
		"1101111",
		"1101101",
		"1110111",
		"1110011",
		"1110001",
		"1100101",
		"1111010",
		"1101001",
		"1101100",
	};

	uint8_t index = 0, equl = 1;
	for (uint8_t i = 0; i < GET_ARRAY_LENGEH(s); i++)
	{
		equl = 1;
		for (uint8_t j = 0; j < 7; j++)
		{
			if (value[j] != s[i][j])
			{
				equl = 0;
				break;
			}
		}
		if (equl)
		{
			index = i;
			break;
		}
	}
	return c[index];
}
/************************************************************************************************************
 【函 数】:hexValue
 【参 数】:c:字符的ASCII码
 【返 回】:整数
 【简 例】:hexValue('A'); 返回10
 【说 明】:通过ASCII码返回整数
 ************************************************************************************************************/
int hexValue(char c)
{
	if (c >= '0' && c <= '9')
		return c - '0';
	else if (c >= 'A' && c <= 'F')
		return c - 'A' + 10;
	else if (c >= 'a' && c <= 'f')
		return c - 'a' + 10;
	else
		return -1; // 输入无效，返回-1表示错误
}
/************************************************************************************************************
 【函 数】:combineHex
 【参 数】:c1 & c2:字符的ASCII码
 【返 回】:整数
 【简 例】:combineHex('A', '1'); 返回0xA1
 【说 明】:通过两个ASCII码返回整数
 ************************************************************************************************************/
int combineHex(char c1, char c2)
{
	print_info("%c %c\r\n", c1, c2);
	int value1 = hexValue(c1);
	int value2 = hexValue(c2);

	if (value1 == -1 || value2 == -1)
		return -1; // 输入无效，返回-1表示错误

	return (value1 << 4) | value2;
}
/************************************************************************************************************
 【函 数】:combineHex
 【参 数】:a & b:两个16进制数
 【返 回】:整数
 【简 例】:combineHex(0x0A, 0x01); 返回0xA1
 【说 明】:返回整数
 ************************************************************************************************************/
uint8_t mergeHexValues(uint8_t a, uint8_t b)
{
	uint8_t result = (a << 4) | b;
	return result;
}
