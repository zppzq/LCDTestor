/****************************************Copyright (c)************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------
**文   件   名: WirelessInterface.h
**创   建   人: 罗仕强
**创 建 日  期: 2009年4月14日
**最后修改日期: 2009年4月14日
**描        述: 无线接口函数
**说        明: 				
******************************************************************************/
#ifndef _WIRELESS_INTERFACE_H_
#define _WIRELESS_INTERFACE_H_

#include "Si4432.h"
//#include "Nrf905.h"

//*****************************************************************
//若使用4432
#ifdef	_SI4432_H_
#if 0
typedef struct tagWIRELESS_S
{
	SI4432DCB	WIRELESSDCB;		//定义DCB
	void (* WirelessOpen)();
	void (* WirelessClose)();
	void (* WirelessReceive)();
	void (*WirelessSend)();
}WIRELESS_S;
#endif

#define	WIRELESSDCB			SI4432DCB

#define	WirelessPortInit()		Si4432PortInit()
#define	WirelessVariInit()		Si4432VariInit()
#define	WirelessOpen()			Si4432Open()
#define	WirelessClose()			Si4432Close()
#define	WirelessIdleEnter()		//Si4432IdleEnter()
#define	WirelessIdleExit()		//Si4432IdleExit()
#define	WirelessReceive()		Si4432Receive()
#define	WirelessReadData(x,y)	Si4432ReadData(x,y)
#define	WirelessSend(x,y)		Si4432Send(x,y)
#define	WirelessGetDCB()			Si4432GetDCB()
#define	WirelessSetDCB(x)		Si4432SetDCBToChip(x)
#define	WirelessSetNeedFreq(x)	Si4432SetNeedFreq(x)
#define	WirelessSendCarry(x)		Si4432SendCarry(x)
#define	WirelessSetRecvTimeOut(x)	Si4432SetRecvTimeOut(x)

#endif

#ifdef	_NRF905_H_

#define	WIRELESSDCB			NRF905DCB

#define	WirelessPortInit()		Nrf905PortInit()
#define	WirelessVariInit()		Nrf905VariInit()
#define	WirelessOpen()			OpenNrf905()
#define	WirelessClose()			CloseNrf905()
#define	WirelessReceive()		Nrf905Receive()
#define	WirelessReadData(x,y)	Nrf905ReadData(x,y)
#define	WirelessSend(x,y)		Nrf905Send(x,y)
#define	WirelessDCBInit(x)		_nop_()
#define	WirelessSetDCB(x)		Nrf905SetDcb(x)
#define	WirelessSendCarry(x)		Nrf905SendCarry(x)
#define	WirelessSetRecvTimeOut(x)	Nrf905SetRecvTimeOut(x)

#endif

//文件结束=====================================================
#endif



