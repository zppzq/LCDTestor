/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: UserComm.c
**创   建   人: 杨承凯
**创 建 日  期: 2007年09月18日
**最后修改日期: 2007年09月18日
**描        述: 传感器源文件
*****************************************************************************************************************/
#define _USERCOMM_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "UserComm.h"
//#include "BrgNet.h"
//#include "DataManage.h"
#include "Sensor.h"

//defines
#define COLLECTOR_MODEL			201

#define ENABLE_LACAL_ZEROED		1					//本地调零使能
#define ENABLE_CARRY_SEND		1					//载波使能
//#define DEBUG_RECV_SYNC_INFOR	1					//是否回传同步信息

//超时时间定义
#define SLEEP_RECV_TIMEOUT		200					//休眠时接收超时时间，单位为ms
#define NORMAL_RECV_TIMEOUT		5000				//常态下接收超时时间，单位为ms

#define DEVICE_WRONG_ORDER		0

//signal defines--------------------------------------------------------------------------------------------------
//****************************************************************************************************************
//需要高效处理的数据
uint8 	 g_nState = 0;							//采集器状态字
uint8 	 Timer3Counter = 0;						//定时器3分段计数器
uint8 	 Timer3CounterMax = 4;					//定时器3分段计数器最大值
BOOL  	 bDisableDynamic = TRUE;					//禁止、使能动态采集
uint16 	 nRequireCollector = 0;					//动态采集时被呼叫的采集器号
uint16 	 nSyncCode;								//同步功能码
uint32 	 nRequireAddr = 0;						//动态采集时被呼叫的数据地址
uint8 	 nHostReadBytes = 0;						//上级要读取的数据个数
uint8 	 g_nDynChannelID = 0;					//动态采集返回数据通道号
uint32 	 nHostDeviceByteDiff = 0;				//主机和本地的数据个数差值


//Timer3控制========================================================================
#define	Timer3Stop()		TMR3CN &= ~0x04			//停止定时器3
#define	Timer3Start()		TMR3CN |= 0x04			//开定时器3
#define	CallTimer3ISR()		TMR3CN |= 0x80			//调用定时器3中断
#define	ClearTimer3ISR()	TMR3CN &= ~0x80			//清除定时器3中断
#define	CloseTimer3ISR()	EIE1 &= ~0x80			//关定时器3中断
#define	OpenTimer3ISR()		EIE1 |= 0x80			//开定时器3中断
uint8 TH3RL = 0;									//定时器3高字节重载值
uint8 TL3RL = 0;									//定时器3低字节重载值
uint32 nRateTime = 0;								//记录本地完成一次采样所需的采样周期/ 4
uint32 nHostTime = 0;								//记录主机已跑时间，单位100us
uint32 nLocalTime = 0;								//记录本地应该更新到的时间，单位100us
uint32 nCycleTimeLeft = 0;							//记录本地应该更新时间的机器周期
uint32 nCycleTicketLeft = 0;						//记录本地应该更新时间的机器周期
uint32 nTicketPer100us;								//每100us的定时器个数
float fTicketPer100us;								//每100us的定时器个数(浮点数)

//无线部分参数定义=================================
static uint8 NrfRxBuff[64];							//上层数据接收缓冲区
static uint8 NrfTxBuff[128];						//上层数据发送缓冲区

//4432配置信息
WIRELESSDCB *pWirelessDCB;							//无线设备指针

//频率参数
uint16 nDefaultFreq;								//默认频率
uint16 nCurFreq;									//当前频率
uint16 nCarryFreq;									//当前载波频率
uint16 nCarrySendCycles;							//载波发送周期

//功能码
uint8 nCltFunc;										//采集器功能码
uint8 nCltCfg;										//采集器配置字
float fBrgSens[SENSOR_MAX_COUNT];					//灵敏度

//AD存储系数
float g_fADStoreParam[SENSOR_MAX_COUNT];			//数据转换系数

//系数
float fCltK[4];

//采样率
#define DEFAULT_SAMPLE_RATE		40					//默认采样率
uint16 nBrgSetSampleRate;							//要设置的采样率
uint16 nBrgCurSampleRate;							//当前采样率
uint16 nBrgRefSampleSplt = SAMPLE_SLIP_SYNC_RT;		//动态采集参考采样率(同步信号频率)
float SYS_OPT_SEG fBrgSampleDelt;					//主机参考索引乘以这个参数就等于采集次数

//当前状态
BOOL bArrTaskBusy[SENSOR_MAX_COUNT];				//任务忙标志
//uint8 nArrSensorTask[SENSOR_MAX_COUNT];			//传感器任务
uint8 nMemTask;										//存储器任务

BOOL g_bPrepareEnable;								//记录是否使用预采集
BOOL g_bLocalSetZero;								//记录是否使用本地调零
BOOL bDynEnd;										//用于AD结束动态采集时控制一次数据存储

//运行参数
STORE_DATA_TYPE_S g_nCurDataType[SENSOR_MAX_COUNT];	//记录当前要保存的数据类型

//计数值
uint32 SYS_OPT_SEG nLocalCount;						//本地存储计数值
uint32 nDynLocalAddr[SENSOR_MAX_COUNT];				//数据存储区地址长度
uint32 nStoreAddr[SENSOR_MAX_COUNT];				//已存储数据计数
uint32 SYS_OPT_SEG nHostCount;						//远程存储计数值
float  SYS_OPT_SEG fHostCount;						//远程存储计数值浮点数
BOOL   g_nAllowedSample = 0;						//是否允许采样

//临时数据
static float TmpFloat;								//临时数据
static float TmpFloatZero;							//临时调零数据
static float fMesureCur;							//当前应变值

//事件控制块参数定义=======================================
static OS_EVENT 	*pMemEvent = NULL;				//事件控制块
static uint8 		nMemErr;						//错误标志

uint32 nPushDynNum = 0;								//存入动态缓冲区的个数
BOOL bADPushDyn;									//需要存入动态缓冲区的数据个数
uint32 nFlashErasedSect;							//Flash擦除过的地址
uint32 nFlashToBeErase;								//需要擦除的Flash地址

//存储控制
BOOL bDataNeedStore[SENSOR_MAX_COUNT];				//需要存储数据的通道
BOOL bDataNeedStoreClone[SENSOR_MAX_COUNT];			//需要存储数据的通道

//记录是否外接传感器标志
uint8 nSensorConnected[SENSOR_MAX_COUNT];

uint8 g_nMeasureType[SENSOR_MAX_COUNT];				//采集器测量


//外部参数声明==================================
extern float	g_fBatteryHigh;						//电池满电压，单位V
extern float	g_fBatteryLow;						//电池低电压，单位V
extern uint8	g_nBatteryRate;						//电池电量
extern float	g_fBatteryVol;						//电池电压

extern uint16	g_nSleepPoint;					    //多久之后休眠(单位为5S)
extern uint16	g_nSleepTicketMax;					//休眠时间为多久(单位为5S)

extern uint8 g_nStoreSectStart[SENSOR_MAX_COUNT];				//当前缓冲段的开始索引
extern uint8 g_nStoreSectLen[SENSOR_MAX_COUNT];					//当前缓冲段的长度
extern int8 g_nDynBuff[SENSOR_MAX_COUNT][DYN_BUFF_LEN];			//动态数据缓冲区

