/****************************************Copyright (c)************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------
**文   件   名: Si4432.c
**创   建   人: 杨承凯
**创 建 日  期: 2008年12月26日
**最后修改日期: 2008年12月26日
**描        述: Si4432源文件
**说        明: 				
******************************************************************************/
#define _SI4432_C_

#include "includes.h"
#include "WirelessInterface.h"
#include "Si4432.h"
#include "Nrzi.h"

//缓冲区长度定义
#define	SI4432_RXLEN_MAX	64
#define SI4432_TXLEN_MAX	64

#define _SI4432POWER_

//********信号定义***********************************************************
#ifdef _SI4432POWER_
//关闭电源引脚，0：开电，1：关电
#define	SI4432SDN_PORT		GPIOC
#define	SI4432SDN			5
#endif //_SI4432POWER_

//SPI通信片选引脚
#define	SI4432NSEL_PORT		GPIOC
#define	SI4432NSEL			4

//中断输出引脚
#define	NIRQ_PORT			GPIOB
#define	NIRQ				0

//若将GPIO接到单片机IO口，则打开此宏
//#define	SI_GPIO_INPUT
#ifdef 	SI_GPIO_INPUT
// GPIO引脚
#define GPIO0_PORT		PORT(2)
#define GPIO0			BIT(7)
#define GPIO1_PORT		PORT(3)
#define GPIO1			BIT(0)
#define GPIO2_PORT		PORT(3)
#define GPIO2			BIT(1) 
#endif

//--------射频开关收发控制---------------------------------------------------
#define	AS179_92	1
#define	AS178_73	2
//选择所用功率开关，请特别注意
#define	PA_SWITCH_CHIP	AS179_92

#define	SELF_TRSW
// 没有定义自控制射频开关收发，则通过单片机引脚控制
#ifndef	SELF_TRSW				
#define	TRSW1_PORT			PORT(3)
#define	TRSW1				BIT(7)
#define	TRSW2_PORT			PORT(2)
#define	TRSW2				BIT(6)

// 定义了自控制射频开关收发，通过芯片GPIO控制
#else
#define	SI4432GPIO0		0
#define	SI4432GPIO1		1
#define	SI4432GPIO2		2

#define	TRSW1				SI4432_GPIO_0
#define	TRSW2				SI4432_GPIO_1
#endif


//---------------------------------------------------------------------------

//--------功放相关定义-------------------------------------------------------
//#define USE_PA			//如果使用功放，则打开此宏
#ifdef  USE_PA
//功放控制
#define PA_PORT			PORT(2)
#define PA				BIT(5)

//功放开关
#define PAOn()			SetLo(PA)	// 功放开，射频开关到功放通路	
#define PAOff()			SetHi(PA)	// 功放关，射频开关到直通路

//功放专用函数
void NrfDelay(uint16 nTime);
#endif

//#define	USE_NRF_LED
#ifdef USE_NRF_LED
//指示灯
#define NRFLED_PORT			PORT(2)
#define NRFLED				BIT(3)

#define NrfLedOn()			SetLo(NRFLED)
#define NrfLedOff()			SetHi(NRFLED)
#endif
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
//输出信号=================================================================
//时钟信号                                                                                                                                                                                                                                                                                           
#define SI4432_SCK_PORT	SPI1_GPIO
#define SI4432_SCK			5

//输入信号==================================================================
//数据输入
#define SI4432_MISO_PORT	SPI1_GPIO
#define SI4432_MISO		6

//数据输出
#define SI4432_MOSI_PORT	SPI1_GPIO
#define SI4432_MOSI		7

#define BREAD 		1								//读		
#define BWRITE		0								//写  

#ifndef	NOP
#define NOP			_nop_();_nop_();_nop_()	 		//小延时
#endif
#ifdef _F930_H_
#define Si4432IntEnalbe()		IE0 = 0; EX0 = 1	//开4432连接到单片机的外部中断
#define Si4432IntDisable()		EX0 = 0				//关中断
#else
void Si4432IntEnalbe(void);							//开4432连接到单片机的外部中断
void Si4432IntDisable(void);						//关中断
#endif	//_F930_H_

//编译控制
#define USE_NRZI_4432
#define USE_NRZI_4432_ON_COLLECTOR
 
//********数据定义***********************************************************
//系统需要信号量变量
static OS_EVENT		*pSi4432Event;		   				//事件控制块
static uint8 		nSi4432Err;							//错误标志
static uint16 		nSi4432TimeOut;						//接收超时时间
static SI4432DCB 	g_sSI4432DCB;						//记录SI4432DCB参数 	
static uint8 		nFixPkLen; 							//固定包长度否，>0：包长度，0：非固定包长度
static uint16 		nIntStatus16;						//中断标志
static uint8 		pSi4432SendBuff[80];				//发送缓冲区
static uint8 		pSi4432RecvBuff[80];				//接收缓冲区
//****************************************************************************************************************
//********函数定义************************************************************************************************
//****************************************************************************************************************


void SI4432_IT_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource0);	  	
}

void Si4432IntDisable(void)
{
	EXTI_InitTypeDef EXTI_InitStructure; 
	EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
	EXTI_InitStructure.EXTI_LineCmd = DISABLE; 
	EXTI_Init(&EXTI_InitStructure);
}

void Si4432IntEnalbe(void)
{
	EXTI_InitTypeDef EXTI_InitStructure; 
	EXTI_InitStructure.EXTI_Line = EXTI_Line0; 
	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt; 
	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling; 
	EXTI_InitStructure.EXTI_LineCmd = ENABLE; 
	EXTI_Init(&EXTI_InitStructure);
}

