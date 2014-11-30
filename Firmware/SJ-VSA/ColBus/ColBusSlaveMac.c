/*
*********************************************************************************************************
*                                               ColBus
*                                               通信协议
*
*                      (c) Copyright 2008-2010, ShenZhen SEN&GENE Technologies Co.,Ltd
*                                               版权所有
*
*
* 文件名 : ColBusSlaveMac.c
* 作者   : 杨承凯(kady1984@hotmail.com)
*********************************************************************************************************
*/
#include <string.h>
#include "ColBusDef.h"
#include "CrcCheck.h"
#include "nwk.h"
#include "ColBusSlaveMac.h"

//ColBus数据缓冲区长度
#define CB_BUFF_SEND_MAX	128
#define CB_BUFF_RECV_MAX	128

//#define FLASH_ID_MODE
#ifdef FLASH_ID_MODE
uint16 code g_FlashID _at_ 0x0400;
#endif

//ColBus地址
#define CB_ADDR_LEN			15
static const uint8 m_nAddrMacLen = 4;											//ColBus物理地址长度
static uint8 m_arrAddrMac[CB_ADDR_LEN] = {5, 1, 1, 5};							//该设备地址为0x05010101

static uint8 m_nAddrNetLen = 4;													//ColBus物理地址长度
static uint8 m_arrAddrNet[CB_ADDR_LEN] = {0, 0, 0, 0};					//初始化网络地址为空

//ColBus数据缓冲区定义
static uint8 m_buffCBSend[CB_BUFF_SEND_MAX];
static uint8 m_buffCBRecv[CB_BUFF_RECV_MAX];
static uint8 m_nCBControl = 0xC4;		//ColBus控制字

//接收的地址长度
static uint8 m_nRecvMacLen;	
static uint8 m_nRecvNetLen;	

//内部函数声明
int16 CBSlaveMacSend(uint8* pData, int16 nLen) reentrant;

//初始化
void CBMacInit() reentrant
{
#ifdef FLASH_ID_MODE
	uint16 nID = g_FlashID;
									
	m_arrAddrMac[0] = LOBYTE(nID);
	m_arrAddrMac[1] = HIBYTE(nID);	
#endif
}

//Mac状态处理
uint8 CBSlaveMacFsm() reentrant
{
	uint8* pData = m_buffCBRecv + m_nAddrMacLen + 1;
	uint8 nCmd = pData[0];

	//设置网络地址
	if(nCmd == CB_SET_ADDR)
	{
		//pData[1]为AddrLevel
		m_nAddrNetLen = pData[2];
		m_nAddrNetLen = (m_nAddrNetLen < CB_ADDR_LEN) ? m_nAddrNetLen : CB_ADDR_LEN;
		memcpy(m_arrAddrNet, pData + 3, m_nAddrNetLen);

		//回复原指令
		CBSlaveMacSend(pData, 1);

		return 1;
	}

	return 0;
}

//ColBus从设备接收函数
int16 CBSlaveNetRecv(uint8* pData, int16 nLen) reentrant
{
	int16 nRecv;
	int8 i;
	
	nLen = nLen;

	//接收网络层数据
	nRecv = NwkRecv(m_buffCBRecv, CB_BUFF_RECV_MAX);
	if(nRecv < 4) return nRecv;		//一个ColBus数据包至少包括控制字，指令和CRC，不少于4字节数据

	//获取控制器
	m_nCBControl = m_buffCBRecv[0];

	//校验包传输方向
	if((m_nCBControl & MTS_MASK) == MTS_UP) return 0;

	//校验控制信息
	if((m_nCBControl & CODM_MASK) == CODM_MAC)
	{
		//MAC层访问

		//校验地址
		m_nRecvMacLen = m_nCBControl & CB_ADDR_SIZE_MASK;
		if(m_nRecvMacLen > m_nAddrMacLen) return 0; 				//防止地址长度溢出
		for(i = 0; i < m_nRecvMacLen; i++)
		{
			if(m_buffCBRecv[1+i] != m_arrAddrMac[i]) return 0;  	//地址校验失败
		}
	
		//校验CRC
		if(CRC16(m_buffCBRecv, nRecv) != 0) return 0; //CRC校验失败
	
		//返回数据
		if(CBSlaveMacFsm() == 0)
		{
			//MAC层没有处理，让APS层处理(即允许直接地址访问到APS层)
			nRecv = nRecv - m_nAddrMacLen - 3/*控制字和CRC*/;  //计算数据和指令的长度
			memcpy(pData, m_buffCBRecv + 1 + m_nAddrMacLen, nRecv);

			return nRecv;
		}
		else
		{
			//如果MAC已处理，则不再需要网络层处理
			return 0;
		}
	}	

	if((m_nCBControl & CODM_MASK) == CODM_NET)
	{
		//网络层访问

		//校验地址
		m_nRecvNetLen = m_nCBControl & CB_ADDR_SIZE_MASK;
		if(m_nRecvNetLen > m_nAddrNetLen) return 0; //防止地址长度溢出
		for(i = 0; i < m_nRecvNetLen; i++)
		{
			if(m_buffCBRecv[1+i] != m_arrAddrNet[i]) return 0;  //地址校验失败
		}
	
		//校验CRC
		if(CRC16(m_buffCBRecv, nRecv) != 0) return 0; //CRC校验失败
	
		//返回数据
		nRecv = nRecv - m_nRecvNetLen - 3/*控制字和CRC*/;  //计算数据和指令的长度
		memcpy(pData, m_buffCBRecv + 1 + m_nRecvNetLen, nRecv);
		return nRecv;
	}

	return 0;
}

