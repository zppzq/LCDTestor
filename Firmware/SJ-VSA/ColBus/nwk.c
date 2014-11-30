/*
*********************************************************************************************************
*                                               ColBus
*                                               通信协议
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               版权所有
*
*
* 文件名 : nwk.c
* 作者   : 杨承凯(kady1984@hotmail.com)
*********************************************************************************************************
*/
#include "includes.h"

//描述：
//用于配合ColBus Mac收发函数，测试使用
//
//使用方法：
//使用CBMasterMacSend()发送数据包时，用CBSlaveMacRecv()接收
//使用CBSlaveMacSend()发送数据包时，用CBMasterMacRecv()接收

#include <string.h>
#include "ColBusDef.h"
#include "CrcCheck.h"
#include "nwk.h"

//网络层发送函数
int16 NwkSend(uint8* pData, int16 nLen) reentrant
{
	SendComm2(pData, nLen);

	return nLen;
}

//网络层接收函数
int16 NwkRecv(uint8* pData, int16 nLen) reentrant
{
	uint16 nRecv;


	nRecv = RecvComm2(pData, nLen);
						

	return nRecv;
}




