/****************************************Copyright (c)************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------
**文   件   名: nrf905.c
**创   建   人: 杨承凯
**创 建 日  期: 2006年6月25日
**最后修改日期: 2007年5月7日
**描        述: nrf905源文件
******************************************************************************/
#define _NRF905_C_

#include "bsp.h"
#include "spi.h"
#include "nrf905.h"

//defines---------------------------------------
#define 	NOP		_nop_();_nop_();_nop_()

//编译控制
//#define 	USE_PA

//****************定义控制字**************************************************
#define 	R_TX_ADDR	 	0x23		//读远程地址
#define  	W_TX_ADDR		0x22		//写远程地址
#define  	R_TX_DATA_F 	0x21		//读待发送数据
#define  	W_TX_DATA_F 	0x20		//写待发送数据
#define  	R_CFG_F		 	0x10		//读配置寄存器
#define  	W_CFG_F			0x00		//写配置寄存器
#define  	RECEIVE_DATA 	0x24		//读收到的数据

//*********************信号量定义*********************************************
//输出信号=====================================================
//nrf905片选
#define CSN_PORT		PORT(2)
#define CSN				BIT(5)

//上电
#define POWER_UP_PORT	PORT(2)
#define POWER_UP		BIT(1)

//发送或接收选择信号
#define TX_EN_PORT		PORT(1)
#define TX_EN			BIT(7)


//发送或接收使能信号
#define TRX_CE_PORT		PORT(2)
#define TRX_CE			BIT(0)


//功放控制引脚-------------
#ifdef USE_PA
//功放控制
#define PA_PORT			PORT(1)
#define PA				BIT(0)

//射频开关控制
#define STX_PORT		PORT(2)
#define STX				BIT(7)

void NrfDelay(uint16 nTime);
#endif

//输入信号======================================================
//载波检测
//#define CD_PORT			PORT(2)
//#define CD				BIT(1)

//地址匹配
//#define AM_PORT			PORT(3)
//#define AM				BIT(7)

//数据发送完毕或接收正确
//#define DR_PORT			PORT(2)
//#define DR				BIT(3)

#define DRINT_PORT      PORT(0)    //接收检测中断引脚
#define DRINT           BIT(1)


//数据定义*******************************************************************
#define	DEBUG_SPI	1

//控制字缓冲区
static INT8U xdata RfCfg[10];

//发送地址缓冲区
static INT8U xdata TxAddr[4];

static OS_EVENT 	*pNrf905Event;						    	//事件控制块
static uint16 		nNrf905TimeOut;								//接收超时时间
static uint8 		nNrf905Err;									//错误标志
//**********************函数定义*********************************************

/****************************************************************************
* 名	称：Nrf905PortInit()
* 功	能：nrf905端口初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _NRF905_INIT_
void Nrf905PortInit() reentrant
{
	//---------------------
	MakePushPull(CSN);
	MakePushPull(POWER_UP);
	MakePushPull(TX_EN);
	MakePushPull(TRX_CE);
	//---------------------
	//MakeOpenDrain(CD);
	//MakeOpenDrain(AM);
	//MakeOpenDrain(DR);
	//---------------------
	SetLo(POWER_UP);
	SetLo(TRX_CE);
	SetLo(TX_EN);
	SetHi(CSN);

	//功放控制
#ifdef USE_PA
	MakePushPull(PA);
	MakePushPull(STX);
	SetLo(PA);
	SetHi(STX);	
#endif	

	//中断初始化
	IT0 = 1;
	IT01CF &= 0xF0;
	IT01CF |= 0x08;
	IT01CF	|= GetBitValue(DRINT);
	MakeOpenDrain(DRINT);

}
#endif

/****************************************************************************
* 名	称：Nrf905VariInit()
* 功	能：nrf905全局变量初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _NRF905_INIT_
void Nrf905VariInit() reentrant
{
	pNrf905Event = NULL;
	nNrf905TimeOut = 0;
}
#endif

/****************************************************************************
* 名	称：Nrf905SetDcb()
* 功	能：设置设备控制参数
* 入口参数：pBrfDcb：设备控制参数
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _NRF905_INIT_
void Nrf905SetDcb(NRF905DCB *pBrfDcb)  reentrant
{
	unsigned char tmp;

	//计算频率控制字
	if(pBrfDcb->nTransFreq<4224) pBrfDcb->nTransFreq = 4224;
	else if(pBrfDcb->nTransFreq>4735) pBrfDcb->nTransFreq = 4735;
	
	tmp = ((pBrfDcb->nTransFreq-4224)>>8) & 0x00FF;
	RfCfg[1] = 0x0c;
	RfCfg[1] &= ~0x01;
	RfCfg[1] |= tmp;
	tmp = (pBrfDcb->nTransFreq-4224) & 0x00FF;
	RfCfg[0] = tmp;
	
	//配置DNS长度
	RfCfg[2] = 0x44;
	
	//配置本地DNS
	*((uint32*)(RfCfg+5)) = pBrfDcb->LocalDNS;

	//配置远程DNS
	*((uint32*)TxAddr) = pBrfDcb->RemoteDNS;

	//配置接收数据长度
	RfCfg[3] = pBrfDcb->nRxLen;

	//配置发送数据长度
	RfCfg[4] = pBrfDcb->nTxLen;

	//配置功率以及校验位
	RfCfg[9] = 0x1B;
	
	//
	//配置
	//
	nrf905_rw(W_CFG_F,RfCfg,10,0);
	nrf905_rw(W_TX_ADDR,TxAddr,4,0);

	//创建信号量
	if(pNrf905Event == NULL) 
	{
		pNrf905Event = OSSemCreate(0);
	}

#ifdef	DEBUG_SPI
	for(tmp=0;tmp<10;tmp++) RfCfg[tmp] = 0;	
	nrf905_rw(R_CFG_F,RfCfg,10,1);
#endif

}
#endif

/****************************************************************************
* 名	称：Nrf905SetRecvTimeOut()
* 功	能：设置接收超时时间
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_NRF905_SET_RECV_TIME_OUT_
void Nrf905SetRecvTimeOut(uint16 nTimeOut) reentrant
{
	if(nTimeOut == 0)
	{
		nNrf905TimeOut = 0;
		return;
	}
	nNrf905TimeOut = (nTimeOut - 1) / 10 + 1;
}
#endif


/****************************************************************************
* 名	称：OpenNrf905()
* 功	能：打开905通信
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _NRF905_INIT_
void OpenNrf905()  reentrant
{
	SetHi(POWER_UP);
}
#endif


/****************************************************************************
* 名	称：CloseNrf905()
* 功	能：关闭905通信
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _NRF905_CLOSE_
void CloseNrf905() reentrant
{
	SetLo(POWER_UP);
}
#endif

/****************************************************************************
* 名	称：nrf905_IsFresh()
* 功	能：nrf905判断信道是否空闲
* 入口参数：无
* 出口参数：指示信道是否空闲，1：空闲，2：不空闲
* 说	明：无
****************************************************************************/
#ifdef _NRF905_SET_IS_FRESH_
BOOL Nrf905IsFresh() reentrant
{
	return(!GetSignal(CD));
}
#endif