/****************************************************************************
* 名	称：Si4432PortInit()
* 功	能：Si4432端口初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_INIT_
void Si4432PortInit() reentrant
{
	//设置端口输入输出
#ifdef _SI4432POWER_
	MakePushPull(SI4432SDN);			//关电控制输出
#endif //_SI4432POWER_

//	MakePushPull(SI4432NSEL);			//SPI通信片选输出
	SPI1_Configuration();

#ifdef SI_GPIO_INPUT
	MakeOpenDrain(GPIO0);
	MakeOpenDrain(GPIO1);
	MakeOpenDrain(GPIO2);
#endif
	
	//设置初始端口状态
#ifdef _SI4432POWER_
	SetHi(SI4432SDN);					//初始关电
#endif //_SI4432POWER_
	SetHi(SI4432NSEL);					//片选信号置高

	//中断初始化
#ifdef _F930_H_
	IT0 = 1;				 			//边沿触发
	IT01CF &= 0xF0;						//清除中断0所有标志
	IT01CF |= 0x00;						//低电平有效
	IT01CF |= GetBitValue(NIRQ);		//配置引脚
	MakeOpenDrain(NIRQ);
#else
	SI4432_IT_Configuration();
#endif
	
	//功放相关
#ifdef USE_PA
	MakePushPull(PA);
	PAOff();							//关功放
#endif

#ifdef USE_NRF_LED
	MakePushPull(NRFLED);
	NrfLedOff();
#endif

}
#endif

//端口引脚控制
void Si4432PortShut() reentrant
{
	//MakeOpenDrain(SI4432NSEL);			//SPI通信片选输出
	//SetLo(SI4432NSEL);					//SPI通信片选输出
}

void Si4432PortOpen() reentrant
{
	//SetHi(SI4432NSEL);					//SPI通信片选输出
	//MakePushPull(SI4432NSEL);			//SPI通信片选输出
}

/****************************************************************************
* 名	称：Si4432VariInit()
* 功	能：Si4432全局变量初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_INIT_
void Si4432VariInit() reentrant
{
	pSi4432Event = NULL;
	nSi4432TimeOut = 0;

	//基本参数
	g_sSI4432DCB.nFreq 			= 4420;								//Si4432频率
	g_sSI4432DCB.nFreqStep 		= 50;								//Si4432频率步进
	g_sSI4432DCB.nChannel 		= 0;								//Si4432频率通道
	g_sSI4432DCB.nFreqDeviation = 50;								//Si4432频率偏离值：625Hz*200=125kHz
	g_sSI4432DCB.nDataRate 		= 100;								//Si4432通信速率
	g_sSI4432DCB.nOutPower		= 20;								//输出功率
	g_sSI4432DCB.nModulateType 	= MT_GFSK;							//调制方式
	g_sSI4432DCB.nModDataSource = MDS_FIFO;							//调制数据源
	g_sSI4432DCB.bManCheEn		= FALSE;							//是否使能曼彻斯特编码，注意：使能曼彻斯特编码，有效数据率将减半
	g_sSI4432DCB.bVCOEn			= TRUE;								//是否每次校准VCO

	//包处理结构
	g_sSI4432DCB.sPacketHandler.bPHEn 			= TRUE;				//是否允许包处理
	g_sSI4432DCB.sPacketHandler.bCRCEn 		 	= FALSE;			//是否允许CRC校验
	g_sSI4432DCB.sPacketHandler.nBroad			= 0;				//广播地址个数
	g_sSI4432DCB.sPacketHandler.nPreLen 		= 10;				//前导码长度
	g_sSI4432DCB.sPacketHandler.nPreDectLen	 	= 5;				//前导码检测门限
	g_sSI4432DCB.sPacketHandler.nSysWordLen 	= 4;				//同步字个数
	g_sSI4432DCB.sPacketHandler.SysWord 		= 0x7E2DD4E7;		//同步字
	g_sSI4432DCB.sPacketHandler.nTxHeaderLen 	= 0;				//发送数据头长度
	g_sSI4432DCB.sPacketHandler.TxHeader 		= 0x66666666;		//发送数据头
	g_sSI4432DCB.sPacketHandler.nRxHeaderCheck  = 0;				//接收数据头校验个数
	g_sSI4432DCB.sPacketHandler.RxHeader 		= 0x66666666;		//接收校验用数据头
	g_sSI4432DCB.sPacketHandler.RxHeaderCheckEn = 0x00000000;		//接收校验用数据头使能位控制
	g_sSI4432DCB.sPacketHandler.bFixPkLen		= FALSE;			//是否使用固定包长度
	g_sSI4432DCB.sPacketHandler.TrxPkLen		= 0;				//发送接收包长度
	
	//接收Modem结构
	g_sSI4432DCB.sRxModem.bAFCEn	  	= TRUE;						//是否使能AFC
	g_sSI4432DCB.sRxModem.dwn3_bypass 	= 1;						//dwn3_bypass Bypass Decimator by 3 (if set). 
	g_sSI4432DCB.sRxModem.ndec_exp    	= 0;						//ndec_exp[2:0]：IF Filter Decimation Rates.
	g_sSI4432DCB.sRxModem.filset 	  	= 0x0F;						//filset[3:0]：  IF Filter Coefficient Sets.
	g_sSI4432DCB.sRxModem.rxosr	   	  	= 0x0078;					//rxosr[10:0]：  Oversampling Rate.
	g_sSI4432DCB.sRxModem.ncoff	   		= 0x00011111;				//ncoff[19:0]：  NCO Offset.
	g_sSI4432DCB.sRxModem.crgain	   	= 0x0446;					//crgain[10:0]： Clock Recovery Timing Loop Gain.
}
#endif

/****************************************************************************
* 名	称：Si4432GetDCB()
* 功	能：获取设备控制参数
* 入口参数：pBrfDcb：设备控制参数
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_INIT_
SI4432DCB* Si4432GetDCB() reentrant
{
	return &g_sSI4432DCB;
}
#endif

/****************************************************************************
* 名	称：Si4432SetDCBToChip()
* 功	能：设置参数到芯片
* 入口参数：pBrfDcb：设备控制参数
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_INIT_
void Si4432SetDCBToChip(SI4432DCB *pBrfDcb)  reentrant
{
	uint8 tmp;
	uint16 nIntStatus;

	//清除所有中断使能
	nIntStatus = 0x0000;
	nIntStatus = htons(nIntStatus);
	Si4432_rw(SI4432_INT_EN_1, (uint8 *)(&nIntStatus), 2, BWRITE);

	//设置通信频率即载波频率
	Si4432SetFHSS(pBrfDcb->nFreq, pBrfDcb->nFreqStep, pBrfDcb->nChannel);

	//设置频率偏移
	Si4432SetFreqDeviation(pBrfDcb->nFreqDeviation);
	
	//设置数据通信率
	Si4432SetDataRate(pBrfDcb->nDataRate);

	//设置输出功率
	Si4432SetOutPower(pBrfDcb->nOutPower);

	//设置调制方式
	Si4432SetModulation(pBrfDcb->nModulateType, pBrfDcb->nModDataSource, pBrfDcb->bManCheEn);	
	
	//设置接收器Modem，关于FSK,GFSK
	Si4432SetRXModem(&(pBrfDcb->sRxModem));

	//设置包处理
	Si4432SetPacketHandler(&(pBrfDcb->sPacketHandler));
	
	//设置发送接收时序
	Si4432SetSysTiming(pBrfDcb->bVCOEn, 10, 2);	

	//其他附加设置
	Si4432SetRSSIGate(0xFF);

	//Excel生成值
	tmp = 0x0B;	  											//Set this value for optimal performance!
	Si4432_rw(SI4432_AGC_OVERRIDE_2, &tmp, 1, BWRITE);		//Register 6Ah. AGC Override 2
	
	//记录包长度		
	if(pBrfDcb->sPacketHandler.bFixPkLen == TRUE)					// 如果是固定包长度
	{
		nFixPkLen = pBrfDcb->sPacketHandler.TrxPkLen;	
	}
	else
	{
		nFixPkLen = 0;
	}

	//使能基本的中断(发送完，接收完，CRC校验出错)
	nIntStatus = IS_PACKET_SENT | IS_VALID_PACKET_RX | IS_CRC_ERROR;
	nIntStatus = htons(nIntStatus);
	Si4432_rw(SI4432_INT_EN_1, (uint8 *)(&(nIntStatus)), 2, BWRITE);
}
#endif


/****************************************************************************
* 名	称：Si4432Reset()
* 功	能：软件复位，所有寄存器恢复默认值
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_RESET_
void Si4432Reset(void) reentrant
{
	uint8 tmp;
	
	Si4432_rw(SI4432_MODE_CTRL_1, &tmp, 1, BREAD);			// Register 07h. Operating Mode and Function Control 1
	tmp |= 0x80;
	Si4432_rw(SI4432_MODE_CTRL_1, &tmp, 1, BWRITE);
}
#endif


/****************************************************************************
* 名	称：Si4432TRSWSend()
* 功	能：Si4432发送开关控制
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
static void  Si4432TRSWSend() 
{
#ifdef SELF_TRSW
	uint8 tmp;
#endif

//179-92
#if	(PA_SWITCH_CHIP == AS179_92)
	
	//用单片机控制功率开关
#ifndef	SELF_TRSW				
	SetLo(TRSW1);
	SetHi(TRSW2);

	//用SI4432自己的IO口控制
#else
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BREAD);
	tmp &= (0xFE << (TRSW1 - 0x0B));	//置低TRSW1
	tmp |= (0x01 << (TRSW2 - 0x0B));	//置高TRSW2
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BWRITE);
#endif

//178-73
#elif (PA_SWITCH_CHIP == AS178_73)

	//用单片机控制功率开关
#ifndef	SELF_TRSW				
	SetHi(TRSW1);
	SetLo(TRSW2);

	//用SI4432自己的IO口控制
#else
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BREAD);
	tmp |= (0x01 << (TRSW1 - 0x0B));	//置高TRSW1
	tmp &= (0xFF << (TRSW2 - 0x0B));	//置低TRSW2
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BWRITE);
#endif
#endif
}

/****************************************************************************
* 名	称：Si4432TRSWReceive()
* 功	能：Si4432发送开关控制
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
static void  Si4432TRSWReceive()
{
#ifdef SELF_TRSW
	uint8 tmp;
#endif

// 179-92
#if	(PA_SWITCH_CHIP == AS179_92)
	//用单片机控制功率开关
	#ifndef	SELF_TRSW				
	SetHi(TRSW1);
	SetLo(TRSW2);
	//用SI4432自己的IO口控制
	#else
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BREAD);
	tmp |= (0x01 << (TRSW1 - 0x0B));	//置高TRSW1
	tmp &= (0xFF << (TRSW2 - 0x0B));	//置低TRSW2
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BWRITE);
	#endif
// 178-73
#elif (PA_SWITCH_CHIP == AS178_73)
	//用单片机控制功率开关
	#ifndef	SELF_TRSW				
	SetLo(TRSW1);
	SetHi(TRSW2);
	//用SI4432自己的IO口控制
	#else
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BREAD);
	tmp &= (0xFE << (TRSW1 - 0x0B));	//置低TRSW1
	tmp |= (0x01 << (TRSW2 - 0x0B));	//置高TRSW2
	Si4432_rw(SI4432_IO_PORT_CFG,&tmp,1,BWRITE);
	#endif
#endif
}

/****************************************************************************
* 名	称：Si4432GetIntStatus()
* 功	能：读中断状态寄存器：INTERRUPT_STATUS_1 INTERRUPT_STATUS_2
* 入口参数：无
* 出口参数：两个寄存器值，第一寄存器占高字节，第二寄存器占低字节
* 说	明：无
****************************************************************************/
#ifdef _SI4432_GET_INT_STATUS_
uint16 Si4432GetIntStatus(void)	reentrant
{
	uint16 status;
	
	Si4432_rw(SI4432_INT_STATUS_1, (uint8 *)(&status), 2, BREAD);  	// 读2个

	status = htons(status);
	return status;
}
#endif