extern float idata g_fMesureZeroVol[SENSOR_MAX_COUNT];			//存储到存储器中的调零电压
extern DT_STORE idata g_sMesureZero[SENSOR_MAX_COUNT];			//调零数据，用于计算的参数


//数据组操作
uint8 idata nPrepareCount;
uint8 idata nTempChannel;

//休眠控制
uint16 nSleepRecvTimout = SLEEP_RECV_TIMEOUT;		//休眠时接收超时时间，单位为ms
uint16 nNormalRecvTimeout = NORMAL_RECV_TIMEOUT;	//常态下接收超时时间，单位为ms

float fMesureCurTemp;
int16 nMesureCurTemp;

/****************************************************************************
* 名	称：Timer3Init()
* 功	能：定时器3初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void Timer3Init() reentrant
{
	TMR3RLH = TH3RL;		//初始化重载值
	TMR3RLL = TL3RL;		//初始化重载值
	TMR3CN = 0x00;			//清除标志位
	CKCON &= 0x3F;			//以系统时钟的12分频作为定时器时钟
	EIE1 |= 0x80;			//开定时器3中断
}

/****************************************************************************
* 名	称：Timer3ResetRun()
* 功	能：定时器3复位并运行
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void Timer3ResetRun() reentrant
{
	TMR3CN = 0x00;

 	Timer3Counter = 0;
	TMR3H = TMR3RLH;
	TMR3L = TMR3RLL;	
	
	Timer3Start();			//开始运行
}

/****************************************************************************
* 名	称：SetTimer3Rate()
* 功	能：设置定时器3溢出频率
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void SetTimer3Rate(int nRate) reentrant
{
	unsigned int nCount;

	//规范化采样率
	nRate = (nRate < AD_RATE_MIN) ? AD_RATE_MIN : nRate;
	nRate = (nRate > AD_RATE_MAX) ? AD_RATE_MAX : nRate;

	//确定分段数目
	if(nRate >= 32)	Timer3CounterMax = 1;
	else if(nRate >= 16) Timer3CounterMax = 2;
	else if(nRate >= 8) Timer3CounterMax = 4;
	else if(nRate >= 4) Timer3CounterMax = 8;
	else if(nRate >= 2) Timer3CounterMax = 16;
	else Timer3CounterMax = 32;

	//计算定时时间
	nRateTime = (SYSCLK / 12 ) / nRate;
	nRateTime /= Timer3CounterMax;
	nCount = -nRateTime;

	//加载定时器
	TMR3RLH = HIBYTE(nCount);
	TMR3RLL = LOBYTE(nCount);
	TMR3H = TMR3RLH;
	TMR3L = TMR3RLL;
	
	//预先计算一些参数
	fTicketPer100us	= 100.0f / (12000000.0f / (float)SYSCLK);
	nTicketPer100us = fTicketPer100us;
}

/****************************************************************************
* 名	称：Timer3SetAndRun()
* 功	能：定时器3设置并运行
* 入口参数：无
* 出口参数：无
* 说	明：同步到本地计数，之前先更新本地计数
****************************************************************************/
uint16 nTimeSetData = 0;
void Timer3CaliAndRun() reentrant
{
	TMR3CN = 0x00;

	//需要预设的时间节拍
	nCycleTicketLeft = nCycleTimeLeft * nTicketPer100us;  	

	//计算分段数目
	Timer3Counter = nCycleTicketLeft / nRateTime;
	
	//还需要运行的分段数目
	//if(Timer3Counter >= Timer3CounterMax)
	//{
	//	Timer3Counter = Timer3CounterMax - 1;		//理论上不应该运行到这里
	//}
	Timer3Counter = Timer3CounterMax - Timer3Counter - 1;

	//计算分段后剩余的时间
	nCycleTicketLeft = nCycleTicketLeft % nRateTime;


	//计算剩余时间
	nTimeSetData = -nCycleTicketLeft;
	TMR3H = HIBYTE(nTimeSetData);
	TMR3L = LOBYTE(nTimeSetData);	
	
	//开始运行
	Timer3Start();
}