/****************************************************************************
* 名	称：nrf905_rw()
* 功	能：nrf905读写函数
* 入口参数：ctr 命令字; pd 待读写数据的指针; nd 待读写数据的长度
* 出口参数：无
* 说	明：ctr 在上文中已定义
****************************************************************************/
#ifdef _NRF905_INIT_
static void nrf905_rw(uint8 ctr, uint8 *pd, uint16 nd, uint8 bNeedRead) reentrant
{
	uint16 i;
	uint8  trst;
	
	SetLo(CSN);
	NOP;
	spi_rw(ctr);
	for(i=0;i<nd;i++)
	{
		trst = spi_rw(pd[i]);
		if(bNeedRead)
		{
			pd[i] = trst;
		}
		NOP;
	}
	SetHi(CSN);
	NOP;
}
#endif

/****************************************************************************
* 名	称：Nrf905ReadData()
* 功	能：nrf905读接收数据函数
* 入口参数：pd 待读数据的指针; nd 待读数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _NRF905_READ_
void Nrf905ReadData(uint8 *pd,uint16 nd) reentrant
{
	nrf905_rw(RECEIVE_DATA,pd,nd,1);
}
#endif

/****************************************************************************
* 名	称：Nrf905Send()
* 功	能：nrf905发送数据函数
* 入口参数：pd 待写数据的指针; nd 待写数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _NRF905_SEND_
void Nrf905Send(uint8 *pd,uint16 nd) reentrant
{
	//写发送数据到发送缓冲区
	nrf905_rw(W_TX_DATA_F,pd,nd,0);

	//开功放
#ifdef USE_PA
	SetLo(STX);
	SetHi(PA);
	NrfDelay(200);	
#endif

	//发送
	SetHi(TX_EN);
	SetHi(TRX_CE);

	//清除原有中断标志，使能中断
	IE0 = 0;
	EX0 = 1;

	//无超时等待发送完毕
	OSSemPend(pNrf905Event, 0, &nNrf905Err);
	
	//关闭发送
	SetLo(TX_EN);
	//在中断处理程序中通信被关断，这里就不需要关通信了
}
#endif

/****************************************************************************
* 名	称：Nrf905SendCarry()
* 功	能：发送载波，供测试用
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_NRF905_SEND_CARRY_
void Nrf905SendCarry(uint16 nCycles) reentrant
{
	SetHi(TX_EN);
	SetHi(TRX_CE);

	//持续发送载波	
	if(nCycles == 0) while(1);
	OSTimeDly(nCycles);

	SetLo(TX_EN);
	SetLo(TRX_CE);
}
#endif
/****************************************************************************
* 名	称：Nrf905Receive()
* 功	能：nrf905接收程序
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_NRF905_RECEIVE_
BOOL Nrf905Receive() reentrant
{
	//接收数据
	SetLo(TX_EN);
	SetHi(TRX_CE);
	
	//清除原有中断标志，使能中断
	IE0 = 0;
	EX0 = 1;

	//等待接收完毕
	OSSemPend(pNrf905Event, nNrf905TimeOut, &nNrf905Err);

	//在中断处理程序中通信被关断，这里就不需要关通信了

	//
	//注意：接收完数据后不能关闭POWER_UP，否则读出的数据全为0！
	//

	if(nNrf905Err == OS_NO_ERR) 
	{
		return 1;
	}
	return 0;
}
#endif

/*****************************************************************************************************************
* 名	称	Int0ISR()
* 功	能：发送/接收中断程序
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void Int0ISR() interrupt 0
{		 
	OSIntEnter();				//进入中断
	EX0 = 0;					//禁止外部中断0中断

	SetLo(TRX_CE);				//关闭905通信

	//关闭控制
#ifdef USE_PA
	SetLo(PA);
	SetHi(STX);						   
#endif

	OSSemPost(pNrf905Event);	//向发送或接收程序发送信号量
	OSIntExit();				//退出中断
}


/*****************************************************************************************************************
* 名	称	NrfDelay()
* 功	能：延时函数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef USE_PA
static void NrfDelay(uint16 nTime)
{
	while(nTime--);
}	
#endif
