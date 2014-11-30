/*
*********************************************************************************************************
*                                               ColBus
*                                               通信协议
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               版权所有
*
*
* 文件名 : nwk.h
* 作者   : 杨承凯(kady1984@hotmail.com)
*********************************************************************************************************
*/

#ifndef _NWK_H_
#define _NWK_H_

#include "ColBusDef.h"

//网络层发送函数
int16 NwkSend(uint8* pData, int16 nLen);


//网络层接收函数
int16 NwkRecv(uint8* pData, int16 nLen);


#endif //_NWK_H_
