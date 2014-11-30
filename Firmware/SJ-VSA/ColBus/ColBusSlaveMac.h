/*
*********************************************************************************************************
*                                               ColBus
*                                               通信协议
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               版权所有
*
*
* 文件名 : ColBusSlaveMac.h
* 作者   : 杨承凯(kady1984@hotmail.com)
*********************************************************************************************************
*/

#ifndef _COL_BUS_SLAVE_MAC_H_
#define _COL_BUS_SLAVE_MAC_H_

#include "ColBusDef.h"

//ColBus从设备接收函数
int16 CBSlaveNetRecv(uint8* pData, int16 nLen);


//ColBus从设备发送函数
int16 CBSlaveNetSend(uint8* pData, int16 nLen);

//校验网络地址
BOOL CheckAddrNet(uint8* arrAddr, uint8 nLen);



#endif //_COL_BUS_SLAVE_MAC_H_

