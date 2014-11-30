/*
*********************************************************************************************************
*                                               ColBus
*                                               通信协议
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               版权所有
*
*
* 文件名 : ColBusMisc.c
* 作者   : 杨承凯(kady1984@hotmail.com)
*********************************************************************************************************
*/
#include <string.h>
#include "ColBusDef.h"
#include "ColBusMisc.h"

//端转换函数
void EndianConver(uint8* pDes, uint8* pSrc, uint8 nLen)
{
	uint8 i;
	for(i = 0; i < nLen; i++)
	{
		pDes[i] = pSrc[nLen - 1 - i];
	}
}


