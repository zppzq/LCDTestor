/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : ColBusSlaveMac.h
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/

#ifndef _COL_BUS_SLAVE_MAC_H_
#define _COL_BUS_SLAVE_MAC_H_

#include "ColBusDef.h"

//ColBus���豸���պ���
int16 CBSlaveNetRecv(uint8* pData, int16 nLen);


//ColBus���豸���ͺ���
int16 CBSlaveNetSend(uint8* pData, int16 nLen);

//У�������ַ
BOOL CheckAddrNet(uint8* arrAddr, uint8 nLen);



#endif //_COL_BUS_SLAVE_MAC_H_