/****************************************************************************
* 名	称：Si4432GetDevStatus()
* 功	能：读设备状态
* 入口参数：无
* 出口参数：寄存器值
* 说	明：无
****************************************************************************/
#ifdef _SI4432_GET_DEV_STATUS_
uint8 Si4432GetDevStatus(void)	reentrant
{
	uint8 status;

	Si4432_rw(SI4432_DEVICE_STATUS, &status, 1, BREAD);		// Register 02h. Device Status

	return status;
}
#endif

/****************************************************************************
* 名	称：Si4432GetEZMacStatus()
* 功	能：获取EzMac状态
* 入口参数：无
* 出口参数：无
* 说	明：BIT[5]：正在搜索包，正在接收包，收到有效包，
	        CRC出错，正在发送包，包发送完毕
****************************************************************************/
#ifdef _SI4432_GET_EZMAC_STATUS_
uint8 Si4432GetEZMacStatus(void) reentrant
{
	uint8 tmp;
	
	Si4432_rw(SI4432_EZMAC_STATUS, &tmp, 1, BREAD);		// Address: 31h—EZMac Status, Read Only
	
	return tmp;
}
#endif

/****************************************************************************
* 名	称：Si4432GetPowerStatus()
* 功	能：读电源状态
* 入口参数：无
* 出口参数：寄存器值
* 说	明：无
****************************************************************************/
#ifdef _SI4432_GET_POWER_STATUS_
uint8 Si4432GetPowerStatus(void) reentrant
{
	uint8 status;
	
	Si4432_rw(SI4432_PW_STATUS, &status, 1, BREAD);		// Register 62h. Crystal Oscillator/Power-on-Reset Control
		
	return status;
}
#endif

/****************************************************************************
* 名	称：Si4432SetMode()
* 功	能：设置工作模式
* 入口参数：需要进入的模式
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_MODE_
void Si4432SetMode(uint8 nMode) reentrant
{
	Si4432_rw(SI4432_MODE_CTRL_1, &nMode, 1, BWRITE);		// Register 07h. Operating Mode and Function Control 1
}
#endif

/****************************************************************************
* 名	称：Si4432SetFreq()
* 功	能：设置基本频率
* 入口参数：nFreq：需要设置的频率值，单位：0.1MHz
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_FREQUENCY_
void Si4432SetFreq(uint16 nFreq) reentrant
{
	uint8 tmp;
	uint8 hbsel, fb;
	uint16 fc;
	float fFreqTemp;
	
	fFreqTemp = (float)(nFreq / 10.0f);
	
	//判断频率是否超出范围
	fFreqTemp = (fFreqTemp >= 240)? fFreqTemp : 240;
	fFreqTemp = (fFreqTemp <= 930)? fFreqTemp : 930;

	//判断是在高频率段还是在低频率段
	if(fFreqTemp < 480)
	{
		hbsel = 0;
		fb = (fFreqTemp - 240)/10.0;
	}
	else
	{
		hbsel = 1;
		fb = (fFreqTemp - 480)/20.0;
	}
	tmp = hbsel << 5;
	tmp	+= fb;
	
	tmp |= 0x40;		//sbsel，不知道什么用

	//写频率带宽选择寄存器	
	Si4432_rw(SI4432_FREQ_BAND_SEL, &tmp, 1, BWRITE);			//Register 75h. Frequency Band Select


	//计算小数段
	fFreqTemp /= (hbsel +1);
	fFreqTemp /= 10;
	fFreqTemp = fFreqTemp - fb - 24;	
	fFreqTemp *= 64000;
	fc = fFreqTemp;			// 计算频率寄存器值

	//写频率值
	fc = htons(fc);
	Si4432_rw(SI4432_NOM_CAR_FREQ, (uint8 *)(&fc), 2, BWRITE);	//Register 76h. Nominal Carrier Frequency
}
#endif

/****************************************************************************
* 名	称：Si4432SetFHSS()
* 功	能：FHSS方式设置频率
* 入口参数：nBaseFreq：基频，单位：0.1MHz；nStep：通道频率步进，单位：10KHz；nCh：通道号
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_FHSS_
void Si4432SetFHSS(uint16 nBaseFreq, uint8 nStep, uint8 nCh) reentrant
{
	//设置基本频率
	Si4432SetFreq(nBaseFreq);
		
	//设置频率步进
	Si4432_rw(SI4432_FREQ_STEP_SIZE, &nStep, 1, BWRITE);		//Register 7Ah. Frequency Hopping Step Size

	//设置频率通道
	Si4432_rw(SI4432_FREQ_CH_SEL, &nCh, 1, BWRITE);				//Register 79h. Frequency Hopping Channel Select
}
#endif

/****************************************************************************
* 名	称：Si4432SetFreqDeviation()
* 功	能：设置调制频率偏离值
* 入口参数：nFD：偏移值，单位：625Hz
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_FREQUENCY_DEVIATION_
void Si4432SetFreqDeviation(uint16 nFD) reentrant
{
	uint32 nFdForCount;
	uint8 tmp;
	uint8 fd;

	//判断输入值是否超出最大范围
	nFD = (nFD >= 1)? nFD : 1;
	nFD = (nFD <= 320)? nFD : 320;

	//换算成寄存器数值
	nFdForCount = (uint32)nFD;								//计算时nFD字长不够
	nFdForCount = (nFdForCount * 1000) / 625;				//计算
	nFD = (uint16)nFdForCount;								//转换回uint16

	//最高位
	fd = nFD >> 8;

	//fd[8:0]共9位，写BIT[8]
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BREAD);			//Register 71h. Modulation Mode Control 2
	if(fd > 0)	tmp |= 0x04;								//BIT[2]
	else 		tmp &= ~0x04;
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BWRITE);			

	//fd[8:0]共9位，写BIT[7:0]
	tmp = nFD & 0x00FF;
	Si4432_rw(SI4432_FREQ_DEVIATION, &tmp, 1, BWRITE);		//Register 72h. Frequency Deviation
}
#endif

/****************************************************************************
* 名	称：Si4432SetDataRate()
* 功	能：设置数据通信率
* 入口参数：ndr数据通信率，1-128，单位：kbps
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_DATA_RATE_
void Si4432SetDataRate(uint8 ndr) reentrant
{
	uint8 tmp,r;
	uint16 txdr;
	uint32 n;

	ndr = (ndr >= 1)? ndr : 1;
	ndr = (ndr <= 128)? ndr : 128;

	//判断数据率是否小于30kbps如果小则将指定位致置1，否则清0
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BREAD);	// Register 70h. Modulation Mode Control 1
	
	//修改值
	if(ndr < 30)	
	{
		tmp |= 0x20;				// BIT[5]								
		r = 1;	
	}
	else			
	{
		tmp &= ~0x20;
		r = 0;		
	}
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BWRITE);	// Register 70h. Modulation Mode Control 1

	//计算寄存器的值
	n = 1;
	n = n<<(16+5*r);
	n *= ndr;
	n /= 1000;
	txdr = n;

	//设置通信速率	
	txdr = htons(txdr);
	Si4432_rw(SI4432_TX_DATA_RATE, (uint8 *)(&txdr), 2, BWRITE); 	// Register 6Eh-6Fh. TX Data Rate 1 - 0
}
#endif

/****************************************************************************
* 名	称：Si4432SetOutPower()
* 功	能：设置输出功率
* 入口参数：设置的输出功率值：11、14、17、20
* 出口参数：无
* 说	明：入口参数：0<=(11)<14，14<=(14)<17，17<=(17)<20，20==(20)
****************************************************************************/
#ifdef _SI4432_SET_OUT_POWER_
void Si4432SetOutPower(uint8 ndbm)	reentrant
{
	if((20 < ndbm) || (11 > ndbm)) ndbm = 20;		//其他值，默认设定为20dbm
	// 规范输出功率值，只能是11、14、17、20这4个值
	ndbm = (ndbm - 11) / 3;

	// 设置功率	
	Si4432_rw(SI4432_TX_POWER, &ndbm, 1, BWRITE);  	// Register 6Dh. TX Power
}
#endif

