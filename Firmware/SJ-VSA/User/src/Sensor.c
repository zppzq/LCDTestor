/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: Sensor.c
**创   建   人: 杨承凯
**创 建 日  期: 2007年09月18日
**最后修改日期: 2007年10月18日
**描        述: 传感器源文件
*****************************************************************************************************************/
#define 	_SENSOR_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "Sensor.h"
//#include "DataManage.h"
#include "UserComm.h"

//编译控制========================================================================================================
#define FILTER_BUFF_MODE_NON		1
#define FILTER_BUFF_MODE_SUM		2
#define FILTER_BUFF_MODE_QUEUE		3

#define FILTER_BUFF_MODE 			FILTER_BUFF_MODE_QUEUE
#define USE_PLAN_DATA		  		//用于防止AD更新不及时，数据就看起来是平的
#define USE_MACRO_TO_SPEED


//signal defines--------------------------------------------------------------------------------------------------

//应变桥电源参数计算描述
//计算AD结果每个步长代表的微伏数
//令AD读数为N
//
//					   2.5V * 10^6 
//FKBRGVOT_DEFAUT = -------------------
//						   2^23
//计算得：
//#define FKBRGVOT_DEFAUT		0.298f				//计算微伏时的理论计算参数
//#define FKBRGVOT_DEFAUT		0.2896097f			//计算微伏时的实测合理参数(因为PGA的128倍并不是完全精确)
//则：ΔV = N * FKBRGVOT_DEFAUT
//
//			  S*ΔV
//因为：1ε= -------- ，(全桥时S = 1，半桥时S = 2，四分之一桥时S = 4)
//			  K * U
//又：U = 3.3V
//则：FKBRGVOT_DEFAUT = FKBRGVOT_DEFAUT / 3.3 = 0.0877605f
//此时：
//			  N * S * FKBRGVOT_DEFAUT
//		ε= --------------------------- ，(全桥时S = 1，半桥时S = 2，四分之一桥时S = 4)
//			             K
//#define FKBRGVOT_DEFAUT 		0.0877605f

//****************************************************************************************************************
//理论计算的AD步进
#define	AD_DEFUAT_STEP		 0.298f


//内部变量定义
static INT8U Sensorerr;
static OS_EVENT *pSensorEvent;
static BOOL bSensorOpen;

//传感器任务
static uint8 nSensorTaskInner = SENSOR_TASK_NONE;

//计算系数===============================================
//g_fADParam = g_fADVolParam * g_fADStoreParam
//AD总系数
float g_fADParam[SENSOR_MAX_COUNT];		

//AD计算电压系数
float g_fADVolParam[SENSOR_MAX_COUNT];

//定义用于记录当前AD步进参数，记录到flash中
float	g_fADStep[SENSOR_MAX_COUNT];

//----------------------------------------------------------------------------


//数据缓冲
uint8 nDynIndex[SENSOR_MAX_COUNT];					//数据索引
uint8 nCurSectStart[SENSOR_MAX_COUNT];				//当前缓冲段的开始索引
uint8 nCurSectLen[SENSOR_MAX_COUNT];				//当前缓冲段的长度
uint8 g_nStoreSectStart[SENSOR_MAX_COUNT];			//当前缓冲段的开始索引
uint8 g_nStoreSectLen[SENSOR_MAX_COUNT];			//当前缓冲段的长度
int8 g_nDynBuff[SENSOR_MAX_COUNT][DYN_BUFF_LEN];	//动态数据缓冲区


static uint16 nDynCount[SENSOR_MAX_COUNT];			//动态数据个数
int8 *pCurDynBuff;									//当前操作地址
int8 *pStoreDynBuff;								//待存储缓冲区地址

//声明外部变量===========================================
extern uint32  nLocalCount;				//本地存储计数值

extern uint8 g_nADCurGain[SENSOR_MAX_COUNT];		//记录当前增益

float g_fADStoreParam[SENSOR_MAX_COUNT];		//AD存储系数

extern uint32 nDynLocalAddr[SENSOR_MAX_COUNT];		//数据存储区地址长度

extern BOOL bDataNeedStore[SENSOR_MAX_COUNT];		//需要存储数据的通道

extern BOOL bArrTaskBusy[SENSOR_MAX_COUNT];			//任务忙标志

uint8 g_nMeasureType[SENSOR_MAX_COUNT];				//采集器测量

/*****************************************************************************************************************
* 名	称：SensorInit()
* 功	能：传感器初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/

/*****************************************************************************************************************
* 名	称：WaitSensorStart()
* 功	能：等待打开传感器
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_SENSOR_START_EN_
void WaitSensorEvent()
{
	OSSemPend(pSensorEvent, 0, &Sensorerr);
}
#endif

/*****************************************************************************************************************
* 名	称：SetSensorTask()
* 功	能：设置传感器任务
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SetSensorTask(uint8 nTask)
{
	nSensorTaskInner = nTask;
}

/*****************************************************************************************************************
* 名	称：GetSensorTask()
* 功	能：获取传感器任务
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
uint8 GetSensorTask() 
{
	return nSensorTaskInner;
}

/*****************************************************************************************************************
* 名	称：SensorParamCacul()
* 功	能：计算AD参数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SensorParamCacul()
{
	uint8 i;
	for(i=0;i<SENSOR_MAX_COUNT;i++)
	{
		if(g_nMeasureType[i] == MEASURE_STRAIN)
		{
			//如测量应变，则用校正系数计算AD电压系数
			g_fADVolParam[i] = g_fADStep[i] / (fp32)(0x01 << g_nADCurGain[i]);
		}
		else 
		{
			//若测量电压，则使用理论步进计算AD电压系数
			g_fADVolParam[i] = AD_DEFUAT_STEP / (fp32)(0x01 << g_nADCurGain[i]);
		}

		//计算AD总系数
		g_fADParam[i] = g_fADStoreParam[i] * g_fADVolParam[i];
	}
}