/****************************************************************************
* 名	称：SetNormalRecvTimeOut()
* 功	能：设置正常的接收超时时间
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetNormalRecvTimeOut() reentrant
{
	//设置Nrf905接收超时时间
	//注意这个接收时间给采集器休眠时间有关
	WirelessSetRecvTimeOut(nNormalRecvTimeout);
}

/****************************************************************************
* 名	称：SetSleepRecvTimeOut()
* 功	能：设置睡眠时的接收超时时间
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetSleepRecvTimeOut() reentrant
{
	//设置Nrf905接收超时时间
	//注意这个接收时间给采集器休眠时间有关
	WirelessSetRecvTimeOut(nSleepRecvTimout);
}

/****************************************************************************
* 名	称：SetFreqToDefault()
* 功	能：频率设置为默认频率
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetFreqToDefault() reentrant
{
	//配置为默认频率
	WirelessSetNeedFreq(nDefaultFreq);
}

/****************************************************************************
* 名	称：SetFreqToCur()
* 功	能：频率设置为当前频率
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetFreqToCur() reentrant
{
	//频率配置
	WirelessSetNeedFreq(nCurFreq);
}

/*****************************************************************************************************************
* 名	称：UserCommInit()
* 功	能：通信应用层初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void UserCommInit() reentrant
{
	uint8 i;

	//创建存储器擦除事件
	if(pMemEvent == NULL)
	{
		pMemEvent = OSSemCreate(0);	
	}	

	//定时器3初始化(同步时钟定时器)
	Timer3Init();

	//采样率设置
	nBrgCurSampleRate = 32;
	nBrgSetSampleRate = 32;

	//禁止动态采集，擦除Flash时自动使能，停止动态采集时自动禁止
	bDisableDynamic = TRUE;
	nFlashErasedSect = 0;
	nFlashToBeErase = 0;
	g_nAllowedSample = 0;

	//设置通信参数
	pWirelessDCB = Si4432GetDCB();
	nDefaultFreq = FREQ_DEVICE_DEFAULT;		//设置默认频率
	nCurFreq = nDefaultFreq;				//当前为默认频率
	pWirelessDCB->nFreq = nDefaultFreq;		//通信频率设置

	//设置常规超时时间
	SetNormalRecvTimeOut();

	//数据初始化
	DataInitZero(bArrTaskBusy);
	DataInitZero(bDataNeedStore);
	nMemTask = 0;

	g_bPrepareEnable = FALSE;		//初始状态，不使用预采集。在开机时不进行采集
	g_bLocalSetZero = FALSE;

	for(i=0;i<SENSOR_MAX_COUNT;i++)
	{
		g_nCurDataType[i].nStoreType = STORE_TYPE_UV;	//默认存电压
		g_nCurDataType[i].nDataType = DATA_TYPE_0F_128G;
		fBrgSens[i] = 2.0f;

		g_nMeasureType[i] = MEASURE_VOL;
	}

}

/*****************************************************************************************************************
* 名	称：SetStoreType()
* 功	能：设置存储类型
* 入口参数：存储类型:是否要计算成应变。fSense:灵敏度系数。fRange:量程。
* 出口参数：无
* 说	明：无
*====================================================================
* PGA		电压(uv)		应变(ue)
* 1			2500000			1515151
* 2			1250000			757575
* 4			625000			378787
* 8			312500			189393
* 16		156250			94696
* 32		78125			47348
* 64		39062.5			23674
* 128		19531.25		11837
*====================================================================
*****************************************************************************************************************/
void SetStoreType(uint8 nType, fp32 fSense, fp32 fRange) reentrant
{
	uint8 tmp1,tmp2;
	
	//判断低4位，确定存储数据格式
	tmp1 = nType;

	//要求存储上传电压值==================
	if(5 == tmp1) 		
	{
		g_nMeasureType[g_nChannelID] = MEASURE_VOL;		//测量电压

		g_fADStoreParam[g_nChannelID] = 1.0f;

		//根据电压值设置存储类型
		if(fRange <= 301) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 100.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 601) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 50.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 1501) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 20.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 3001)
		{
			tmp2 = DATA_TYPE_1F_128G;
			g_fADStoreParam[g_nChannelID] = 10.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 19532) tmp2 = DATA_TYPE_0F_128G;
		else if(fRange <= 30001) tmp2 = DATA_TYPE_0F_64G;
		else if(fRange <= 39062) 
		{
			tmp2 = DATA_TYPE_0F_64G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 78126) 
		{
			tmp2 = DATA_TYPE_0F_32G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 156251) 
		{
			tmp2 = DATA_TYPE_0F_16G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 312501) 
		{
			tmp2 = DATA_TYPE_0F_8G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}			
		else if(fRange <= 625001) 
		{
			tmp2 = DATA_TYPE_0F_4G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}			
		else if(fRange <= 1250001) 
		{
			tmp2 = DATA_TYPE_0F_2G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}			
		else 
		{
			tmp2 = DATA_TYPE_0F_1G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}			
	}

	//存储上传应变值
	else		
	{
		g_nMeasureType[g_nChannelID] = MEASURE_STRAIN;		//测量应变

		//全桥
		if(1 == tmp1) g_fADStoreParam[g_nChannelID] = 1.0f / (3.3f * fSense);

		//半桥
		else if(2 == tmp1) g_fADStoreParam[g_nChannelID] = 1.0f / (1.65f * fSense);

		// 1/4桥
		else if(4 == tmp1) g_fADStoreParam[g_nChannelID] = 1.0f / (0.825f * fSense);

		//根据应变值设置存储类型
		if(fRange <= 101) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 100.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 201) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 50.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 501) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[g_nChannelID] = 20.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 2001)
		{
			tmp2 = DATA_TYPE_1F_128G;
			g_fADStoreParam[g_nChannelID] = 10.0f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 10001) tmp2 = DATA_TYPE_0F_128G;
		else if(fRange <= 20001) tmp2 = DATA_TYPE_0F_64G;
		else if(fRange <= 40001) 
		{
			tmp2 = DATA_TYPE_0F_32G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 80001) 
		{
			tmp2 = DATA_TYPE_0F_16G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 160001) 
		{
			tmp2 = DATA_TYPE_0F_8G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 320001) 
		{
			tmp2 = DATA_TYPE_0F_4G;
			g_fADStoreParam[g_nChannelID] = 0.1f * g_fADStoreParam[g_nChannelID];
		}
		else if(fRange <= 640001) 
		{
			tmp2 = DATA_TYPE_0F_2G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}
		else 
		{
			tmp2 = DATA_TYPE_0F_1G;
			g_fADStoreParam[g_nChannelID] = 0.01f * g_fADStoreParam[g_nChannelID];
		}
	}

	//数据类型
	g_nCurDataType[g_nChannelID].nDataType = tmp2;
}