/****************************************************************************
* 名	称：Si4432SetModulation()
* 功	能：设置调制模式
* 入口参数：nMT：调制类型，nMDS：调制数据源，
* 出口参数：无
* 说	明：
****************************************************************************/
#ifdef _SI4432_MODULATION_
void Si4432SetModulation(uint8 nMT, uint8 nMDS, BOOL bManChe) reentrant
{
	uint8 tmp;

	//调制模式和调制数据源控制
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BREAD);			// Register 71h. Modulation Mode Control 2
	tmp = (tmp & 0xFC) + (nMT & 0x03);
	tmp = (tmp & 0xCF) + (nMDS & 0x30);
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BWRITE);

	//关于曼彻斯特编码，保证前导码为：10101010......
	tmp = 0x0A;
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BWRITE); 		// Register 70h. Modulation Mode Control 1

	//曼彻斯特编码控制
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BREAD);			// Register 70h. Modulation Mode Control 1
	if(bManChe == TRUE)	tmp |= 0x02;						// BIT[1]
	else				tmp &= ~0x02;
	Si4432_rw(SI4432_MODUL_CTRL_1, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* 名	称：Si4432SetPacketHandler()
* 功	能：设置包处理
* 入口参数：无
* 出口参数：无
* 说	明：设置30h-4Bh寄存器。
			uint8	bPHEn;					// 是否允许包处理
			uint8  	bCRCEn;					// 是否允许CRC校验
			uint8 	nBroad;					// 广播地址个数
			uint8	nRxHeaderCheck;			// 接收数据头校验个数
			uint8 	nSysWordLen;			// 同步字个数
			uint8	nTxHeaderLen;			// 发送数据头长度
			uint16  nPreLen;				// 前导码长度
			uint8	nPreDectLen;			// 前导码检测门限
			uint32	SysWord;				// 同步字
			uint32	TxHeader;				// 发送数据头
			uint32 	RxHeader;				// 接收校验用数据头
			uint32 	RxHeaderCheckEn;		// 接收校验用数据头使能位控制
			uint8	bFixPkLen;				// 是否使用固定包长度
			uint8	TrxPkLen;				// 发送接收包长度
包结构：Preamble -> Sync Word -> TX Header -> Packet Length -> Data -> CRC
		(1-512B)	( 1-4B  )	 ( 0-4B  )	  (   0or1B   )	   (***B)  (0or2B)
旧资料
Recommended preamble length: (参看Table 49. Minimum Receiver settling time)
Mode 					Arst		Rpl when pdt = 8b 		Rpl when pdt = 16b
(G)FSK AFC Disabled 	1 byte 		2 byte 					3 byte
(G)FSK AFC Enabled 		2 byte 		3 byte 					4 byte
新资料
Recommended preamble length: (参看Table 15. Minimum Receiver Settling Time)
Mode 					Arst		Rpl when pdt = 8b 		Rpl when pdt = 20b
(G)FSK AFC Disabled 	1 byte 		20 bits 					32 bits
(G)FSK AFC Enabled 		2 byte 		28 bits 					40 bits
****************************************************************************/
#ifdef _SI4432_SET_PACKET_HANDLER_
void Si4432SetPacketHandler(SI4432PH *pSi4432PH) reentrant
{
	uint8 tmp;
	uint32 tmp32;

	pSi4432PH = pSi4432PH;
	
	//设置包处理、CRC等
	tmp = 0x00;												//初始值：不允许接收包处理，不使用LSB在前，不允许发送包处理，禁止CRC
	if(pSi4432PH->bPHEn == TRUE)	tmp = 0x88;				//允许包处理
	if(pSi4432PH->bCRCEn == TRUE)	tmp |= 0x05;			//允许CRC
	Si4432_rw(SI4432_DATA_ACCESS_CTRL, &tmp, 1, BWRITE);	//Address 30h—Data Access Control
															//Address: 31h—EZMac Status, Read Only
	//设置广播地址和数据头校验	
	tmp = (pSi4432PH->nRxHeaderCheck) & 0x0F;				//接收数据头校验
	tmp += (pSi4432PH->nBroad<<4) & 0xF0;	   				//广播地址个数
	Si4432_rw(SI4432_HEADER_CTRL_1, &tmp, 1, BWRITE);		//Address: 32h—Header Control 1

	//设置同步字、发送数据头长度、是否固定包长度、前导码长度最高位
	tmp = ((pSi4432PH->nSysWordLen-1) << 1) & 0x06;			//同步字长度
	tmp += (pSi4432PH->nTxHeaderLen << 4) & 0xE0;			//发送头长度
	if(pSi4432PH->bFixPkLen)	tmp |= 0x08;				//固定包长度
	if(pSi4432PH->nPreLen >255)	tmp |= 0x01;				//前导码长度的最高位：prealen[8]
	Si4432_rw(SI4432_HEADER_CTRL_2, &tmp, 1, BWRITE);		//Address: 33h—Header Control 2

	//设置前导码长度：前导码长度的低8位：prealen[7:0]，prealen[8]需要在33h中设置		
	tmp = (pSi4432PH->nPreLen) & 0x00FF;			
	Si4432_rw(SI4432_PREAMBLE_LEN, &tmp, 1, BWRITE);								//Address: 34h—Preamble Length 				

	//设置前导码检测门限值		
	tmp = ((pSi4432PH->nPreDectLen) << 3) & 0x00F8;	
	Si4432_rw(SI4432_PREAMBLE_DET_CTRL, &tmp, 1, BWRITE);							//Address: 35h—Preamble Detection Control 1

	//设置同步字内容
	tmp32 = htonl(pSi4432PH->SysWord);
	Si4432_rw(SI4432_SYNC_WORD, (uint8 *)(&(tmp32)), 4, BWRITE);		//Address: 36h-39h — Synchronization Word 3-0

	//设置发送数据头
	tmp32 = htonl(pSi4432PH->TxHeader);
	Si4432_rw(SI4432_TX_HEADER, (uint8 *)(&(tmp32)), 4, BWRITE);		//Address: 3Ah-3Dh — Transmit Header 3-0
	
	//设置发送数据包长度，如果使用固定包长度，则发送和接收都使用该长度，否则只是发送长度
	Si4432_rw(SI4432_TX_LEN, &(pSi4432PH->TrxPkLen), 1, BWRITE);					//Address: 3Eh—Transmit Packet Length

	//设置本地数据头
	tmp32 = htonl(pSi4432PH->RxHeader);
	Si4432_rw(SI4432_CHECK_HEADER, (uint8 *)(&(tmp32)), 4, BWRITE);	//Address: 3Fh-42h — Check Header 3-0
	
	//设计接收数据头校验屏蔽位
	tmp32 = htonl(pSi4432PH->RxHeaderCheckEn);
	Si4432_rw(SI4432_HEADER_EN, (uint8 *)(&(tmp32)), 4, BWRITE);	//Address: 43h-46h — Header Enable 3-0
																			// Address: 47h-4Ah — Received Header 3-0, Read Only
																			// Address: 4Bh—Received Packet Length, Read Only. Note: 总表（142）上的说明才正确
}
#endif

/****************************************************************************
* 名	称：Si4432SetRXModem()
* 功	能：设置接收Modem
* 入口参数：无
* 出口参数：无
* 说	明：设置1Ch-25h寄存器。
			uint8	bAFCEn;					// 是否使能AFC
			// 以下各变量值可由附带excel表计算所得，可以手工计算，但有一参数未知
			uint8 	dwn3_bypass;			// dwn3_bypass Bypass Decimator by 3 (if set). 
			uint8	ndec_exp;				// ndec_exp[2:0]：IF Filter Decimation Rates.
			uint8	filset;					// filset[3:0]：  IF Filter Coefficient Sets.
			uint16	rxosr;		 			// rxosr[10:0]：  Oversampling Rate.
			uint32	ncoff;					// ncoff[19:0]：  NCO Offset.
			uint16	crgain;					// crgain[10:0]： Clock Recovery Timing Loop Gain.
****************************************************************************/
#ifdef _SI4432_SET_RX_MODEM_
void Si4432SetRXModem(SI4432RM *pSi4432RM) reentrant
{
	uint8 tmp;
	uint8 tmp2;
	uint16 rxosr;
	uint32 ncoff;
	uint16 crgain;

	pSi4432RM = pSi4432RM;

	//设置中频滤波
	tmp  = ((pSi4432RM->dwn3_bypass) << 7) & 0x80;
	tmp += ((pSi4432RM->ndec_exp) << 4) & 0x70;
	tmp += (pSi4432RM->filset) & 0x0F;
	Si4432_rw(SI4432_IF_FILTER_BW, &tmp, 1, BWRITE);		// Register 1Ch. IF Filter Bandwidth：//dwn3_bypass, ndec_exp[2:0], filset[3:0]

	//设置AFC环路	
	Si4432_rw(SI4432_AFC_LOOP_GEAR, &tmp, 1, BREAD); 		// Register 1Dh. AFC Loop Gearshift Override	NOTE: 使用默认值
	if(pSi4432RM->bAFCEn == TRUE)	tmp |= 0x40;			//使能AFC
	else							tmp &= ~0x40;			//禁止AFC
	Si4432_rw(SI4432_AFC_LOOP_GEAR, &tmp, 1, BWRITE);

	// Register 1Eh. AFC Timing Control  				NOTE: 使用默认值
	//tmp = 0x08;
	//Si4432_rw(SI4432_AFC_TIMING_CTRL, &tmp, 1, BWRITE);

	// Register 1Fh. Clock Recovery Gearshift Override	NOTE: 使用默认值
	//tmp = 0x05;
	//Si4432_rw(SI4432_CLK_REC_GEAR, &tmp, 1, BWRITE); 

	//设置过采样
	rxosr = pSi4432RM->rxosr;
	tmp = rxosr & 0x00FF;
	Si4432_rw(SI4432_OVERSP_RATE, &tmp, 1, BWRITE);				//Register 20h. Clock Recovery Oversampling Rate
	
	//设置时间恢复偏移	
	tmp2 = rxosr >> 8;
	tmp2 = tmp2 << 5;							
	Si4432_rw(SI4432_CLK_REC_OFFSET_2, &tmp, 1, BREAD);			//Register 21h. Clock Recovery Offset 2
	tmp	= (tmp & 0x1F) + (tmp2 & 0xE0);

	ncoff = pSi4432RM->ncoff;
	tmp2 = ncoff >> 16;
	tmp = (tmp & 0xF0) + (tmp2 & 0x0F);
	Si4432_rw(SI4432_CLK_REC_OFFSET_2, &tmp, 1, BWRITE);

	tmp = ncoff >> 8;
	Si4432_rw(SI4432_CLK_REC_OFFSET_1, &tmp, 1, BWRITE);		//Register 22h. Clock Recovery Offset 1

	tmp = ncoff & 0x000000FF;
	Si4432_rw(SI4432_CLK_REC_OFFSET_0, &tmp, 1, BWRITE);			// Register 23h. Clock Recovery Offset 0
	
	//设置。。。。
	crgain = pSi4432RM->crgain;
	tmp = crgain >> 8;
	tmp &= 0x07;
	Si4432_rw(SI4432_CLK_REC_GAIN_1, &tmp, 1, BWRITE); 			// Register 24h. Clock Recovery Timing Loop Gain 1

	tmp = crgain & 0x00FF;
	Si4432_rw(SI4432_CLK_REC_GAIN_0, &tmp, 1, BWRITE);			// Register 25h. Clock Recovery Timing Loop Gain 0
}
#endif

/****************************************************************************
* 名	称：Si4432SetSysTiming()
* 功	能：设置发送接收时序
* 入口参数：bEnVCO: 是否允许VCO，nPLLTS: PLL Soft Settling Time，单位：10us, nPLLTO: PLL Settling Time，单位：10us
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_SYS_TIMING_
void Si4432SetSysTiming(uint8 bEnVCO, uint8 nPLLTS, uint8 nPLLTO) reentrant
{
	uint8 tmp;
	
	//发送是否校准VCO
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BREAD);   	//Register 55h. Calibration Control
	if(bEnVCO == TRUE)	tmp &= ~0x01; 					//BIT[0], 使用校准
	else				tmp |=  0x01;					//BIT[0], 不使用校准
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BWRITE);	
	
	//临时
	nPLLTS = nPLLTS;
	nPLLTO = nPLLTO;
	tmp = 0xA2;
	Si4432_rw(0x53, &tmp, 1, BWRITE);

/*
	//PLL 时序设置
	//Register 53h. PLL Tune Time
	tmp = nPLLTS << 3;
	tmp = (tmp & 0xF8) + (nPLLTO & 0x07);
	Si4432_rw(SI4432_PLL_TUNE_TIME, &tmp, 1, BWRITE);		//0x53

	//Register 52h. TX Ramp Control 	NOTE: 需要设置
	tmp = 20;							// 默认：8us-5us-5us
	Si4432_rw(SI4432_TX_RAMP_CTRL, &tmp, 1, BWRITE);		//0x52
*/
}
#endif

