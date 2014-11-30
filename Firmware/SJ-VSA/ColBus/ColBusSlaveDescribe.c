/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : ColBusSlaveDescribe.c
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/
#include <string.h>
#include "ColBusDef.h"
#include "ColBusSlaveDescribe.h"

//�汾����
uint8 const nCBVersion[2] = {0x01, 0x00};

//�豸����������
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

//�豸����
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

//�˵���Ϣ����
//�˵����
uint8 const arrEpType[DEVICE_EP_COUNT] = {0x09, 0x09, 0x09, 0x09};		//�˵���������
uint8 const arrEpProp[DEVICE_EP_COUNT] = {0x21, 0x21, 0x21, 0x21};		//�˵�����(���ã���̬������)

//�˵�����
DT_STORE g_fEpValue[DEVICE_EP_COUNT];	
void* const arrEpDataPt[DEVICE_EP_COUNT] = {&(g_fEpValue[0]), &(g_fEpValue[1]), &(g_fEpValue[2]), &(g_fEpValue[3])};
uint8 const arrEpDataLen[DEVICE_EP_COUNT] = {2, 2, 2, 2};				//�˵�����ݳ���

//�˵�����λ
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
											sizeof(arrUnitEp0)};					//�˵�����ݳ���




