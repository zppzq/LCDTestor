/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : nwk.c
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/
#include "includes.h"

//������
//�������ColBus Mac�շ�����������ʹ��
//
//ʹ�÷�����
//ʹ��CBMasterMacSend()�������ݰ�ʱ����CBSlaveMacRecv()����
//ʹ��CBSlaveMacSend()�������ݰ�ʱ����CBMasterMacRecv()����

#include <string.h>
#include "ColBusDef.h"
#include "CrcCheck.h"
#include "nwk.h"

//����㷢�ͺ���
int16 NwkSend(uint8* pData, int16 nLen) reentrant
{
	SendComm2(pData, nLen);

	return nLen;
}

//�������պ���
int16 NwkRecv(uint8* pData, int16 nLen) reentrant
{
	uint16 nRecv;


	nRecv = RecvComm2(pData, nLen);
						

	return nRecv;
}