//ColBus从设备发送函数
int16 CBSlaveMacSend(uint8* pData, int16 nLen) reentrant
{
	uint16 nCrc16;
	int8 i;

	//传输类型
 	m_nCBControl &= ~CODM_MASK;
	m_nCBControl |= CODM_MAC;

	//传输方向
	m_nCBControl &= ~MTS_MASK;
	m_nCBControl |= MTS_UP;

	//地址长度
	m_nRecvMacLen = (m_nRecvMacLen < CB_ADDR_LEN) ? m_nRecvMacLen : CB_ADDR_LEN;	//防止地址长度溢出
 	m_nCBControl &= ~CB_ADDR_SIZE_MASK;
	m_nCBControl |= m_nRecvMacLen;

	//设置控制字
	m_buffCBSend[0] = m_nCBControl;
	
	//设置地址
	for(i = 0; i < m_nRecvMacLen; i++)
	{
		m_buffCBSend[1+i] = m_arrAddrMac[i];
	}

	//填充数据(注意，未检查发送区是否会溢出)
	memcpy(m_buffCBSend + 1 + m_nRecvMacLen, pData, nLen);

	//设置CRC校验字段
	nCrc16 = CRC16(m_buffCBSend, 1 + m_nRecvMacLen + nLen);
	m_buffCBSend[1 + m_nRecvMacLen + nLen] = (uint8)((nCrc16 >> 8) & 0xFF);
	m_buffCBSend[2 + m_nRecvMacLen + nLen] = (uint8)(nCrc16 & 0xFF);

	//接收网络层数据
	NwkSend(m_buffCBSend, 3/*控制字与CRC*/ + m_nRecvMacLen + nLen);

	return nLen;
}

//ColBus从设备发送函数
int16 CBSlaveNetSend(uint8* pData, int16 nLen) reentrant
{
	uint16 nCrc16;
	int8 i;

	//传输类型
 	m_nCBControl &= ~CODM_MASK;
	m_nCBControl |= CODM_NET;

	//传输方向
	m_nCBControl &= ~MTS_MASK;
	m_nCBControl |= MTS_UP;

	//地址长度
	m_nRecvNetLen = (m_nRecvNetLen < CB_ADDR_LEN) ? m_nRecvNetLen : CB_ADDR_LEN;	//防止地址长度溢出
	m_nRecvNetLen = (m_nRecvNetLen == 0) ? m_nAddrNetLen : m_nRecvNetLen;			//防止空地址，广播单选指令时

 	m_nCBControl &= ~CB_ADDR_SIZE_MASK;
	m_nCBControl |= m_nRecvNetLen;

	//设置控制字
	m_buffCBSend[0] = m_nCBControl;
	
	//设置地址
	for(i = 0; i < m_nRecvNetLen; i++)
	{
		m_buffCBSend[1+i] = m_arrAddrNet[i];
	}

	//填充数据(注意，未检查发送区是否会溢出)
	memcpy(m_buffCBSend + 1 + m_nRecvNetLen, pData, nLen);

	//设置CRC校验字段
	nCrc16 = CRC16(m_buffCBSend, 1 + m_nRecvNetLen + nLen);
	m_buffCBSend[1 + m_nRecvNetLen + nLen] = (uint8)((nCrc16 >> 8) & 0xFF);
	m_buffCBSend[2 + m_nRecvNetLen + nLen] = (uint8)(nCrc16 & 0xFF);

	//接收网络层数据
	NwkSend(m_buffCBSend, 3/*控制字与CRC*/ + m_nRecvNetLen + nLen);

	return nLen;
}

//校验网络地址
BOOL CheckAddrNet(uint8* arrAddr, uint8 nLen)
{
	uint8 i;

	if(nLen == 0) return FALSE;
	if(nLen != m_nAddrNetLen) return FALSE;

	for(i = 0; i < nLen; i++)
	{
		if(arrAddr[i] != m_arrAddrNet[i])
		{
			return FALSE;
		}
	}

	return TRUE;
}



