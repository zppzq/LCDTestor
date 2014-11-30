/****************************************Copyright (c)**************************************************
**                              
**                               
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: BrgNet.h
**创   建   人: 杨承凯
**创 建 日  期: 2007年4月7日
**最后修改日期: 2007年4月7日
**描        述: 120.11通信协议测量端网络层协议
********************************************************************************************************/
#ifndef		_BRG_NET_H_
#define		_BRG_NET_H_

#ifdef 		_BRG_NET_C_
#define		BRG_NET_EXT
#else
#define		BRG_NET_EXT	extern
#endif

//函数编译控制
#define	_BRG_NET_RECV_
#define	_BRG_NET_SEND_
#define	_GET_LOCAL_ID_
#define	_SET_SEND_CHANNEL_ID_


#define	BRG_NET_DEBUG		0

//全局变量定义
extern uint8 idata	g_nChannelID;		//通道号地址

BRG_NET_EXT uint16 GetCollectorLocalID(void) reentrant;
BRG_NET_EXT BOOL BrgNetRecv(uint8 *pBuff,uint16 nLen) reentrant;
BRG_NET_EXT void BrgNetSend(uint8 *pBuff,uint16 nLen) reentrant;
void SetSendChannelID(uint8 nChannelID) reentrant;

//**************************************************************************************************
#endif
