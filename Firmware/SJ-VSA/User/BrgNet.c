/****************************************Copyright (c)**************************************************
**                              
**                                
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: BrgNet.c
**创   建   人: 杨承凯
**创 建 日  期: 2007年4月7日
**最后修改日期: 2007年4月7日
**描        述: 120.11通信协议测量端网络层协议
********************************************************************************************************/
#define		_BRG_NET_C_

//头文件
#include "includes.h"
#include "BrgNet.h"


//上下层协议函数接口


//数据定义
//#define Flash_ID_Mode
#ifdef Flash_ID_Mode								 
uint16	code 		g_FlashBrgID  _at_ 0x0400;
#endif

//指定本地ID号
uint16 idata 		g_nCollectorLocalID = 56;

//接收到的ID号
uint16  		g_nNetID;			//网络总地址
uint16  		g_nRelayID;			//中转器地址
uint16  		g_nCollectorID;		//采集器地址
uint8 	 		g_nOutChannelID;	//外部通道号
uint8 idata		g_nChannelID;		//通道号地址
		  														   
/****************************************************************************
* 名	称：GetCollectorLocalID()
* 功	能：获取采集器本地地址
* 入口参数：无
* 出口参数：采集器ID
* 说	明：无
****************************************************************************/
#ifdef	_GET_LOCAL_ID_
uint16 GetCollectorLocalID() reentrant
{
#ifdef Flash_ID_Mode
	g_nCollectorLocalID = g_FlashBrgID;	
#endif
	
	return g_nCollectorLocalID;
}
#endif

/****************************************************************************
* 名	称：GetInnerChannel()
* 功	能：获取内部通道号
* 入口参数：外部通道号1 ~ 4
* 出口参数：内部通道号0 ~ 3
* 说	明：无
****************************************************************************/
BOOL GetInnerChannel(INT8U nChannelNum) reentrant
{
	if(0xFF == nChannelNum)	return 0;	//如果是0xFF，则返回0
	return (nChannelNum -1);
}


/****************************************************************************
* 名	称：BrgNetRecv()
* 功	能：102.11网络层接收协议
* 入口参数：pBuff  待接收的数据
			nLen   数据长度
* 出口参数：0   数据出错
			非0 处理正常
* 说	明：无
****************************************************************************/
#ifdef	_BRG_NET_RECV_
BOOL BrgNetRecv(uint8 *pBuff,uint16 nLen) reentrant
{
	//临时变量
	uint8 i;

#ifdef Flash_ID_Mode
	g_nCollectorLocalID = g_FlashBrgID;	
#endif

	//读取无线数据
	WirelessReadData(pBuff, nLen+BRG_ADRC_LEN);

	//CRC校验，如果出错，则返回false
	if(CRC16(pBuff,nLen+BRG_ADRC_LEN) != 0) 
	{
		_nop_();
		return FALSE;
	}

	//获取ID	
	g_nNetID = ntohs(*((uint16*)(pBuff)));
	g_nRelayID = g_nNetID & RELAY_NET_ID;
	g_nCollectorID = g_nNetID & COLLECTOR_NET_ID;
	g_nOutChannelID = pBuff[BRG_CHANNEL_OFFSET];
	g_nChannelID = GetInnerChannel(g_nOutChannelID);

	//检查是否呼叫本机，如果不是，返回false
	if((g_nCollectorID == g_nCollectorLocalID) || (g_nCollectorID == COLLECTOR_NET_ID))
	{
		//缓冲数据到应用层
		for(i = 0; i < nLen; i++)
		{
			pBuff[i] = pBuff[i + BRG_ADDR_LEN];
		}
		return TRUE;
	}
	return FALSE;
}
#endif


/****************************************************************************
* 名	称：BrgNetSend()
* 功	能：102.11网络层发送协议
* 入口参数：pBuff  待接收的数据
			nLen   数据长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_BRG_NET_SEND_
void BrgNetSend(uint8 *pBuff,uint16 nLen) reentrant
{ 
#ifdef Flash_ID_Mode
	g_nCollectorLocalID = g_FlashBrgID;	
#endif

	//移动数据指针到帧头部
	pBuff -= BRG_ADDR_LEN;

	//添加ID信息
	*((uint16*)(pBuff)) = htons(g_nCollectorLocalID | g_nRelayID);
	pBuff[BRG_CHANNEL_OFFSET] = g_nOutChannelID;

	nLen += BRG_ADDR_LEN;

	//添加CRC校验码
	*((uint16*)(pBuff+nLen)) = htons(CRC16(pBuff,nLen));
	nLen += BRG_CRC_LEN;
	
	//发送数据
	WirelessSend(pBuff,nLen);
}
#endif

#ifdef	_SET_SEND_CHANNEL_ID_
void SetSendChannelID(uint8 nChannelID) reentrant
{
	g_nOutChannelID = nChannelID;
}
#endif