/****************************************************************************
* 名	称：Si4432SetRecvTimeOut()
* 功	能：设置接收超时时间
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_RECV_TIME_OUT_
void Si4432SetRecvTimeOut(uint16 nTimeOut) reentrant
{
	if(nTimeOut == 0)
	{
		nSi4432TimeOut = 0;
		return;
	}
	nSi4432TimeOut = (nTimeOut - 1) / 10 + 1;
}
#endif

/****************************************************************************
* 名	称：Si4432SetTX()
* 功	能：设置发送
* 入口参数：无
* 出口参数：无
* 说	明：设置是否自动发送，是否自动重发，清除TX FIFO数据
****************************************************************************/
#ifdef _SI4432_SET_TX_
void Si4432EnableTxInt(BOOL nEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_1, &tmp, 1, BREAD);				//Register 05h. Interrupt Enable 1
	
	if(nEnable == TRUE)	 tmp |= 0x04;
	else				 tmp &= ~0x04;

	Si4432_rw(SI4432_INT_EN_1, &tmp, 1, BWRITE);			  
}
#endif

/****************************************************************************
* 名	称：Si4432SetRX()
* 功	能：设置接收
* 入口参数：无
* 出口参数：无
* 说	明：设置是否接收多个PACKET，清除RX FIFO数据
****************************************************************************/
#ifdef _SI4432_ENABLE_RX_INT_
void Si4432EnableRxInt(BOOL nEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_1, &tmp, 1, BREAD);			//Register 05h. Interrupt Enable 1

	if(nEnable == TRUE)		tmp |= 0x02;
	else				 	tmp &= ~0x02;									

	Si4432_rw(SI4432_INT_EN_1, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* 名	称：Si4432SetInvalidPreInt()
* 功	能：设置错误前导码中断
* 入口参数：bEnable, 0：不使能，1：使能
* 出口参数：无
* 说	明：检测到错误前导码，是否允许产生中断
****************************************************************************/
#ifdef _SI4432_ENABLE_INVALIDPRE_INT_
void Si4432EnableInvalidPreInt(BOOL bEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BREAD);	// Register 06h. Interrupt Enable 2
	
	if(nEnable == TRUE)		tmp |=  0x20;			// Enable Invalid Preamble Detected.		
	else					tmp &= ~0x20;			// Enable Invalid Preamble Detected.
	
	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* 名	称：Si4432SetValidPreInt()
* 功	能：设置前导码中断
* 入口参数：bEnable, 0：不使能，1：使能
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_ENABLE_VALIDPRE_INT_
void Si4432EnableValidPreInt(BOOL bEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BREAD);	// Register 06h. Interrupt Enable 2
	
	if(nEnable == TRUE)		tmp |=  0x40;			// BIT[6], Enable Valid Preamble Detected.		
	else					tmp &= ~0x40;			// BIT[6], Enable Valid Preamble Detected.
	
	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* 名	称：Si4432SetSyncWordInt()
* 功	能：设置同步字中断
* 入口参数：bEnable, 0：不使能，1：使能
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_ENABLE_SYNCWORD_INT_
void Si4432EnableSyncWordInt(BOOL bEnable) reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BREAD);	// Register 06h. Interrupt Enable 2

	if(nEnable == TRUE)		tmp |=  0x80;			// BIT[8], Enable Sync Word Detected.		
	else					tmp &= ~0x80;			// BIT[8], Enable Sync Word Detected.

	Si4432_rw(SI4432_INT_EN_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* 名	称：Si4432SetCOLC()
* 功	能：设置晶振负载电容
* 入口参数：fLC: 负载电容值，单位：pF，0 - 97fF*127=12.319pF
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_COLC_
void Si4432SetCOLC(float fLC) reentrant
{
	uint8 tmp;
	
	// 计算寄存器值	
	fLC = (fLC >= 0)? fLC : 0;
	fLC = (fLC < 12.5)? fLC : 12.5;  
	tmp = fLC * 1000 / 97.0;
	tmp = (tmp < 127)? tmp : 127;
	tmp &= 0x7F;

	// 设置
	Si4432_rw(SI4432_LOAD_CAP, &tmp, 1, BWRITE);	// Register 09h. 30 MHz Crystal Oscillator Load Capacitance
}
#endif

/****************************************************************************
* 名	称：Si4432SetCalibration()
* 功	能：设置校准
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_CALIBRATION_
void Si4432SetCalibration(void) reentrant
{
	uint8 tmp;

	//禁止RC精校准，使能VCO双精度校准，发送接收之前取消VCO校准
	tmp = 0x04;		
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BWRITE);	// Address: 55h—Calibration Control
}
#endif

/****************************************************************************
* 名	称：Si4432Calibration()
* 功	能：校准
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_CALIBRATE_
void Si4432Calibrate(void) reentrant
{
	uint8 tmp; 

	//读出原始值，只做应该的操作，不影响其他值
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BREAD);	//Address: 55h—Calibration Control
	tmp |= 0x0A;									//校准32 kHz RC Oscillator，校准VCO 
	Si4432_rw(SI4432_CALIB_CTRL, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* 名	称：Si4432Open()
* 功	能：打开Si4432电源
* 入口参数：无
* 出口参数：无
* 说	明：打开Si4432电源，等待芯片准备好后，设置DCB
****************************************************************************/
#ifdef _SI4432_INIT_
void Si4432Open()  reentrant
{
	uint8 tmp;

	//创建信号量
	if(pSi4432Event == NULL) 
	{
		pSi4432Event = OSSemCreate(0);
	}

	//开SI4432
#ifdef _SI4432POWER_
	SetLo(SI4432SDN);				
#endif //_SI4432POWER_
	
	//延时100mS
	//0x03
	OSTimeDly(10);
	do
	{
		Si4432_rw(SI4432_INT_STATUS_2, &tmp, 1, BREAD);
	}while(!(tmp & 0x02));



	//进入READY状态
	Si4432SetMode(MODE_READY);

#ifdef	SELF_TRSW
	//配置GPIO为直接数字输出
	tmp = 0x0A;
	Si4432_rw(TRSW1,&tmp,1,BWRITE);
	Si4432_rw(TRSW2,&tmp,1,BWRITE);
#endif

	//设置为发送(安全考虑)
	Si4432TRSWSend();						
	
	//设置DCB
	Si4432SetDCBToChip(&g_sSI4432DCB);
}
#endif

/****************************************************************************
* 名	称：Si4432Close()
* 功	能：关闭Si4432电源
* 入口参数：无
* 出口参数：无
* 说	明：关闭Si4432电源之后所有寄存器设置均丢失，请谨慎使用
****************************************************************************/
#ifdef _SI4432_CLOSE_
void Si4432Close() reentrant
{
#ifdef _SI4432POWER_
	SetHi(SI4432SDN);
#endif //_SI4432POWER_
}
#endif

/****************************************************************************
* 名	称：Si4432Standby()
* 功	能：进入待机模式
* 入口参数：无
* 出口参数：无
* 说	明：减少功耗
****************************************************************************/
#ifdef _SI4432_STANDBY_
void Si4432Standby()  reentrant
{
	//进入STANDBY状态
	Si4432SetMode(MODE_STANDBY);						 		
	//Si4432SetMode(MODE_SLEEP);
}
#endif

/****************************************************************************
* 名	称：Si4432Ready()
* 功	能：进入Ready模式
* 入口参数：无
* 出口参数：无
* 说	明：减少功耗
****************************************************************************/
#ifdef _SI4432_STANDBY_
void Si4432Ready()  reentrant
{
	//进入STANDBY状态
	Si4432SetMode(MODE_READY);						 		
}
#endif

/****************************************************************************
* 名	称：Si4432IsChannelClear()
* 功	能：Si4432判断信道是否空闲
* 入口参数：无
* 出口参数：指示信道是否空闲，1：空闲，2：不空闲
* 说	明：无
****************************************************************************/
#ifdef _SI4432_IS_CHANNEL_CLEARH_
BOOL Si4432IsChannelClear() reentrant
{
	uint8 tmp;

	Si4432_rw(SI4432_RX_STRENGTH, &tmp, 1, BREAD);	// Register 26h. Received Signal Strength Indicator
	
	//需要完善.....
	return tmp;
}
#endif

/****************************************************************************
* 名	称：Si4432TxFifoClear()
* 功	能：清除TXFIFO的内容
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_TX_FIFO_CLEAR_
void Si4432TxFifoClear(void) reentrant
{
	uint8 tmp;
	
	//清除FIFO，先写1，再写0
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BREAD);	// Register 08h. Operating Mode and Function Control 2
	tmp |= 0x01;
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BWRITE);			

	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BREAD);	// Register 08h. Operating Mode and Function Control 2
	tmp &= ~0x01;
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* 名	称：Si4432RxFifoClear()
* 功	能：清除RXFIFO的内容
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_RX_FIFO_CLEAR_
void Si4432RxFifoClear(void) reentrant
{
	uint8 tmp;
	
	//清除FIFO，先写1，再写0
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BREAD);	// Register 08h. Operating Mode and Function Control 2
	tmp |= 0x02;
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BWRITE);
		
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BREAD);	// Register 08h. Operating Mode and Function Control 2
	tmp &= ~0x02;
	Si4432_rw(SI4432_MODE_CTRL_2, &tmp, 1, BWRITE);
}
#endif

/****************************************************************************
* 名	称：Si4432SetNeedFreq()
* 功	能：设置需要频率
* 入口参数：nFreq：待设频率
* 出口参数：无
* 说	明：无线需打开
****************************************************************************/
#ifdef _SI4432_SET_NEED_FREQ_
void Si4432SetNeedFreq(uint16 nFreq) reentrant
{
	if(g_sSI4432DCB.nFreq != nFreq)
	{
		g_sSI4432DCB.nFreq = nFreq;
		//设置频率
		Si4432SetFreq(nFreq);
	}
}
#endif

/****************************************************************************
* 名	称：Si4432Send()
* 功	能：Si4432发送数据
* 入口参数：pd 待写数据的指针; nd 待写数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SEND_
void Si4432Send(uint8 *pd, uint16 nd) reentrant
{
	uint8 nSendLen = (uint8)nd;
	uint8 nCodeType = 0x40;
//	uint16 nIdMask;

#ifdef USE_NRF_LED
	NrfLedOn();
#endif

#ifdef USE_NRZI_4432
#ifndef USE_NRZI_4432_ON_COLLECTOR
	//映射ID号===================================================================
	//中转器ID
	nIdMask = htons(RELAY_NET_ID);	  
	if((*((uint16*)pd) & nIdMask) == nIdMask)
	{
		nCodeType |= 0x20;
		*((uint16*)pd) &= ~nIdMask;
	}													  
	else
	{
		nCodeType |= 0x10;
	}

	//采集器ID
	nIdMask = htons(COLLECTOR_NET_ID);
	if((*((uint16*)pd) & nIdMask) == nIdMask)
	{
		nCodeType |= 0x08;
		*((uint16*)pd) &= ~nIdMask;
	}
	else
	{
		nCodeType |= 0x04;
	}

	//通道号
	if((pd[2] & COLLECTOR_CHANNEL_MASK) == COLLECTOR_CHANNEL_MASK)
	{
		nCodeType |= 0x02;
		pd[2] &= ~COLLECTOR_CHANNEL_MASK;
	}
	else
	{
		nCodeType |= 0x01;
	}
#else
	nCodeType = 0x55;
#endif

	//编码=====================================================================
	nSendLen = NrziEncode0(pd, nd, pSi4432SendBuff+1); 
	
	//设置编码类型
	pSi4432SendBuff[0] = nCodeType;
	nSendLen++;

	//将数据写到发送缓冲区=====================================================
	Si4432_rw(SI4432_FIFO_ACCESS, pSi4432SendBuff, nSendLen, BWRITE);		//0x7F
	
#else

	//将数据写到发送缓冲区
	Si4432_rw(SI4432_FIFO_ACCESS, pd, nd, BWRITE);		//0x7F
#endif


 	//如果是可变长度，设置发送长度
	if(nFixPkLen == 0)
	{
		//设置发送数据包长度
		Si4432_rw(SI4432_TX_LEN, &nSendLen, 1, BWRITE);
	}

	//转换天线通道到发送
	Si4432TRSWSend();					

	//开功放
#ifdef USE_PA
	PAOn();	
	Si4432PADelay(10000);
	Si4432PADelay(10000);	
#endif

	//使能中断
	Si4432IntEnalbe();
	
	//进入发送
	Si4432SetMode(MODE_TX);				
		
	//无超时等待发送完毕
	OSSemPend(pSi4432Event, 0, &nSi4432Err);

	//进入READY状态
	Si4432SetMode(MODE_READY);

	//读4432中断寄存器(芯片自动清楚标志位)	
	Si4432_rw(SI4432_INT_STATUS_1, (uint8 *)(&nIntStatus16), 2, BREAD);

	//关功放
#ifdef USE_PA
	PAOff();
#endif
			
#ifdef USE_NRF_LED
	NrfLedOff();
#endif
}
#endif


/****************************************************************************
* 名	称：Si4432SendCarry()
* 功	能：发送载波，供测试用
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_SI4432_SEND_CARRY_
void Si4432SendCarry(uint16 nCycles) reentrant
{
	uint8 tmp;
	
//开功放
#ifdef 	USE_PA
	PAOn();
	Si4432PADelay(200);	
#endif

	//选择发送不调制载波方式，选择发送数据源为直接方式
	//设置调制方式： MT_NONE（Unmodulated Carrier），MDS_DIRECT_GPIO（Direct Mode using TX_Data via GPIO pin）
	tmp = 0x00;
	Si4432_rw(SI4432_MODUL_CTRL_2, &tmp, 1, BWRITE);		// Register 71h. Modulation Mode Control 2

	//--------开始发送--------
	//转换天线通道到发送
	Si4432TRSWSend();					

	//关闭中断
	Si4432IntDisable(); 					

	//进入发送
	Si4432SetMode(MODE_TX);				

	//一直发送
	if(0 == nCycles) while(1);
}

void Si4432SendCarryEnd() reentrant
{
	//进入READY状态
	Si4432SetMode(MODE_READY);

	//读4432中断寄存器(芯片自动清楚标志位)	
	Si4432_rw(SI4432_INT_STATUS_1, (uint8 *)(&nIntStatus16), 2, BREAD);

	//使能中断
	Si4432IntEnalbe();				
}


#endif

/****************************************************************************
* 名	称：Si4432Receive()
* 功	能：Si4432接收程序
* 入口参数：无
* 出口参数：接收到数据的个数，如果未接收成功，返回0
* 说	明：无
****************************************************************************/
#ifdef	_SI4432_RECEIVE_
uint8 Si4432Receive() reentrant
{
	uint8 tmp;

#ifdef USE_NRZI_4432
	uint8 nCodeType;
#endif

	//清标志和接收FIFO
	Si4432RxFifoClear();

	//转换天线通道到接收
	Si4432TRSWReceive();

	//使能中断
	Si4432IntEnalbe();

	//进入接收状态
	Si4432SetMode(MODE_RX);
	
	//等待接收完毕
	OSSemPend(pSi4432Event, nSi4432TimeOut, &nSi4432Err);

	//退出接收模式
	Si4432SetMode(MODE_READY);

	//读4432中断寄存器(芯片自动清除标志位)	
	Si4432_rw(SI4432_INT_STATUS_1, (uint8 *)(&nIntStatus16), 2, BREAD);
	nIntStatus16 = htons(nIntStatus16);

	//正常返回
	if(nSi4432Err == OS_NO_ERR) 
	{
		//接收：接收到正确的包
		if((nIntStatus16 & IS_VALID_PACKET_RX) != 0)				
		{
 			//如果是固定包长度，就返回该长度
			if(nFixPkLen != 0)
			{
				return nFixPkLen;
			}

			//如果不是固定包长度，则读会收到数据长度，返回长度值
			Si4432_rw(SI4432_RX_LEN, &tmp, 1, BREAD);	// Register 4Bh. Received Packet Length


#ifdef USE_NRZI_4432
			
			//检查接收长度，防止缓冲区溢出
			if(tmp == 0) return 0;
			if(tmp > SI4432_RXLEN_MAX) return 0;
			
			//读出数据(借用SendBuff作为缓存)===========================================
			Si4432_rw(SI4432_FIFO_ACCESS, pSi4432SendBuff, tmp, BREAD);
			nCodeType = pSi4432SendBuff[0];
			if((nCodeType & 0x40) != 0x40) return 0;

			//解码=====================================================================
			tmp = NrziDecode0(pSi4432SendBuff+1, tmp-1, pSi4432RecvBuff); 

			//映射ID号=================================================================
			//中转器ID
			if((nCodeType & 0x20) == 0x20)
			{
				*((uint16*)pSi4432RecvBuff) |= htons(RELAY_NET_ID);
				//pSi4432RecvBuff[0] |= RELAY_NET_ID >> 8;
				//pSi4432RecvBuff[1] |= RELAY_NET_ID;
			}													  
		
			//采集器ID
			if((nCodeType & 0x08) == 0x08)
			{
				*((uint16*)pSi4432RecvBuff) |= htons(COLLECTOR_NET_ID);
				//pSi4432RecvBuff[0] |= COLLECTOR_NET_ID >> 8;
				//pSi4432RecvBuff[1] |= COLLECTOR_NET_ID;
			}
		
			//通道号
			if((nCodeType & 0x02) == 0x02)
			{
				pSi4432RecvBuff[2] |= COLLECTOR_CHANNEL_MASK;
			}

#endif

			return tmp;
		}
	}

	//接收超时
	if(nSi4432Err == OS_TIMEOUT) return 1;

	return 0;
}
#endif

/****************************************************************************
* 名	称：Si4432ReadData()
* 功	能：Si4432读接收数据
* 入口参数：pd 待读数据的指针; nd 待读数据的长度
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_READ_
void Si4432ReadData(uint8 *pd,uint16 nd) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif
#ifdef USE_NRZI_4432
	OS_ENTER_CRITICAL();
	memcpy(pd, pSi4432RecvBuff, nd);
	OS_EXIT_CRITICAL();
#else
	Si4432_rw(SI4432_FIFO_ACCESS, pd, nd, BREAD);	//0x7F
#endif
}
#endif
	
/****************************************************************************
* 名	称	Int0ISR()
* 功	能：发送/接收中断程序
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void EXTI0_IRQHandler()
{		 
	//进入中断
	OSIntEnter();

    //清除中断标志
    EXTI_ClearITPendingBit(EXTI_Line0);

	//关4432中断
	Si4432IntDisable();

	//发送信号量
	OSSemPost(pSi4432Event);

	//关闭控制
#ifdef USE_PA
	PAOff();					   
#endif

	//退出中断
	OSIntExit();				
}

/****************************************************************************
* 名	称：Si4432_rw()
* 功	能：Si4432读写函数
* 入口参数：addr 读写寄存器地址; pd 待读写数据的指针; nd 待读写数据的长度; bNeedRead 读写控制,是否需要读回数据
* 出口参数：无
* 说	明：
****************************************************************************/
#ifdef _SI4432_INIT_
static void Si4432_rw(uint8 addr, uint8 *pd, uint16 nd, uint8 bNeedRead) reentrant
{
//#if OS_CRITICAL_METHOD == 3  
//	OS_CPU_SR  cpu_sr = 0;
//#endif

	uint16 i;
	uint8  trst;

	//读写控制
	if(bNeedRead)	addr &= 0x7F;	//读，最高位为0
	else			addr |= 0x80;	//写，最高位为1

	//OS_ENTER_CRITICAL();
	
	SetLo(SI4432NSEL);
	
	SPI1_RwByte(addr);
	for(i=0; i<nd; i++)
	{
		trst = SPI1_RwByte(pd[i]);
		if(bNeedRead)
		{
			pd[i] = trst;
		}
	}

	SetHi(SI4432NSEL);
	
	//OS_EXIT_CRITICAL();
}
#endif

/****************************************************************************
* 名	称	Si4432PADelay()
* 功	能：延时函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef USE_PA
static void Si4432PADelay(uint16 nTime)
{
	while(nTime--);
}	
#endif

//****************************************************************************************************************
//****************************************************************************************************************
//********附加功能函数********************************************************************************************
//****************************************************************************************************************
//****************************************************************************************************************

/****************************************************************************
* 名	称：Si4432SetADC()
* 功	能：设置ADC
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_ADC_
void Si4432SetADC(void) reentrant
{

}
#endif

/****************************************************************************
* 名	称：Si4432GetADC()
* 功	能：获取ADC值
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_GET_ADC_
uint8 Si4432GetADC(void) reentrant
{
	uint8 tmp;
	
	tmp =0;

	return tmp;
}
#endif

/****************************************************************************
* 名	称：Si4432SetRSSI()
* 功	能：设置无线信号强度指示门限值，单位：0.5dB
* 入口参数：门限值
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_RSSI_GATE_
void Si4432SetRSSIGate(uint8 nStep) reentrant
{
	Si4432_rw(SI4432_RSSI_THRES, &nStep, 1, BWRITE);	// Register 27h. RSSI Threshold for Clear Channel Indicator
}
#endif

/****************************************************************************
* 名	称：Si4432GetRSSI()
* 功	能：获取无线信号强度
* 入口参数：无
* 出口参数：无线信号强度
* 说	明：无
****************************************************************************/
#ifdef _SI4432_GET_RSSI_
uint8 Si4432GetRSSI(void) reentrant
{
	uint8 tmp;
	
	Si4432_rw(SI4432_RX_STRENGTH, &tmp, 1, BREAD);	// Register 26h. Received Signal Strength Indicator

	//需要完善。。。。。。。。。。。。。
	return tmp;
}
#endif

/****************************************************************************
* 名	称：Si4432SetTS()
* 功	能：设置温度传感器
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_TS_
void Si4432SetTS(void) reentrant
{
	uint8 tmp;

	Si4432_rw(0x66, &tmp, 1, BREAD); 		// 读使能温度传感器寄存器
	tmp |= 0x20;							// 修改值，使能温度传感器
	Si4432_rw(0x66, &tmp, 1, BWRITE); 		// 写使能温度传感器寄存器

	tmp = 0x00;
	Si4432_rw(0x0F, &tmp, 1, BWRITE); 		// 设置ADC采样温度传感器，ADC参考电压
	tmp = 0x20;
	Si4432_rw(0x12, &tmp, 1, BWRITE); 		// 设置温度传感器温度范围、偏置	
	
	tmp = 0x80;
	Si4432_rw(0x0F, &tmp, 1, BWRITE); 		// 开始采集		
	Si4432_rw(0x11, &tmp, 1, BREAD); 		// 读温度值
	Si4432_rw(0x13, &tmp, 1, BREAD); 		// 读温度偏置
}
#endif

/****************************************************************************
* 名	称：Si4432GetTemp()
* 功	能：读温度值
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_TS_
float Si4432GetTemp(void) reentrant
{
	uint8 tmp;
	float f;

	tmp = 0x80;
	Si4432_rw(0x0F, &tmp, 1, BWRITE); 	// 开始采集		
	Si4432_rw(0x11, &tmp, 1, BREAD); 	// 读温度值
	//Si4432_rw(0x13, &tmp, 1, BREAD); 	// 读温度偏置

	f = 0.5 * tmp - 64;
	
	return f;
}
#endif


/****************************************************************************
* 名	称：Si4432SetGPIO()
* 功	能：设置GPIO
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_GPIO_
void Si4432SetGPIO(void) reentrant
{
	uint8 tmp;

	// 读GPIO配置
	//Si4432_rw(0x0B, &tmp, 1, BREAD);			// 读
	//Si4432_rw(0x0C, &tmp, 1, BREAD);			// 读
	//Si4432_rw(0x0D, &tmp, 1, BREAD);			// 读
	tmp = 0x1F;				// 均设置为GND
	Si4432_rw(0x0B, &tmp, 1, BWRITE);			// 写
	Si4432_rw(0x0C, &tmp, 1, BWRITE);			// 写
	Si4432_rw(0x0D, &tmp, 1, BWRITE);			// 写

}
#endif

/****************************************************************************
* 名	称：Si4432SetLDO()
* 功	能：设置LDO
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_LDO_
void Si4432SetLDO(void) reentrant
{

}
#endif


/****************************************************************************
* 名	称：Si4432SetWUT()
* 功	能：设置唤醒定时器
* 入口参数：需要定时的时间，单位：秒（S），范围：0 - 65535*32=2097120(S)=34952(M)=582.53(H)=24.27(D)
* 出口参数：无
* 说	明：唤醒定时器只在SLEEP模式下起作用
****************************************************************************/
#ifdef _SI4432_SET_WUT_
void Si4432SetWUT(uint32 nTimeOut) reentrant
{
	uint8 tmp;
	uint16 ntime;
	uint8 b,r;

	nTimeOut = (nTimeOut <= 2097120)? nTimeOut : 2097120;		// 不能超过最大值
	
	tmp = (nTimeOut-1) / 65535 + 1;					// 求取倍数
	ntime = nTimeOut / tmp;		

	//求2的次方数
	b = 1;
	r = 0;
	while(b < tmp)
	{
		b <<= 1;
		r++;
	}
	
	tmp = r + 10;									// 适应公式
	tmp = tmp << 2;									// 将倍数移动到指定位置
													
	Si4432_rw(0x14, &tmp, 1, BWRITE); 				// 设置D和R
	
	ntime = htons(ntime);
	Si4432_rw(0x15, (uint8 *)(&ntime), 2, BWRITE); 	// 设置M
	
	// 使能WUT中断
	Si4432_rw(0x06, &tmp, 1, BREAD); 				// 读中断使能寄存器 
	tmp |= 0x08;									// 修改之，使能WUT中断
	Si4432_rw(0x06, &tmp, 1, BWRITE); 				// 写回
	
}
#endif

/****************************************************************************
* 名	称：Si4432SetLBD()
* 功	能：设置低电压检测
* 入口参数：低电压门限值
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_LBD_
void Si4432SetLBD(float nVoltage) reentrant
{
	uint8 tmp;
	uint8 i;

	nVoltage = 0;

	//使能低电压检测
	Si4432_rw(SI4432_MODE_CTRL_1, &tmp, 1, BREAD); 		//读回模式控制寄存器1的值
	tmp |= 0x40;								   		//修改值，增加低电压检测功能
	Si4432_rw(SI4432_MODE_CTRL_1, &tmp, 1, BWRITE); 	//写回模式控制寄存器1的值

	tmp = 0x00;
   	Si4432_rw(0x1A, &tmp, 1, BWRITE);					//写低电压门限值

	NOP;NOP;NOP;

	for(i=0; i<4; i++)									//读4次数据，才能正常产生中断
	{
		Si4432_rw(0x1B, &tmp, 1, BREAD);				//读电压值水平
	}
}
#endif

/****************************************************************************
* 名	称：Si4432SetMCUClk()
* 功	能：设置输出时钟
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_MCU_CLK_
void Si4432SetMCUClk(void) reentrant
{

}
#endif

/****************************************************************************
* 名	称：Si4432SetLDCM()
* 功	能：设置低占空比模式
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_SET_LDCM_
void Si4432SetLDCM(void) reentrant
{

}
#endif