/*****************************************************************************************************************
* 名	称：HostCommProcess()
* 功	能：上层通信处理
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
BOOL HostCommProcess(uint8 nLen) reentrant
{
	uint8 i;
	uint8 nOrder;										//命令字
	BOOL bNeedReply;									//标志位，指示是否需要发送数据
	uint8 *pBuff = NrfRxBuff;							//接收缓冲区
	uint8 *pSendBuff = NrfTxBuff+BRG_ADDR_LEN;			//应用程发送数据帧头部
	uint8 nSendLen;										//发送数据长度

	//接收下层协议数据
	if(BrgNetRecv(pBuff, nLen - BRG_ADRC_LEN) != TRUE) return FALSE;
		
	//回复命令初始化
	nOrder = *pBuff++;									//获取命令字
	*pSendBuff++ = nOrder;								//复制指令，回复时需要

	//处理优先级别高的任务
	if(BRG_WAKE_UP == nOrder)
	{
		//清任务忙标志
		DataInitZero(bArrTaskBusy);

		//使用预采集,唤醒时停止采集，复位AD
		#ifdef USE_STATIC_PREPARE
		{
			SensorStop();
		}
		#endif
	}

	//如果任务忙
	if(bArrTaskBusy[g_nChannelID] == TRUE)
	{
		*pSendBuff++ = DEVICE_STATE_BUSY;
		nSendLen = 2;
		BrgNetSend(NrfTxBuff + BRG_ADDR_LEN, nSendLen);
		return TRUE;
	}	

	//回复状态初始化
	*pSendBuff++ = DEVICE_DATA_VALID;					//指示回复数据有效，移动指针到数据区
	bNeedReply = TRUE;									//默认需要回复	
	nSendLen = 2;										//默认回复长度为2

	// 协议处理，处理需要回复前的事情============================
	switch(nOrder)
	{
 		//唤醒信号，不回复
		case BRG_WAKE_UP:
			if(pBuff[0] == WAKEUP_RESET_FREQ)
			{
				if(*((uint16*)(pBuff+1)) == WAKEUP_RESET_KEY)
				{	
					//复位频率
					nCurFreq = FREQ_DEVICE_DEFAULT;
				}
			}
			bNeedReply = FALSE;
			break;

		//配置频率，数据区前两字节表示要设置的频率值
		case BRG_SET_FREQ:
			if(BRG_FREQ_KEY == *(uint32*)(pBuff))
			{
				//完成本次循环后，通过调用SetFreqToCur设置
				nCurFreq = *((uint16*)(pBuff+4));
			}
			else
			{
				bNeedReply = FALSE;	 
			}		
			break;


		//连接设备
		case BRG_CONNECT_DEVICE:
			//校验关键字
			if(BRG_CONNECT_KEY != *(uint32*)pBuff)
			{
				bNeedReply = FALSE;		//不回复
				nOrder = 0;	 			//取消后面的操作
				break;
			}

			//校验设备类型
			if(COLLECTOR_MODEL == *((uint16*)(pBuff+4)))
			{
				nCltFunc = *(pBuff+6);						//功能号
				nCltCfg = *(pBuff+7);						//配置字
				
				//预采集标志
				if(0 < (nCltCfg & 0x02)) g_bPrepareEnable = TRUE;
				else g_bPrepareEnable = FALSE;
				
				fCltK[0] = *((float*)(pBuff+8));		//量程
				fCltK[1] = *((float*)(pBuff+12));		//灵敏度系数(K)
				fCltK[2] = *((float*)(pBuff+16));		//电池最低电压，

				//电压率设置
				if((fCltK[2] < g_fBatteryVol) && (fCltK[2] > 3.5f))
				{
					//重新设置电量范围
					g_fBatteryLow = fCltK[2];
					g_fBatteryHigh =  g_fBatteryLow + 1.0f;
				}

			}

			//采集器类型错误
			else
			{
				//取消后面的操作
				nOrder = 0;
			}

			//返回电池电量
			*pSendBuff = g_nBatteryRate;
			
			//设置回传数据长度
			nSendLen += sizeof(g_nBatteryRate);

			//返回当前设备类型
			*((uint16*)(pSendBuff+1)) = (uint16)COLLECTOR_MODEL;
			nSendLen += 2;

			break;
		
		//读采集器状态，数据区前一字节存放状态
		case BRG_READ_STATE:

			//查询电池电量
			if(pBuff[0] == BRG_STATE_BATTERY)
			{
				*pSendBuff = g_nState;
				*(pSendBuff+1) = BRG_STATE_BATTERY;
				*(pSendBuff+2) = g_nBatteryRate;
				nSendLen += 3;
			}

			//预查询传感器状态
			else if(pBuff[0] == BRG_STATE_SENSOR_PRE)
			{
				//设置传感器任务及状态
				DataInit(bArrTaskBusy, TRUE);

				SetSensorTask(SENSOR_TASK_DETECTION);
				PostSensorPro();
			}

			//查询传感器状态
			else if(pBuff[0] == BRG_STATE_SENSOR)
			{
				*pSendBuff = g_nState;
				*(pSendBuff+1) = BRG_STATE_SENSOR;
				*(pSendBuff+2) = nSensorConnected[0];
				*(pSendBuff+3) = nSensorConnected[1];
				*(pSendBuff+4) = nSensorConnected[2];
				*(pSendBuff+5) = nSensorConnected[3];
				nSendLen += 6;
			}

			//查询版本号
			else if(pBuff[0] == BRG_STATE_VERSION)
			{
				*pSendBuff = g_nState;
				*(pSendBuff+1) = BRG_STATE_VERSION;	 
				*((unsigned long*)(pSendBuff+2)) = SYSTEM_VERSION;

				nSendLen += 6;
			}

			break;
		
		//获取电池电量
		case BRG_GET_POWER:
			*pSendBuff = g_nBatteryRate;
			nSendLen += 1;
			break;

		case BRG_SLEEP_CONTROL:
			//关键字校验
			if(BRG_SLEEP_KEY != *(uint32*)pBuff) 
			{
				bNeedReply = FALSE;
				break;
			}

			//多久休眠(单位为5秒)
			g_nSleepPoint = *((uint16*)(pBuff+4));

			//休眠多久(单位为5秒)
			g_nSleepTicketMax = *((uint16*)(pBuff+6));

			//休眠时侦听时间(毫秒)
			nSleepRecvTimout = *((uint16*)(pBuff+8));
			break;

		//采集器校零，数据区未用
		case BRG_SET_ZERO:

			//校验关键字
			if(*((uint32*)pBuff) != BRG_ZERO_KEY)
			{
				bNeedReply = FALSE;
				break;				
			}

			//只有本地调零时才做下一操作
			if(TRUE == g_bLocalSetZero)
			{
				//调零
				if(pBuff[4] == SET_ZERO_EACH)
				{
					g_fMesureZeroVol[g_nChannelID] = SensorGetStatic(g_nChannelID);
					g_sMesureZero[g_nChannelID] = (DT_STORE)(g_fMesureZeroVol[g_nChannelID] * g_fADStoreParam[g_nChannelID]);
					*(DT_STORE*)pSendBuff = g_sMesureZero[g_nChannelID];
					g_nState |= COLLECTOR_ZERO;

					//设置回传数据长度
					nSendLen += sizeof(DT_STORE);
				}
				
				//取消调零
				else if(pBuff[4] == CANCEL_ZERO_EACH)
				{
					g_fMesureZeroVol[g_nChannelID] = 0;
					g_sMesureZero[g_nChannelID] = 0;
					*(DT_STORE*)pSendBuff = g_sMesureZero[g_nChannelID];
					g_nState &= ~COLLECTOR_ZERO;

					//设置回传数据长度
					nSendLen += sizeof(DT_STORE);
				}

				//组调零
				else if(pBuff[4] == SET_ZERO_ARR)
				{
					nPrepareCount = pBuff[5];
					nPrepareCount = (nPrepareCount > SENSOR_MAX_COUNT) ? SENSOR_MAX_COUNT : nPrepareCount;
					
					OS_ENTER_CRITICAL();
					for(i = 0; i < nPrepareCount; i++)
					{
						//获取通道号
						nTempChannel = pBuff[6+i] - 1;
						nTempChannel = (nTempChannel >= SENSOR_MAX_COUNT) ? (SENSOR_MAX_COUNT-1) : nTempChannel;

						//对该通道调零
						g_fMesureZeroVol[nTempChannel] = SensorGetStatic(nTempChannel);
						g_sMesureZero[nTempChannel] = (DT_STORE)(g_fMesureZeroVol[nTempChannel] * g_fADStoreParam[nTempChannel]);
						*(DT_STORE*)pSendBuff = g_sMesureZero[nTempChannel];
						g_nState |= COLLECTOR_ZERO;

						//设置回传数据长度
						pSendBuff += sizeof(DT_STORE);
						nSendLen += sizeof(DT_STORE);
					}
					OS_EXIT_CRITICAL();	
				}

				//组取消调零
				else if(pBuff[4] == CANCEL_ZERO_ARR)
				{
					nPrepareCount = pBuff[5];
					nPrepareCount = (nPrepareCount > SENSOR_MAX_COUNT) ? SENSOR_MAX_COUNT : nPrepareCount;
					
					OS_ENTER_CRITICAL();
					for(i = 0; i < nPrepareCount; i++)
					{
						//获取通道号
						nTempChannel = pBuff[6+i] - 1;
						nTempChannel = (nTempChannel >= SENSOR_MAX_COUNT) ? (SENSOR_MAX_COUNT-1) : nTempChannel;

						//对该通道取消调零
						g_fMesureZeroVol[nTempChannel] = 0;
						g_sMesureZero[nTempChannel] = 0;
						*(DT_STORE*)pSendBuff = g_sMesureZero[nTempChannel];
						g_nState &= ~COLLECTOR_ZERO;

						//设置回传数据长度
						pSendBuff += sizeof(DT_STORE);
						nSendLen += sizeof(DT_STORE);
					}
					OS_EXIT_CRITICAL();	
				}

			}
			break;
			
		//静态采集操作
		case BRG_STATIC_OPERATION:
			
			//开始静态采集
			if(pBuff[0] == BRG_START_STATIC)
			{
				g_nState |= COLLECTOR_STATIC;					//置静态采集标志位
				g_nState &= ~COLLECTOR_DYNAMIC;					//取消动态采集标志位

				//不使用预采集时，开始静态采集时
				#ifndef USE_STATIC_PREPARE
				{
					SensorStart();
				}
				#endif
			}

			//开始静态采集
			else if(pBuff[0] == BRG_START_STATIC_ARR)
			{
				g_nState |= COLLECTOR_STATIC;					//置静态采集标志位
				g_nState &= ~COLLECTOR_DYNAMIC;					//取消动态采集标志位

				//不使用预采集时，开始静态采集时
				#ifndef USE_STATIC_PREPARE
				{
					SensorStart();
				}
				#endif
			}

			//结束静态采集
			else if(pBuff[0] == BRG_END_STATIC)
			{
				g_nState &= ~COLLECTOR_STATIC;					//置静态采集标志位
				SensorStop();									//停止数据采集
			}

			//结束静态采集
			else if(pBuff[0] == BRG_END_STATIC_ARR)
			{
				g_nState &= ~COLLECTOR_STATIC;					//置静态采集标志位
				SensorStop();									//停止数据采集
			}

			//预采集  			
			else if(pBuff[0] == BRG_PREPARE_STATIC)
			{
				g_nState |= COLLECTOR_STATIC;					//置静态采集标志位
				g_nState &= ~COLLECTOR_DYNAMIC;					//取消动态采集标志位

				OS_ENTER_CRITICAL();
				bArrTaskBusy[g_nChannelID] = TRUE;
				OS_EXIT_CRITICAL();

				SensorStart();
			}

			//预采集组  			
			else if(pBuff[0] == BRG_PREPARE_STATIC_ARR)
			{
				g_nState |= COLLECTOR_STATIC;					//置静态采集标志位
				g_nState &= ~COLLECTOR_DYNAMIC;					//取消动态采集标志位

				OS_ENTER_CRITICAL();
				DataInit(bArrTaskBusy, TRUE);
				OS_EXIT_CRITICAL();

				SensorStart();
			}
			
			//读取静态数据
			else if(pBuff[0] == BRG_READ_STATIC)
			{
				//获取数据
				OS_ENTER_CRITICAL();

				*(DT_STORE*)pSendBuff = (DT_STORE)(SensorGetStatic(g_nChannelID) * g_fADStoreParam[g_nChannelID]) 
										- g_sMesureZero[g_nChannelID];

				//设置回传数据长度
				nSendLen += sizeof(DT_STORE);

				//返回电池电量
				*pSendBuff++ = g_nBatteryRate;
				nSendLen += 1;
				OS_EXIT_CRITICAL();

			}

			//读取静态数据组
			else if(pBuff[0] == BRG_READ_STATIC_ARR)
			{
				nPrepareCount = pBuff[1];	

				//读取数据
				OS_ENTER_CRITICAL();
				for(i = 0; i < nPrepareCount; i++)
				{
					//获取指定的通道号
					nTempChannel = pBuff[2+i] - 1;
					nTempChannel = (nTempChannel >= SENSOR_MAX_COUNT) ? (SENSOR_MAX_COUNT-1) : nTempChannel;

					//设置数据有效标志
					*pSendBuff++ = DEVICE_DATA_VALID;
					nSendLen++;

					//获取数据
					*(DT_STORE*)pSendBuff = (DT_STORE)(SensorGetStatic(nTempChannel) * g_fADStoreParam[nTempChannel]) - g_sMesureZero[nTempChannel];
					pSendBuff += sizeof(DT_STORE);
					nSendLen += sizeof(DT_STORE);
	
				}

				//返回电池电量
				*pSendBuff++ = g_nBatteryRate;
				nSendLen += 1;
				
				OS_EXIT_CRITICAL();
			}
			
			//跳出
			break;
		
		//动态数据操作
		case BRG_DYNC_OPERATION:
			if(pBuff[0] == BRG_ENABLE_DYNC)
			{
				//使能动态采集
				bDisableDynamic = FALSE;
			}
			else if(pBuff[0] == BRG_DISABLE_DYNC)
			{
				//禁止动态采集
				bDisableDynamic = TRUE;
			}

			//开始动态采集
			else if(pBuff[0] == BRG_START_DYNC)
			{
				//开始动态采集
				if(bDisableDynamic == TRUE)
				{
					pSendBuff[0] = 0x00;					//回复无效信号
				}
				else
				{
					//获取参数
					nBrgSetSampleRate = *((uint16*)(pBuff + 1));		//记录要设置的采样率参数
					nBrgRefSampleSplt = *((uint16*)(pBuff + 3));		//记录参考采样率
		
					//回复有效信号
					pSendBuff[0] = 0x01;
				}
				
				//设置回传数据长度
				nSendLen += 1; 
			}
			
			//停止动态采集			
			else if(pBuff[0] == BRG_END_DYNC)
			{
				//结束动态采集
				g_nAllowedSample = 0;						//不允许采样
				Timer3Stop();								//关定时器3
				CloseTimer3ISR();							//关定时器3中断
				g_nState &= ~COLLECTOR_DYNAMIC;				//取消动态采集标志位
				SensorStop();								//关传感器
				nFlashErasedSect = 0;						//置Flash擦除段清零
				if(FALSE == bDynEnd)
				{
					bDynEnd = TRUE;
					bDisableDynamic = TRUE;					//禁止动态采集(只有擦除采集器了之后才会响应)
					
					//存储最后一段数据
					SensorDataStore();
				}
			}

			//读取动态数据
			else if(pBuff[0] == BRG_READ_DYNC)
			{
				//用于异步读取动态数据，数据区存放从指定采集次数开始的连续指定个字节数
				nRequireAddr = *((uint32*)(pBuff+1));		//获取主机数据索引 
				nHostReadBytes = pBuff[5];
				DataLoad(pSendBuff, nRequireAddr, nHostReadBytes, g_nChannelID);
				nSendLen += nHostReadBytes;
			}


			//暂停动态采集
			else if(pBuff[0] == BRG_PAUSE_DYNC)
			{
				g_nAllowedSample = 0;						//不允许采样
				Timer3Stop();								//关定时器3
				CloseTimer3ISR();							//关定时器3中断
				SensorStop();								//关传感器
			
				//回复当前索引
				*((uint32*)pSendBuff) = nLocalCount;
				nSendLen += 4; 
			}

			//暂停同步
			else if(pBuff[0] == BRG_PAUSE_SYNC)
			{
				nHostCount = *((uint32*)(pBuff+1));				//主机数据个数
	
				//存储数据==========================
				if(nLocalCount < nHostCount)
				{
					//存储数据
					PostSampleSem();			
				}
			}

			//暂停恢复动态
			else if(pBuff[0] == BRG_PAUSE_RESUME)
			{
				SensorPauseResume();					//开传感器
	
				//开启定时器
				bDynEnd = FALSE;						//取消结束标志
				g_nAllowedSample = 0;					//不允许采样
				Timer3ResetRun();						//开启定时器3
				OpenTimer3ISR();						//开启定时器3中断
	
				//回复有效信号
				pSendBuff[0] = 0x01;
			
				//设置回传数据长度
				nSendLen += 1; 
			}

			break;
			
		//用于同步读取动态数据，数据区存放从指定采集次数开始的连续指定个字节数
		case BRG_READ_DYNAMIC:
			nRequireAddr = *((uint32*)pBuff);				//获取主机数据索引 
			nHostReadBytes = pBuff[4];
			DataLoad(pSendBuff, nRequireAddr, nHostReadBytes, g_nChannelID);
			nSendLen += nHostReadBytes;
			break;
			
		//动态同步信号
		case BRG_SYNC_DYNAMIC:
			//判断是否被禁止=======================================================
			if(bDisableDynamic == TRUE) return FALSE;

			//是否已经开始采集，如果没有则退出=====================================			
			if((g_nState & COLLECTOR_DYNAMIC) != COLLECTOR_DYNAMIC) return FALSE;

			//允许采样
			g_nAllowedSample = TRUE;

			//获取功能字===========================================================
			nRequireCollector = *(uint16*)(pBuff);
			nSyncCode = nRequireCollector & BRG_SYNC_MASK;
			
			//同步和采样数据=======================================================
			if((nSyncCode & SYNC_CALI_TIME) != 0x0000 )
			{
				//主机的索引
				nHostTime = *((uint32*)(pBuff+2));						//获取主机数据索引

				//计算主机次数
				fHostCount = (float)nHostTime;						  	//数据索引(浮点数运算)
				fHostCount *= fBrgSampleDelt;						   	//计算采样索引
				nHostCount = (uint32)fHostCount;						//采样索引(整型)

				//计算空余的时间
				fHostCount = (float)nHostCount;							//当前采集个数
				fHostCount /= fBrgSampleDelt;							//当前采集个数对应的时间(浮点数运算)
				nCycleTimeLeft = (uint32)fHostCount;					//当前采集个数对应的时间(取整)
				nCycleTimeLeft = nHostTime - nCycleTimeLeft;			//空余的时间(100us单位)
				
				//校准定时器========================
				//关定时器3中断以防止异常
				CloseTimer3ISR();	
				
				//定时器3停止
				Timer3Stop();

				//校准定时器3
				//如果采集器快很多
				if(nLocalCount > nHostCount + 1)
				{
					Timer3ResetRun();
				}
				else
				{
					//快一点或慢一些
					Timer3CaliAndRun();
				}

				//存储数据==========================
				if(nLocalCount < nHostCount)
				{
					//存储数据
					PostSampleSem();			
				}

				//开定时器3中断
				OpenTimer3ISR();					
			}

			//是否要回传动态实时数据=====================================================
			nRequireCollector &= COLLECTOR_NET_ID;
			if(GetCollectorLocalID() == nRequireCollector)
			{
				//回传空数据
				if((nSyncCode & SYNC_RETURN_EMPTY) != 0x0000 )
				{
					*(pSendBuff - 2) = BRG_REASON_REPORT;	
					pSendBuff[0] = BRG_REASON_EMPTY;
					nSendLen++;
					break;
				} 

				//解析通道号
				g_nDynChannelID = (*(pBuff + 6) & 0xF0) >> 4;

				//需要回传数据地址
				*(pBuff + 6) &= 0x0F;
				nRequireAddr = *(uint32*)(pBuff+6);
				
#ifdef DEBUG_RECV_SYNC_INFOR
				//关定时器3中断以防止异常
				CloseTimer3ISR();				
				//计算要回传的数据长度
				OS_ENTER_CRITICAL();
				if(nDynLocalAddr[g_nDynChannelID] > nRequireAddr)
				{
					nHostDeviceByteDiff = nDynLocalAddr[g_nDynChannelID] - nRequireAddr;
					if(nHostDeviceByteDiff > UPDATA_AREA_BYTES-8)
					{
						nHostDeviceByteDiff = UPDATA_AREA_BYTES-8;
					}
				}
				else nHostDeviceByteDiff = 0;
				OS_EXIT_CRITICAL();

				//设置校验字段
				*(pSendBuff - 1) = nRequireAddr;
				
				//装载收发信息
				*((uint32*)(pSendBuff+0)) = nRequireAddr;
				*((uint32*)(pSendBuff+4)) = nDynLocalAddr[g_nDynChannelID];

				//装载数据
				nHostReadBytes = nHostDeviceByteDiff;
				pSendBuff[8] = nHostReadBytes;
				DataLoad(pSendBuff+9, nRequireAddr, nHostReadBytes, g_nDynChannelID);	//读数据
				SetSendChannelID(g_nDynChannelID + 1);
				
				//设置回传数据长度
				nSendLen++;					   		//长度字
				nSendLen += nHostReadBytes+8;		//数据区	

				//开定时器3中断
				OpenTimer3ISR();
#else
				//关定时器3中断以防止异常
				CloseTimer3ISR();				
				//计算要回传的数据长度
				OS_ENTER_CRITICAL();
				if(nDynLocalAddr[g_nDynChannelID] > nRequireAddr)
				{
					nHostDeviceByteDiff = nDynLocalAddr[g_nDynChannelID] - nRequireAddr;
					if(nHostDeviceByteDiff > UPDATA_AREA_BYTES)
					{
						nHostDeviceByteDiff = UPDATA_AREA_BYTES;
					}
				}
				else nHostDeviceByteDiff = 0;
				OS_EXIT_CRITICAL();

				//设置校验字段
				*(pSendBuff - 1) = nRequireAddr;
				
				//装载数据
				nHostReadBytes = nHostDeviceByteDiff;
				pSendBuff[0] = nHostReadBytes;
				DataLoad(pSendBuff+1, nRequireAddr, nHostReadBytes, g_nDynChannelID);	//读数据
				SetSendChannelID(g_nDynChannelID + 1);
				
				//设置回传数据长度
				nSendLen++;					   	//长度字
				nSendLen += nHostReadBytes;		//数据区	

				//开定时器3中断
				OpenTimer3ISR();
#endif
			}
			else
			{
				bNeedReply = FALSE;
			}
			
			//退出
			break;

		//擦除Flash，数据区第一字节存放安全码，为0x35
		case BRG_ERASE_FLASH:
			//判断安全码
			if(*((uint16*)pBuff) != BRG_ERASE_FLASH_KEY)
			{
				bNeedReply = FALSE;
				break;
			}

			//获取擦除长度
			nFlashToBeErase = *((uint32*)(pBuff + 2));

			//设置状态
			Timer3Stop();						//关定时器3
			break;

		//校准采集器
		case BRG_CALI_VALUE:
			//校验关键字
			if(*((uint32*)pBuff) != BRG_CALI_KEY)
			{
				bNeedReply = FALSE;
				break;				
			}

			//操作
			if(pBuff[4] == CALI_MODE_SELF)
			{
				//校正量
				TmpFloat = *((fp32*)(pBuff+5));
				TmpFloat = TmpFloat * 1.65f ;
				
				//零点
				TmpFloatZero= *((fp32*)(pBuff+9));
				if(TRUE == g_bLocalSetZero)
				{
					//获取本地调零值
					TmpFloatZero = g_fMesureZeroVol[g_nChannelID];	
				}
				
				//获取当前测量电压值
				fMesureCur = SensorGetStatic(g_nChannelID);
				
				OS_ENTER_CRITICAL();

				//若当前电压值与零点的差值
				if(0 ==(fMesureCur - TmpFloatZero)) fMesureCur = 0.0001f;
				else fMesureCur = (float)(fMesureCur -TmpFloatZero);
				
				//(步进*10000)/0到10000的电压差值
				g_fADStep[g_nChannelID] = (g_fADStep[g_nChannelID] * TmpFloat) / fMesureCur;

				OS_EXIT_CRITICAL();
			}
			else if(pBuff[4] == CALI_MODE_SET)
			{
				//直接设置
				g_fADStep[g_nChannelID] = *((float*)(pBuff+5));
			}
			else if(pBuff[4] == CALI_MODE_FACTORY)
			{
				//恢复出厂参数
				g_fADStep[g_nChannelID] = F_AD_STEP;
			}
			
			//回复新参数
			*((float*)pSendBuff) = g_fADStep[g_nChannelID];
			nSendLen += sizeof(float);
			
			break;

		//发送载波
		case BRG_SEND_CARRY:
			nCarryFreq = *((uint16*)pBuff);
			nCarrySendCycles = *((uint16*)(pBuff+2));
			break;
			
		//默认处理
		default:
			//回复没有对应指令
			*(pSendBuff - 1) = DEVICE_WRONG_ORDER;
			nSendLen += 2;
			return FALSE;
	}
	//-----------------------------------------------------------------------

	//回复主机数据===========================================================
	if(bNeedReply != FALSE)
	{	
		BrgNetSend(NrfTxBuff + BRG_ADDR_LEN, nSendLen);
	}
	//-----------------------------------------------------------------------

	//进行回复后的操作==================================
	switch(nOrder)
	{
		case BRG_CONNECT_DEVICE:
			
			//计算存储系数，及增益系数等参数
			SetStoreType(nCltFunc, fCltK[1], fCltK[0]);

			//调零标志，放在SetStoreType之后，需要参数
			if(nCltCfg & 0x01) 
			{
				//初始化本地调零值
				for(i=0;i<SENSOR_MAX_COUNT;i++)
				{
					g_sMesureZero[i] = (DT_STORE)(g_fMesureZeroVol[i] * g_fADStoreParam[i]);
				}
				
				//调零标志置位
				g_bLocalSetZero = TRUE;
			}
			else 
			{
				//清除调零数值
				DataInitZero(g_sMesureZero);
				
				//调零标志复位
				g_bLocalSetZero = FALSE;
			}
			break;

		//采集器校零，数据区未用
		case BRG_SET_ZERO:
			
			//校验关键字
			if(*((uint32*)pBuff) != BRG_ZERO_KEY) break;
			
			if(pBuff[4] == SET_ZERO_EACH)
			{
				OS_ENTER_CRITICAL();
				bArrTaskBusy[g_nChannelID] = TRUE;
				OS_EXIT_CRITICAL();

				nMemTask = COLLECTOR_ZERO;	
				OSSemPost(pMemEvent);
			}
			else if(pBuff[4] == CANCEL_ZERO_EACH)
			{
				OS_ENTER_CRITICAL();
				ZeroParamRemove(g_nChannelID);
				OS_EXIT_CRITICAL();
			}

			else if(pBuff[4] == SET_ZERO_ARR)
			{
				nMemTask = COLLECTOR_ZERO;	
				OSSemPost(pMemEvent);
			}

			else if(pBuff[4] == CANCEL_ZERO_ARR)
			{
				nPrepareCount = pBuff[5];
				nPrepareCount = (nPrepareCount > SENSOR_MAX_COUNT) ? SENSOR_MAX_COUNT : nPrepareCount;

				for(i = 0; i < nPrepareCount; i++)
				{
					//获取通道号
					nTempChannel = pBuff[6+i] - 1;
					nTempChannel = (nTempChannel >= SENSOR_MAX_COUNT) ? (SENSOR_MAX_COUNT-1) : nTempChannel;

					//取消调零
					OS_ENTER_CRITICAL();
					ZeroParamRemove(nTempChannel);
					OS_EXIT_CRITICAL();	
				}
			}
			break;

		//动态数据操作
		case BRG_DYNC_OPERATION:
			//开始动态采集
			if(pBuff[0] == BRG_START_DYNC)
			{
				if(bDisableDynamic == FALSE)
				{
					//计算采样周期
					fBrgSampleDelt = (float)nBrgSetSampleRate;
					fBrgSampleDelt *= (float)nBrgRefSampleSplt;
					fBrgSampleDelt /= 1000000.0f;
		
					//设置采样率
					SetTimer3Rate(nBrgSetSampleRate);
					
					//更改状态
					nFlashErasedSect = 0;					//Flash被擦除的长度
 					nLocalCount = 0;						//索引字清零
					nHostCount = 0;							//主机索引清零
					DataInitZero(nStoreAddr);				//清除本地存储地址
					DataInitZero(nDynLocalAddr);			//清除本地总地址
					g_nState |= COLLECTOR_DYNAMIC;			//置动态采集标志位
					
					//开传感器
					SensorStart();							//开传感器

					//开启定时器
					bDynEnd = FALSE;						//取消结束标志
					g_nAllowedSample = 0;					//不允许采样
					Timer3ResetRun();						//开启定时器3
					OpenTimer3ISR();						//开启定时器3中断
				}
			}
			break;
			
		//擦除Flash
		case BRG_ERASE_FLASH:
			//判断安全码
			if(*((uint16*)pBuff) != BRG_ERASE_FLASH_KEY) break;

			//停止传感器以防止异常
			SensorStop();

			//响应SensorStop()任务
			OSTimeDly(3);
		 
			//设置存储任务
			nMemTask = BRG_ERASE_FLASH;

			//发送任务信号
			OSSemPost(pMemEvent);

			break;

		//校正
		case BRG_CALI_VALUE :
			//校验关键字
			if(*((uint32*)pBuff) != BRG_CALI_KEY) break;
			
			OS_ENTER_CRITICAL();
			ParamErase();									//擦除参数区
			MainParamStore();								//存储参数
			OS_EXIT_CRITICAL();
			SensorParamCacul();								//重新计算AD系数
			break;
			
		//发送载波
		case BRG_SEND_CARRY : 
			pWirelessDCB->nFreq = nCarryFreq;
			WirelessSetNeedFreq(nCarryFreq);				//设置参数
			WirelessSendCarry(1);
			pWirelessDCB->nFreq = nCurFreq;
			WirelessSetNeedFreq(nCurFreq);					//设置参数
			break;
			
		default : break;
	}
	//---------------------------------------------------------------------
	
	return TRUE;	
}

/****************************************************************************
* 名	称：SensorProcess()
* 功	能：传感器处理程序
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
BOOL SensorProcess() reentrant
{
	uint8 i = 0;
	uint8 nTask;

	//等待传感器开启
	WaitSensorEvent();

	//获取任务(四通道同步采集，将任务综合)
	OS_ENTER_CRITICAL();
	nTask = GetSensorTask();
	SetSensorTask(SENSOR_TASK_NONE);
	OS_EXIT_CRITICAL();

	//启动传感器====================================================================================
	if(nTask == SENSOR_TASK_START)
	{
		//开启AD
		ADOpen();
	
		//参数设置
		for(i=0; i<SENSOR_MAX_COUNT; i++)
		{
			//设置采样率===============================
			if((COLLECTOR_STATIC & g_nState) == COLLECTOR_STATIC)		//静态模式下
			{
				ADSetSampleRate(20, i);
			}
			else									//动态采集模式
			{
				ADSetSampleRate(nBrgSetSampleRate, i);
			}
			
			//设置增益================================
			switch(g_nCurDataType[i].nDataType)
			{
				case DATA_TYPE_2F_128G : 
				case DATA_TYPE_1F_128G :
				case DATA_TYPE_0F_128G :
					ADSetGain(AD_GAIN_SELECT_128, i);		//增益128倍
					break;
				case DATA_TYPE_0F_64G :
					ADSetGain(AD_GAIN_SELECT_64, i);		//增益64倍
					break;
				case DATA_TYPE_0F_32G :
					ADSetGain(AD_GAIN_SELECT_32, i);		//增益32倍
					break;
				case DATA_TYPE_0F_16G :
					ADSetGain(AD_GAIN_SELECT_16, i);		//增益16倍
					break;
				case DATA_TYPE_0F_8G :
					ADSetGain(AD_GAIN_SELECT_8, i);			//增益8倍
					break;
				case DATA_TYPE_0F_4G :
					ADSetGain(AD_GAIN_SELECT_4, i);			//增益4倍
					break;
				case DATA_TYPE_0F_2G :
					ADSetGain(AD_GAIN_SELECT_2, i);			//增益2倍
					break;
				case DATA_TYPE_0F_1G :
					ADSetGain(AD_GAIN_SELECT_1, i);			//增益1倍
					break;
				default : 
					ADSetGain(AD_GAIN_SELECT_1, i);			//增益1倍
					break;
			}
		}
	
		//计算参数
		SensorParamCacul();			
	
		//开始采集======================
		ADStartSample();

		//返回
		return TRUE;
	}
	
	//关闭传感器====================================================================================
	else if(nTask == SENSOR_TASK_STOP)
	{
		//关闭AD
		ADClose();
		
		//返回
		return TRUE;
	}

	//检测传感器====================================================================================
	else if(nTask == SENSOR_TASK_DETECTION)
	{
		//检测采集器外接传感器
		ADSensorDetection(nSensorConnected);

		//设置传感器任务及状态
		DataInitZero(bArrTaskBusy);

		//返回
		return TRUE;
	}

	return TRUE;
}

/****************************************************************************
* 名	称：PostDataStore()
* 功	能：发送存数据信号量
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
uint8 nDynStoreChannel;
void PostDataStore(uint8 nChannelID) reentrant
{
	OS_ENTER_CRITICAL();
	bDataNeedStore[nChannelID] = TRUE;
	OS_EXIT_CRITICAL();

	OSSemPost(pMemEvent);
}

/****************************************************************************
* 名	称：MemEraseProcess()
* 功	能：擦除存储器程序
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
uint8 *pStoreDynBuffTmp;
uint8 nMemTaskCache;				//存储任务缓冲
BOOL MemProcess() reentrant
{
	uint8 nTask = 0;
	uint8 i;

	//无超时等待AD转换完成，或存储信号
	OSSemPend(pMemEvent, 0, &nMemErr);

	//动态数据存储管理================================================================================================
	//获取要存储数据的通道
	OS_ENTER_CRITICAL();
	memcpy(bDataNeedStoreClone, bDataNeedStore, sizeof(bDataNeedStore));
	memset(bDataNeedStore, 0, sizeof(bDataNeedStore)); 
	OS_EXIT_CRITICAL();

	//统计是否有通道需要保存数据
	for(nTask = 0, i = 0; i < SENSOR_MAX_COUNT; i++)
	{
		nTask += bDataNeedStoreClone[i];
	}

	//判断是否需要存储
	if(nTask > 0)
	{
		for(i = 0; i < SENSOR_MAX_COUNT; i++)
		{
			if(bDataNeedStoreClone[i] == FALSE) continue;
			
			//切换到要存储的通道
			nDynStoreChannel = i;	

			//!!!!确保缓冲区大小为存储段大小的整数倍!!!!!!!!!
			pStoreDynBuffTmp = g_nDynBuff[nDynStoreChannel] + g_nStoreSectStart[nDynStoreChannel];
			MemDataStore(pStoreDynBuffTmp, nStoreAddr[nDynStoreChannel], g_nStoreSectLen[nDynStoreChannel], nDynStoreChannel);
			
			OS_ENTER_CRITICAL();
			nStoreAddr[nDynStoreChannel] += g_nStoreSectLen[nDynStoreChannel];
			//g_nStoreSectLen[nDynStoreChannel] = 0;
			OS_EXIT_CRITICAL();
		}

		return TRUE;
	}
	//------------------------------------------------------------------------------------------------------------------

	
	//获取存储任务
	OS_ENTER_CRITICAL();
	nMemTaskCache = nMemTask;
	nMemTask = 0;
	OS_EXIT_CRITICAL();

	//解析任务=========================
	switch(nMemTaskCache)
	{
		//擦除整个零点
		case COLLECTOR_ZERO:
			ZeroParamErase();
			MainZeroParamStore();
			break;

		//擦除数据区
		case BRG_ERASE_FLASH:

			bDisableDynamic = FALSE;					//使能动态采集
			
			//判断要求的存储段是否已经擦除
			if(nFlashErasedSect < nFlashToBeErase)
			{
				//擦除指定扇区的数据
				MemorizerEraseToSect(nFlashErasedSect, nFlashToBeErase);
	
				//记录信息
				nFlashErasedSect = nFlashToBeErase;		//记录擦除长度
			}
			
			//指示灯显示
			PostLightOn(20);
			break;
		default : break;
	}

	OS_ENTER_CRITICAL();
	bArrTaskBusy[g_nChannelID] = FALSE;
	OS_EXIT_CRITICAL();

	
	//返回
	return TRUE;
}

/****************************************************************************
* 名	称：SampleProcess()
* 功	能：AD任务
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
BOOL SampleProcess() reentrant
{
	//无超时等待信号量=================================
	WaitSampleEvent();
	
	//计算需要存储的数据个数
	nPushDynNum = 1;
	if(nHostCount > nLocalCount) 
	{
		nPushDynNum = nHostCount - nLocalCount;
	}
	
	//存储动态数据
	SensorPushDynamic(nPushDynNum);	
	
	//返回
	return TRUE;  
}

//定时器3中断相应程序(数据采样定时器)
void Timer3ISR() interrupt 14
{
	//进中断
	OSIntEnter();

	//清除中断标志
	ClearTimer3ISR();
	
	//定时器分段数加一
	Timer3Counter++;

	//是否到达采样时间
	if(Timer3Counter >= Timer3CounterMax)
	{
		//定时器分段清零
		Timer3Counter = 0;

		//采样数据
		if(g_nAllowedSample == TRUE)
		{
			PostSampleSem();
		}
	}

	//出中断
	OSIntExit();
}





/****************************************************************************
* 名	称：SensorDetectionAndShow()
* 功	能：检查传感器并用LED显示检查传感器的结果
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
#ifdef	POWER_ON_SENSOR_DETECTION 
void SensorDetectionAndShow() reentrant
{
	unsigned char i,j;

	//检测采集器外接传感器
	ADSensorDetection(nSensorConnected);

	for(i=0;i<SENSOR_MAX_COUNT;i++)
	{
		for(j = 0; j < (i+1); j++)
		{
			LightOn();
			OSTimeDly(2);

			LightOff();
			OSTimeDly(20);
		}  

		//停时拍
		OSTimeDly(50);

		//检测到传感器
		if(TRUE == nSensorConnected[i])
		{
			LightOn();
			OSTimeDly(50);
			LightOff();
			OSTimeDly(50);
		}

		//没检测到传感器
		else
		{
			//对应检测到的时间
			LightOff();
			OSTimeDly(50);
			LightOff();
			OSTimeDly(50);
		}	
	}
}
#endif
