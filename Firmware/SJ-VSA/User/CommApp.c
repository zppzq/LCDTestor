/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: CommApp.c
**创   建   人: 杨承凯
**创 建 日  期: 2007年09月18日
**最后修改日期: 2007年09月18日
**描        述: 通信应用源文件
*****************************************************************************************************************/
#define _USERCOMM_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "DataManage.h"
#include "CommApp.h"
#include "DataAcq.h"
#include "ColBusSlaveMac.h"
#include "Bsp\LowPower.h"
#include "SyncTimer.h"


//defines
#define COLLECTOR_MODEL			201

#define USE_STATIC_PREPARE

#define ENABLE_LACAL_ZEROED		1					//本地调零使能
#define ENABLE_CARRY_SEND		1					//载波使能

//超时时间定义
#define SLEEP_RECV_TIMEOUT		500					//休眠时接收超时时间，单位为ms
#define NORMAL_RECV_TIMEOUT		5000				//常态下接收超时时间，单位为ms


//休眠时间控制，此参数乘以NORMAL_RECV_TIMEOUT为休眠前时间============================
#define	SLEEP_POINT			60

//-----------------------------------------------------------------------------------	
//全局变量===========================================================================
uint16 g_nSleepPoint = SLEEP_POINT;
uint16 g_nSleepTicketMax = 1;


//4432配置信息
#ifdef _SI4432_H_

//无线设备描述符
WIRELESSDCB *pWirelessDCB;							//无线设备指针

//频率参数
uint16 nDefaultFreq;								//默认频率
uint16 nCurFreq;									//当前频率
uint16 nCarryFreq;									//当前载波频率
uint16 nCarrySendCycles;							//载波发送周期
#endif


//外部参数声明==================================

extern uint16	g_nSleepPoint;					    //多久之后休眠(单位为5S)
extern uint16	g_nSleepTicketMax;					//休眠时间为多久(单位为5S)

extern uint32 g_nStoreSectStart[DATA_ACQ_COUNT];				//当前缓冲段的开始索引
extern uint32 g_nStoreSectLen[DATA_ACQ_COUNT];					//当前缓冲段的长度

extern float idata g_fMesureZeroVol[DATA_ACQ_COUNT];			//存储到存储器中的调零电压
extern DT_STORE idata g_sMesureZero[DATA_ACQ_COUNT];			//调零数据，用于计算的参数


//休眠控制
uint16 nSleepRecvTimout = SLEEP_RECV_TIMEOUT;		//休眠时接收超时时间，单位为ms
uint16 nNormalRecvTimeout = NORMAL_RECV_TIMEOUT;	//常态下接收超时时间，单位为ms





/****************************************************************************
* 名	称：SetNormalRecvTimeOut()
* 功	能：设置正常的接收超时时间
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void SetNormalRecvTimeOut() reentrant
{
	//注意这个接收时间给采集器休眠时间有关
#ifdef _SI4432_H_
	WirelessSetRecvTimeOut(nNormalRecvTimeout);
#endif
	
#ifdef _USE_COMM2_
	SetCommRecvTimeOut(2, nNormalRecvTimeout);
#endif
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
	//注意这个接收时间给采集器休眠时间有关
#ifdef _SI4432_H_
	WirelessSetRecvTimeOut(nSleepRecvTimout);
#endif

#ifdef _USE_COMM2_
	SetCommRecvTimeOut(2, nSleepRecvTimout);
#endif

}

/****************************************************************************
* 名	称：SetFreqToDefault()
* 功	能：频率设置为默认频率
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_H_
void SetFreqToDefault() reentrant
{
	//配置为默认频率
	WirelessSetNeedFreq(nDefaultFreq);
}
#endif

/****************************************************************************
* 名	称：SetFreqToCur()
* 功	能：频率设置为当前频率
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef _SI4432_H_
void SetFreqToCur() reentrant
{
	//频率配置
	WirelessSetNeedFreq(nCurFreq);
}
#endif

/***************************************************************************
* 名	称：UserCommInit()
* 功	能：通信应用层初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/  
void UserCommInit() reentrant
{
	//设置通信参数
#ifdef _SI4432_H_
	pWirelessDCB = Si4432GetDCB();
	nDefaultFreq = FREQ_DEVICE_DEFAULT;		//设置默认频率
	nCurFreq = nDefaultFreq;				//当前为默认频率
	pWirelessDCB->nFreq = nDefaultFreq;		//通信频率设置
#endif

	//设置常规超时时间
	SetNormalRecvTimeOut();

	//ColBus初始化
	CBSlaveApsInit();
}


/***************************************************************************
* 名	称：CommProcess()
* 功	能：通信任务处理
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
//休眠唤醒控制
int16	nWakeCount = 0;
BOOL	bSleep = FALSE;

//休眠控制
uint16 g_nSleepIndex;
void CommProcess(void)
{
	int16 nRecvLen;

	//接收无线数据
	nRecvLen = CBSlaveApsFsm();

	//如果收到数据，执行通信协议
	if(nRecvLen > 0)
	{
		//信号指示
		PostLightOn(1);

		//清除连续未接收数据的次数
		nWakeCount = 0;

		//关闭睡眠标志
		if(bSleep == TRUE)
		{
			SetNormalRecvTimeOut();
			bSleep = FALSE;
		}

		//设置到当前频率
		#ifdef _SI4432_H_
		SetFreqToCur();
		#endif

		return;				 
	}

	//如果接收超时，统计超时次数=====================================================
	if(nRecvLen  == -1)
	{
 		//如果没收到数据，统计无接收次数=============================================
		if(bSleep == FALSE)
		{
			nWakeCount++;
			if(nWakeCount >= g_nSleepPoint)
			{
				//设置休眠时的超时时间
				SetSleepRecvTimeOut();			
	
				#ifdef _SI4432_H_
				//设置到默认频率
				SetFreqToDefault();
				#endif
	
				//进入休眠状态
				bSleep = TRUE;
			}
		}

		//如果是休眠状态，则休眠5秒钟================================================
		if(bSleep == TRUE)
		{
			#ifdef _SI4432_H_
			//电源控制，准备进入休眠模式
			Si4432Standby();
			#endif
			
			PowerLightOff();
			SigLightOff();

			#ifdef _MCU_ENTER_SLEEP_
			MCUEnterSleep();
			#endif
			
			//进入休眠模式
			g_nSleepIndex = g_nSleepTicketMax;
			while(g_nSleepIndex--)
			{
				MCUSleep();
			}

			#ifdef _MCU_ENTER_SLEEP_
			MCUExitSleep();	 
			#endif

			#ifdef _SI4432_H_
			//恢复休眠前状态
			Si4432Ready();
			#endif

			//闪一下指示灯
			PostLightOn(1);
		}
	} 
}



