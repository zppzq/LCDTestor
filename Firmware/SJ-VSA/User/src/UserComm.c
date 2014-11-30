#include "includes.h"
#include "ADS1246.h"
#include "Sensor.h"
#include "UserComm.h"
#include "CpuPortAccess.h"
#include <ucos_ii.h>

#define COLLECTOR_STATIC 0

uint8 g_nState = 0;							//采集器状态字
uint16 nBrgSetSampleRate;							//要设置的采样率

//运行参数
STORE_DATA_TYPE_S g_nCurDataType[SENSOR_MAX_COUNT];	//记录当前要保存的数据类型

//记录是否外接传感器标志
uint8 nSensorConnected[SENSOR_MAX_COUNT];
//当前状态
BOOL bArrTaskBusy[SENSOR_MAX_COUNT];				//任务忙标志

//函数定义
#define	ADPortInit()		ADS1246PortInit()
#define	ADVariInit()		ADS1246VariInit()
#define	ADOpen()			ADS1246Open()
#define	ADClose()			ADS1246Close()
#define	ADSetSampleRate(x,y)	ADS1246SetSampleRate(x,y)
#define	ADSetRange(x,y)		ADS1246SetRange(x,y)
#define	ADSetGain(x,y)		ADS1246SetGain(x,y)
#define	ADStartSample()		ADS1246StartSample()		
#define	ADStopSample()		ADS1246StopSample()
#define	ADGetVol(x)			ADS1246GetVol(x)
#define	ADParamUpdate(x,y)	ADS1246SetParam(x,y)		//x:参数；y:通道
#define	ADWaitDataReady()	ADS1246WaitDataReady()
#define	ADSensorDetection(x)	ADS1246SensorDetection(x)
/****************************************************************************
* 名	称：SensorProcess()
* 功	能：传感器处理程序
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
BOOL SensorProcess()
{
	uint8 i = 0;
	uint8 nTask;
	uint8 *pTask = &nTask;
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//等待传感器开启
	WaitSensorEvent();

	//获取任务(四通道同步采集，将任务综合)
	OS_ENTER_CRITICAL();
	nTask = 1;//GetSensorTask();
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