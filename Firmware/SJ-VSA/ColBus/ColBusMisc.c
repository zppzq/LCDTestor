/*
*********************************************************************************************************
*                                               ColBus
*                                               ͨ��Э��
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               ��Ȩ����
*
*
* �ļ��� : ColBusMisc.c
* ����   : ��п�(kady1984@hotmail.com)
*********************************************************************************************************
*/
#include <string.h>
#include "ColBusDef.h"
#include "ColBusMisc.h"

//��ת������
void EndianConver(uint8* pDes, uint8* pSrc, uint8 nLen)
{
	uint8 i;
	for(i = 0; i < nLen; i++)
	{
		pDes[i] = pSrc[nLen - 1 - i];
	}
}


