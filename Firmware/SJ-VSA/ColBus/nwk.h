/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : nwk.h
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/

#ifndef _NWK_H_
#define _NWK_H_

#include "ColBusDef.h"

//����㷢�ͺ���
int16 NwkSend(uint8* pData, int16 nLen);


//�������պ���
int16 NwkRecv(uint8* pData, int16 nLen);


#endif //_NWK_H_
