/*
*********************************************************************************************************
*                                               ColBus
*                                               通信协议
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               版权所有
*
*
* 文件名 : ColBusSlaveDescribe.c
* 作者   : 杨承凯(kady1984@hotmail.com)
*********************************************************************************************************
*/
#include <string.h>
#include "ColBusDef.h"
#include "ColBusSlaveDescribe.h"

//版本定义
uint8 const nCBVersion[2] = {0x01, 0x00};

//设备制造商描述
uint8 const arrFactoryInfo[18] = 
{
	'S', 0,
	'E', 0,
	'N', 0,
	'&', 0,
	'G', 0,
	'E', 0,
	'N', 0,
	'E', 0,
	 0, 0
};

//设备描述
uint8 const arrDeviceInfo[14] = 
{
	'S', 0,
	'J', 0,
	'-', 0,
	'V', 0,
	'S', 0,
	'A', 0,
	 0, 0
};

//端点信息定义
//端点个数
uint8 const arrEpType[DEVICE_EP_COUNT] = {0x09, 0x09, 0x09, 0x09};		//端点数据类型
uint8 const arrEpProp[DEVICE_EP_COUNT] = {0x21, 0x21, 0x21, 0x21};		//端点属性(启用，静态，输入)

//端点数据
DT_STORE g_fEpValue[DEVICE_EP_COUNT];	
void* const arrEpDataPt[DEVICE_EP_COUNT] = {&(g_fEpValue[0]), &(g_fEpValue[1]), &(g_fEpValue[2]), &(g_fEpValue[3])};
uint8 const arrEpDataLen[DEVICE_EP_COUNT] = {2, 2, 2, 2};				//端点的数据长度

//端点物理单位
uint8 const arrUnitEp0[6] = 
{
	'u', 0,
	'V', 0,
	0, 0
};

void const* const arrEpUnitPt[DEVICE_EP_COUNT] = {arrUnitEp0, arrUnitEp0, arrUnitEp0, arrUnitEp0};
uint8 const arrEpUnitLen[DEVICE_EP_COUNT] = {sizeof(arrUnitEp0), 
											sizeof(arrUnitEp0), 
											sizeof(arrUnitEp0), 
											sizeof(arrUnitEp0)};					//端点的数据长度




