/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: DataAcq.c
**创   建   人: 杨承凯
**创 建 日  期: 2007年09月18日
**最后修改日期: 2007年10月18日
**描        述: 数据采集源文件
*****************************************************************************************************************/
#define 	_DATA_ACQ_C_

//includes--------------------------------------------------------------------------------------------------------
#include "includes.h"
#include "DataManage.h"
#include "DataAcq.h"
#include "CommApp.h"

//使用宏来执行动态采集代码
#define USE_MACRO_TO_SPEED


//数据采集任务定义
#define DATA_ACQ_TASK_NONE			0		
#define DATA_ACQ_TASK_START			1
#define DATA_ACQ_TASK_STOP			2
#define DATA_ACQ_TASK_DETECTION		3
#define DATA_ACQ_TASK_PREPARE		4


//测点测量方式
#define	MEASURE_VOL			0x01		//测量电压，使用AD理论步进
#define	MEASURE_STRAIN		0x02		//测量应变，使用校正步进


//动态数据采集的缓冲区大小控制
#define	DYN_BUFF_LEN		NAND_PAGE_SIZE*2		//动态数据缓冲区大小
#define	DYN_STORE_THRES		NAND_PAGE_SIZE			//数据存储阀值

#define DYN_BUFF_LEN_MASK	0x00000FFF
#define DYN_SECT_LEN_MASK	0x000007FF


//数据定义========================================================================================================

//应变桥电源参数计算描述
//计算AD结果每个步长代表的微伏数
//令AD读数为N
//
//					   2.5V * 10^6 
//FKBRGVOT_DEFAUT = -------------------
//						   2^23
//计算得：
//#define FKBRGVOT_DEFAUT		0.298023f				//计算微伏时的理论计算参数
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


//事件
static OS_EVENT *pDataAcqEvent;			//数据采样事件
static OS_EVENT *pDataStoreEvent;		//数据存储事件
static OS_EVENT* pDataStoreDiskSem; 	//数据存盘信号


//传感器任务
static uint8 nDataAcqTaskInner;

//PGA增益为1时AD步进量代表的微伏数
float g_fADStep[DATA_ACQ_COUNT];

//PGA增益为设置值时AD步进量代表的微伏数(g_fADStep除以增益)
float g_fADVolParam[DATA_ACQ_COUNT];

//将电压换算为目标量的系数
float g_fADStoreParam[DATA_ACQ_COUNT];
														   
//运算为上位机需要的数据的总系数(如换算为应变或计入取整系数)
//g_fADParam = g_fADVolParam * g_fADStoreParam;
float g_fADParam[DATA_ACQ_COUNT];

//记录当前要保存的数据类型
STORE_DATA_TYPE_S g_nCurDataType[DATA_ACQ_COUNT];	

//端点的测量方式
uint8 g_nMeasureType[DATA_ACQ_COUNT];


//采样率
uint16 nBrgSetSampleRate;							//要设置的采样率
float fBrgSetSampleRate;

//动态数据采集信息
uint32 nLocalCount;									//本地存储计数值
uint32 nDynLocalAddr[DATA_ACQ_COUNT];				//数据存储区地址长度

int8 g_nDynBuff[DATA_ACQ_COUNT][DYN_BUFF_LEN];		//动态数据缓冲区
uint32 nDynIndex[DATA_ACQ_COUNT];					//数据索引
uint32 nCurSectStart[DATA_ACQ_COUNT];				//当前缓冲段的开始索引
uint32 nCurSectLen[DATA_ACQ_COUNT];					//当前缓冲段的长度
uint32 g_nStoreSectStart[DATA_ACQ_COUNT];			//当前缓冲段的开始索引
uint32 g_nStoreSectLen[DATA_ACQ_COUNT];				//当前缓冲段的长度


BOOL bDataNeedStore[DATA_ACQ_COUNT];				//需要存储数据的通道
BOOL bDataNeedStoreClone[DATA_ACQ_COUNT];			//需要存储数据的通道
uint32 nStoreAddr[DATA_ACQ_COUNT];					//已存储数据计数

//零点参数
float g_fMesureZeroVol[DATA_ACQ_COUNT];				//存储到存储器中的调零电压
DT_STORE g_sMesureZero[DATA_ACQ_COUNT];				//调零数据，用于计算的参数

//记录是否外接传感器标志
uint8 nDataAcqConnected[DATA_ACQ_COUNT];  


//采集器状态字
uint8 g_nState = 0;	

//任务忙标志
uint8 g_bTaskBusy;


//声明外部变量===========================================
extern uint32 nHostCount;							//远程存储计数值


/*****************************************************************************************************************
* 名	称：DataStoreInit()
* 功	能：数据存储初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_INIT_EN_
void DataStoreInit() reentrant
{
	//创建数据存储信号
	pDataStoreEvent = OSSemCreate(0);  

	//创建数据存盘信号
	pDataStoreDiskSem = OSSemCreate(0);
}
#endif



/*****************************************************************************************************************
* 名	称：DataAcqInit()
* 功	能：数据采集初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_INIT_EN_
void DataAcqInit() reentrant
{
	//创建数据采集信号量
	pDataAcqEvent = OSSemCreate(0);	 

	//数据采集任务
	nDataAcqTaskInner = DATA_ACQ_TASK_NONE;

	//状态初始化
	g_bTaskBusy = FALSE;	
}
#endif


/*****************************************************************************************************************
* 名	称：DataAcqParamInit()
* 功	能：数据采集参数初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void DataAcqParamInit(void) reentrant
{
	uint8 i;

	//默认采样率
	nBrgSetSampleRate = 20;

	//设置默认参数
	for(i=0; i < DATA_ACQ_COUNT; i++)
	{
		g_fADStep[i] = F_AD_STEP;
		g_fMesureZeroVol[i] = 0;
		g_sMesureZero[i] = 0;


		//默认存电压
		g_nMeasureType[i] = MEASURE_VOL;
		g_nCurDataType[i].nStoreType = STORE_TYPE_UV;
		g_nCurDataType[i].nDataType = DATA_TYPE_0F_128G;
	}

	//清零的参数
	DataInitZero(bDataNeedStore);
	DataInitZero(g_fADStoreParam); 
}

/*****************************************************************************************************************
* 名	称：OnHostParamSet()
* 功	能：主机设置参数
* 入口参数：存储类型:是否要计算成应变，fSense:灵敏度系数。fRange:量程。
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
void OnHostParamSet(uint8 nEp, uint8 nType, fp32 fSense, fp32 fRange) reentrant
{
	uint8 tmp1,tmp2;
	
	//判断低4位，确定存储数据格式
	tmp1 = nType;

	//要求存储上传电压值==================
	if(5 == tmp1) 		
	{
		g_nMeasureType[nEp] = MEASURE_VOL;		//测量电压

		g_fADStoreParam[nEp] = 1.0f;

		//根据电压值设置存储类型
		if(fRange <= 301) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 100.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 601) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 50.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 1501) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 20.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 3001)
		{
			tmp2 = DATA_TYPE_1F_128G;
			g_fADStoreParam[nEp] = 10.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 19532) tmp2 = DATA_TYPE_0F_128G;
		else if(fRange <= 30001) tmp2 = DATA_TYPE_0F_64G;
		else if(fRange <= 39062) 
		{
			tmp2 = DATA_TYPE_0F_64G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 78126) 
		{
			tmp2 = DATA_TYPE_0F_32G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 156251) 
		{
			tmp2 = DATA_TYPE_0F_16G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 312501) 
		{
			tmp2 = DATA_TYPE_0F_8G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}			
		else if(fRange <= 625001) 
		{
			tmp2 = DATA_TYPE_0F_4G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}			
		else if(fRange <= 1250001) 
		{
			tmp2 = DATA_TYPE_0F_2G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}			
		else 
		{
			tmp2 = DATA_TYPE_0F_1G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}			
	}

	//存储上传应变值
	else		
	{
		g_nMeasureType[nEp] = MEASURE_STRAIN;		//测量应变

		//全桥
		if(1 == tmp1) g_fADStoreParam[nEp] = 1.0f / (3.3f * fSense);

		//半桥
		else if(2 == tmp1) g_fADStoreParam[nEp] = 1.0f / (1.65f * fSense);

		// 1/4桥
		else if(4 == tmp1) g_fADStoreParam[nEp] = 1.0f / (0.825f * fSense);

		//根据应变值设置存储类型
		if(fRange <= 101) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 100.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 201) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 50.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 501) 
		{
			tmp2 = DATA_TYPE_2F_128G;
			g_fADStoreParam[nEp] = 20.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 2001)
		{
			tmp2 = DATA_TYPE_1F_128G;
			g_fADStoreParam[nEp] = 10.0f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 10001) tmp2 = DATA_TYPE_0F_128G;
		else if(fRange <= 20001) tmp2 = DATA_TYPE_0F_64G;
		else if(fRange <= 40001) 
		{
			tmp2 = DATA_TYPE_0F_32G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 80001) 
		{
			tmp2 = DATA_TYPE_0F_16G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 160001) 
		{
			tmp2 = DATA_TYPE_0F_8G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 320001) 
		{
			tmp2 = DATA_TYPE_0F_4G;
			g_fADStoreParam[nEp] = 0.1f * g_fADStoreParam[nEp];
		}
		else if(fRange <= 640001) 
		{
			tmp2 = DATA_TYPE_0F_2G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}
		else 
		{
			tmp2 = DATA_TYPE_0F_1G;
			g_fADStoreParam[nEp] = 0.01f * g_fADStoreParam[nEp];
		}
	}

	//数据类型
	g_nCurDataType[nEp].nDataType = tmp2;
}


/*****************************************************************************************************************
* 名	称：DataAcqParamCacul()
* 功	能：计算AD参数
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void DataAcqParamCacul() reentrant
{
	uint8 i;

	for(i = 0; i < DATA_ACQ_COUNT; i++)
	{
		if(g_nMeasureType[i] == MEASURE_STRAIN)
		{
			//如测量应变，则用校正系数计算AD电压系数
			g_fADVolParam[i] = g_fADStep[i] / (fp32)(0x01 << ADGetGain(i));
		}
		else 
		{
			//若测量电压，则使用理论步进计算AD电压系数
			g_fADVolParam[i] = F_AD_STEP / (fp32)(0x01 << ADGetGain(i));
		}

		//计算AD总系数
		g_fADParam[i] = g_fADStoreParam[i] * g_fADVolParam[i];

		//计算零点
		g_sMesureZero[i] = (DT_STORE)(g_fMesureZeroVol[i] * g_fADStoreParam[i]);
	}
}


/*****************************************************************************************************************
* 名	称：DataAcqParamStore()
* 功	能：存储传感器参数
* 入口参数：开始字节地址
* 出口参数：新字节地址
* 说	明：无
*****************************************************************************************************************/
#ifdef	_MAIN_PARAM_STORE_
void DataAcqParamStore() reentrant
{
	uint8 i;
	for(i = 0; i < DATA_ACQ_COUNT; i++)
	{
		g_TerminalParam.arrADStep[i] = g_fADStep[i];
		g_TerminalParam.arrZeroVol[i] = g_fMesureZeroVol[i];
	}
}
#endif

/*****************************************************************************************************************
* 名	称：DataAcqParamLoad()
* 功	能：读取传感器参数
* 入口参数：开始字节地址
* 出口参数：新字节地址
* 说	明：无
*****************************************************************************************************************/
#ifdef	_MAIN_PARAM_STORE_
void DataAcqParamLoad() reentrant
{
	uint8 i;
	for(i = 0; i<DATA_ACQ_COUNT; i++)
	{
		g_fADStep[i] = g_TerminalParam.arrADStep[i];
		g_fMesureZeroVol[i] = g_TerminalParam.arrZeroVol[i];
	}
}
#endif


/*****************************************************************************************************************
* 名	称：PostDataAcqPro()
* 功	能：打开传感器任务
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_POST_DATA_ACQ_PRO_
void PostDataAcqPro() reentrant
{
	OSSemPost(pDataAcqEvent);
}
#endif

/*****************************************************************************************************************
* 名	称：WaitDataAcqStart()
* 功	能：等待打开传感器
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_START_EN_
void WaitDataAcqEvent() reentrant
{
	uint8 nErr;
	OSSemPend(pDataAcqEvent, 0, &nErr);
}
#endif

/*****************************************************************************************************************
* 名	称：SetDataAcqTask()
* 功	能：设置传感器任务
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
void SetDataAcqTask(uint8 nTask) reentrant
{
	nDataAcqTaskInner = nTask;
}

/*****************************************************************************************************************
* 名	称：GetDataAcqTask()
* 功	能：获取传感器任务
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
uint8 GetDataAcqTask() reentrant
{
	return nDataAcqTaskInner;
}

/*****************************************************************************************************************
* 名	称：DataAcqStop()
* 功	能：打开传感器
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_STOP_EN_
void DataAcqStop() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//重载开始***************************************
	//关闭传感器
	OS_ENTER_CRITICAL();
	nDataAcqTaskInner = DATA_ACQ_TASK_STOP;
	OS_EXIT_CRITICAL();
	
	//发送信号
	pDataAcqEvent->OSEventCnt = 0;
	OSSemPost(pDataAcqEvent);

	//重载结束***************************************
}
#endif






//
//静态采集数据信息
//
#define STATIC_NOISE_COUNT	4
#define STATIC_BUFF_LEN		8
static int32 nArrStaticData[DATA_ACQ_COUNT][STATIC_BUFF_LEN];
static int32 nArrStaticDataSum[DATA_ACQ_COUNT];
static uint8 nArrStaticIndex[DATA_ACQ_COUNT];
static uint8 nArrStaticNoiseIndex[DATA_ACQ_COUNT];
static BOOL bStaticEnd[DATA_ACQ_COUNT];


/*****************************************************************************************************************
* 名	称：DataAcqResetStaticIndex()
* 功	能：复位静态采集序列
* 入口参数：无
* 出口参数：无
* 说	明：让每次静态采集存储从0开始
*****************************************************************************************************************/
void DataAcqResetStaticIndex() reentrant
{
	//初始化静态采集信息
	DataInitZero(nArrStaticIndex);
	DataInitZero(nArrStaticNoiseIndex);
	DataInit(bStaticEnd, FALSE);
}

/*****************************************************************************************************************
* 名	称：DataAcqSetStatic()
* 功	能：获取传感器静态数据
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_STATIC_EN_
void DataAcqSetStatic(int32 nValue, uint8 nChannelID) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//检查参数
	if(nChannelID >= DATA_ACQ_COUNT) return;

	//判断是否全部转换完毕，如果完毕则关传感器电源，停止采集
	if(bStaticEnd[0] & bStaticEnd[1] & bStaticEnd[2] & bStaticEnd[3])
	{
		ADS1246StopStatic();
		g_bTaskBusy = FALSE;
		return;
	}

	//过滤最开始不稳定的数据
	if(nArrStaticNoiseIndex[nChannelID] < STATIC_NOISE_COUNT)
	{
		nArrStaticNoiseIndex[nChannelID]++;
		return;
	}

	//进入临界区
	OS_ENTER_CRITICAL();
	
	//缓存数据
	if(nArrStaticIndex[nChannelID] < STATIC_BUFF_LEN) 
	{
		nArrStaticData[nChannelID][nArrStaticIndex[nChannelID]] = nValue;
		nArrStaticIndex[nChannelID]++;
	}

	//判断缓存是否完成
	if(nArrStaticIndex[nChannelID] >= STATIC_BUFF_LEN) 
	{
		nArrStaticIndex[nChannelID] = 0;	
		bStaticEnd[nChannelID] = TRUE;
	}

	//退出临界区
	OS_EXIT_CRITICAL();		
}
#endif


/*****************************************************************************************************************
* 名	称：DataAcqGetStatic()
* 功	能：获取传感器静态数据
* 入口参数：无
* 出口参数：电压值
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_STATIC_EN_
float DataAcqGetStatic(uint8 nChannelID) reentrant
{
	uint8 i;

	//检查有效性
	if(nChannelID >= DATA_ACQ_COUNT) return 0;

	//求AD和
	nArrStaticDataSum[nChannelID] = 0;
	for(i = 0; i < STATIC_BUFF_LEN; i++)
	{
		nArrStaticDataSum[nChannelID] += nArrStaticData[nChannelID][i];
	}

	//求AD均值
	nArrStaticDataSum[nChannelID] /= STATIC_BUFF_LEN;

	//返回电压值
	return ((float)nArrStaticDataSum[nChannelID]) * g_fADVolParam[nChannelID];	  
}
#endif








//
//动态采集数据信息
//
static int32 SYS_OPT_SEG nDynSum[DATA_ACQ_COUNT];				//动态数据和
static int16 SYS_OPT_SEG nDynCurGetStore[DATA_ACQ_COUNT];

/*****************************************************************************************************************
* 名	称：DataAcqSetDynamic()
* 功	能：放入堆栈缓冲区中
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_ 
#ifndef USE_MACRO_TO_SPEED	 
void DataAcqSetDynamic(int32 nValue, uint8 nChannelID) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//缓存数据
	OS_ENTER_CRITICAL();
	nDynSum[nChannelID] = nValue; 	  
	OS_EXIT_CRITICAL();	
}
  
#else

//使用宏代替上面的函数以提高运行速度
#define DataAcqSetDynamic(nValue, nChannelID)								   	\
{																				\
	OS_ENTER_CRITICAL();														\
	nDynSum[nChannelID] = nValue; 												\
	OS_EXIT_CRITICAL();															\
}
#endif	//USE_MACRO_TO_SPEED

#endif	//_DATA_ACQ_GET_DYNAMIC_EN_



/*****************************************************************************************************************
* 名	称：DataAcqGetDynamic()
* 功	能：获取传感器静态数据
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
#ifndef USE_MACRO_TO_SPEED
DT_STORE DataAcqGetDynamic(uint8 nChannelID) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif													 

	//从AD值计算到最终数值
	OS_ENTER_CRITICAL();  
	nDynamicAvrg32 = nDynSum[nChannelID];  
	nDynCurGetStore[nChannelID] = (DT_STORE)((float)nDynamicAvrg32 * g_fADParam[nChannelID]);
	OS_EXIT_CRITICAL();
	
	//减去零点值
	nDynCurGetStore[nChannelID] -= g_sMesureZero[nChannelID];	
	
	//返回结果
	return nDynCurGetStore[nChannelID];	
}
#else

//使用宏代替上面的函数以提高运行速度
#define DataAcqGetDynamic(nChannelID)																	  	\
{																											\
	OS_ENTER_CRITICAL();																					\
	nDynCurGetStore[nChannelID] = (DT_STORE)((float)nDynSum[nChannelID] * g_fADParam[nChannelID]);			\
	nDynCurGetStore[nChannelID] -= g_sMesureZero[nChannelID];												\
	OS_EXIT_CRITICAL();																						\
}

#endif		//USE_MACRO_TO_SPEED
#endif		//_DATA_ACQ_GET_DYNAMIC_EN_



//调试时临时使用的代码
//static int32 AdTestValue[4] = {200, 213, 205, 232};
//static int32 AdTestSin[12] = {0, 5, 8, 10, 8, 5, 0, -5, -8, -10, -8, -5};
//    nDynCurGetStore[nChannelID] = AdTestSin[AdTestValue[nChannelID]++];										\
//    if(AdTestValue[nChannelID]	>= 12) AdTestValue[nChannelID]=0;\
//    nDynCurGetStore[nChannelID] = AdTestValue[nChannelID]++;												\
//    if(AdTestValue[nChannelID] > 237)  AdTestValue[nChannelID] = 200;											\
//


//
//数据编码需要的信息
//
#define USE_NEG_INVERT
static uint8 SYS_OPT_SEG nDynCodingLen;
static int8 SYS_OPT_SEG nDynCodingBuff[4];						//动态数据编码缓冲区
static int16 SYS_OPT_SEG nDynCodingPrev[DATA_ACQ_COUNT];		//上一次数据
static int16 SYS_OPT_SEG nDynCodingCur;							//当前数据
static int16 SYS_OPT_SEG nDynCodingDelt;						//数据差值


/*****************************************************************************************************************
* 名	称：LoadCodingData()
* 功	能：装载编码数据
* 入口参数：pData: 指向存储起始地址; nDataLen: 装载数据个数
* 出口参数：编码后返回数据个数:1或3
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
#ifndef USE_MACRO_TO_SPEED
void LoadCodingData(int8 *pData, uint8 nChannelID) reentrant
{
	//获取动态数据
	nDynCodingCur = DataAcqGetDynamic(nChannelID);

	//计算本次数据和上次数据的差值
	nDynCodingDelt = nDynCodingCur - nDynCodingPrev[nChannelID];

	//记录上一次数据数据
	nDynCodingPrev[nChannelID] = nDynCodingCur;

	if(abs(nDynCodingDelt) < (uint16)0x007F)
	{
		pData[0] = nDynCodingDelt;

		pData[0] = nDynCodingDelt;										
		if(pData[0] < 0) 	  											
		{																
			pData[0] -= 1;			   									
			pData[0] = ~pData[0];	   									
			pData[0] |= 0x80;											
		}																
		nDynCodingLen = 1;												
	}
	else
	{
		pData[0] = 0x80;												
		pData[1] = LABYTE(nDynCodingCur, 0);							
		pData[2] = LABYTE(nDynCodingCur, 1);							
		nDynCodingLen = 3;												
	}
}

#else

//使用宏代替上面的函数以提高运行速度
#define LoadCodingData(pData, nChannelID)								\
{																		\
	DataAcqGetDynamic(nChannelID);										\
	nDynCodingCur = nDynCurGetStore[nChannelID];						\
	nDynCodingDelt = nDynCodingCur - nDynCodingPrev[nChannelID];		\
	nDynCodingPrev[nChannelID] = nDynCodingCur;							\
																		\
	if(abs(nDynCodingDelt) < (uint16)0x007F)							\
	{																	\
		pData[0] = nDynCodingDelt;										\
		if(pData[0] < 0) 	  											\
		{																\
			pData[0] -= 1;			   									\
			pData[0] = ~pData[0];	   									\
			pData[0] |= 0x80;											\
		}																\
		nDynCodingLen = 1;												\
	}																	\
	else																\
	{																	\
		pData[0] = 0x80;												\
		pData[1] = LABYTE(nDynCodingCur, 0);							\
		pData[2] = LABYTE(nDynCodingCur, 1);							\
		nDynCodingLen = 3;												\
	}																	\
}																		
#endif		//USE_MACRO_TO_SPEED
#endif		//_DATA_ACQ_GET_DYNAMIC_EN_


/*****************************************************************************************************************
* 名	称：DataAcqPushDynamic()
* 功	能：将当前动态数据放入存储缓冲区中,若存储缓冲区数据已满，则存储
* 入口参数：要存数据个数
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
void DataAcqPushDynamic(uint8 nLen) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	uint8 i;

	//多个数据一起处理
	while(nLen--)
	{
		//通道0======================================================================================================
		//获取编码数据
		LoadCodingData(nDynCodingBuff, 0);

		//存储到动态缓冲区中
		for(i=0; i < nDynCodingLen; i++)
		{
			//进入临界区
			OS_ENTER_CRITICAL();

			//记录数据
			g_nDynBuff[0][nDynIndex[0]] = nDynCodingBuff[i];
			
			//退出临界区
			OS_EXIT_CRITICAL();
			
			//更新索引
			nDynIndex[0]++;							//区长度加一
			nDynIndex[0] &= DYN_BUFF_LEN_MASK;		//防止溢出
			
			nCurSectLen[0]++ ;						//段长度加一
			nDynLocalAddr[0]++;						//累加地址

			//判断是否要存储数据============================
			if(nCurSectLen[0] >= DYN_STORE_THRES)
			{
				//进入临界区
				OS_ENTER_CRITICAL();

				//设置存储段
				g_nStoreSectStart[0] = nCurSectStart[0];
				g_nStoreSectLen[0] = nCurSectLen[0];

				//退出临界区
				OS_EXIT_CRITICAL();

				//更新当前缓冲段
				nCurSectStart[0] += nCurSectLen[0];
				nCurSectStart[0] &= DYN_BUFF_LEN_MASK;	//防止溢出
				
				//当前段长度清零
				nCurSectLen[0] = 0;
				
				//发送存储信号量
				PostDataStore(0);
			}
		}
		//---------------------------------------------------------------------------------------------------------



		//通道1======================================================================================================
		//获取编码数据
		LoadCodingData(nDynCodingBuff, 1);

		//存储到动态缓冲区中
		for(i=0; i < nDynCodingLen; i++)
		{
			//进入临界区
			OS_ENTER_CRITICAL();

			//记录数据
			g_nDynBuff[1][nDynIndex[1]] = nDynCodingBuff[i];
			
			//退出临界区
			OS_EXIT_CRITICAL();

			//更新索引
			nDynIndex[1]++;							//区长度加一
			nDynIndex[1] &= DYN_BUFF_LEN_MASK;		//防止溢出

			nCurSectLen[1]++ ;						//段长度加一
			nDynLocalAddr[1]++;						//累加地址


			//判断是否要存储数据============================
			if(nCurSectLen[1] >= DYN_STORE_THRES)
			{
				//进入临界区
				OS_ENTER_CRITICAL();

				//设置存储段
				g_nStoreSectStart[1] = nCurSectStart[1];
				g_nStoreSectLen[1] = nCurSectLen[1];

				//退出临界区
				OS_EXIT_CRITICAL();

				//更新当前缓冲段
				nCurSectStart[1] += nCurSectLen[1];
				nCurSectStart[1] &= DYN_BUFF_LEN_MASK;	//防止溢出
				nCurSectLen[1] = 0;
				
				//发送存储信号量
				PostDataStore(1);
			}
		}
		//---------------------------------------------------------------------------------------------------------


		//通道2======================================================================================================
		//获取编码数据
		LoadCodingData(nDynCodingBuff, 2);

		//存储到动态缓冲区中
		for(i=0; i < nDynCodingLen; i++)
		{
			//进入临界区
			OS_ENTER_CRITICAL();

			//记录数据
			g_nDynBuff[2][nDynIndex[2]] = nDynCodingBuff[i];
			
			//退出临界区
			OS_EXIT_CRITICAL();

			//更新索引
			nDynIndex[2]++;							//区长度加一
			nDynIndex[2] &= DYN_BUFF_LEN_MASK;		//防止溢出
			
			nCurSectLen[2]++ ;						//段长度加一
			nDynLocalAddr[2]++;						//累加地址


			//判断是否要存储数据============================
			if(nCurSectLen[2] >= DYN_STORE_THRES)
			{
				//进入临界区
				OS_ENTER_CRITICAL();

				//设置存储段
				g_nStoreSectStart[2] = nCurSectStart[2];
				g_nStoreSectLen[2] = nCurSectLen[2];

				//退出临界区
				OS_EXIT_CRITICAL();

				//更新当前缓冲段
				nCurSectStart[2] += nCurSectLen[2];
				nCurSectStart[2] &= DYN_BUFF_LEN_MASK;	//防止溢出
				nCurSectLen[2] = 0;
				
				//发送存储信号量
				PostDataStore(2);
			}
		}
		//---------------------------------------------------------------------------------------------------------



		//通道3======================================================================================================
		//获取编码数据
		LoadCodingData(nDynCodingBuff, 3);

		//存储到动态缓冲区中
		for(i=0; i < nDynCodingLen; i++)
		{
			//进入临界区
			OS_ENTER_CRITICAL();

			//记录数据
			g_nDynBuff[3][nDynIndex[3]] = nDynCodingBuff[i];
			
			//退出临界区
			OS_EXIT_CRITICAL();

			//更新索引
			nDynIndex[3]++;			//区长度加一
			nDynIndex[3] &= DYN_BUFF_LEN_MASK;		//防止溢出
			
			nCurSectLen[3]++ ;		//段长度加一
			nDynLocalAddr[3]++;		//累加地址

			
			//判断是否要存储数据============================
			if(nCurSectLen[3] >= DYN_STORE_THRES)
			{
				//进入临界区
				OS_ENTER_CRITICAL();

				//设置存储段
				g_nStoreSectStart[3] = nCurSectStart[3];
				g_nStoreSectLen[3] = nCurSectLen[3];

				//退出临界区
				OS_EXIT_CRITICAL();

				//更新当前缓冲段
				nCurSectStart[3] += nCurSectLen[3];
				nCurSectStart[3] &= DYN_BUFF_LEN_MASK;	//防止溢出
				nCurSectLen[3] = 0;
				
				//发送存储信号量
				PostDataStore(3);
			}
		}
		//---------------------------------------------------------------------------------------------------------


		OS_ENTER_CRITICAL();			//进入临界区
		nLocalCount++;					//更新本地动态数据个数
		OS_EXIT_CRITICAL();				//退出临界区
	}
}
#endif


/*****************************************************************************************************************
* 名	称：DataAcqDataStore()
* 功	能：存储数据
* 入口参数：无
* 出口参数：无
* 说	明：用于结束动态采集时存储小段数据
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
void DataAcqDataStore() reentrant
{
	uint8 i;
	
	//设置存储段
	for(i=0; i<DATA_ACQ_COUNT; i++)
	{
		//设置要存储的地址
		g_nStoreSectStart[i] = nCurSectStart[i];
		g_nStoreSectLen[i] = nCurSectLen[i];


		PostDataStore(i);
	}
}
#endif

/*****************************************************************************************************************
* 名	称：DataLoad()
* 功	能：装载动态数据
* 入口参数：无
* 出口参数：数据地址
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_GET_DYNAMIC_EN_
void DataLoad(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint16 nlen,uint8 nChannelID) reentrant
{
	uint32 	DLnLenTmp;
	uint32 	DLnMemReadLen;				//存储器中能读的数据长度
	uint32 	DLnDynBuffStart;			//在缓冲区中的开始索引

	//缓冲区已满=========================================
	if(DYN_BUFF_LEN < nDynLocalAddr[nChannelID])
	{
		//若数据地址在存储器中
		if(nStartAddr < nDynLocalAddr[nChannelID] - DYN_BUFF_LEN)
		{
			//从存储器中装载数据
			DLnMemReadLen = nDynLocalAddr[nChannelID] - DYN_BUFF_LEN - nStartAddr;
			DLnLenTmp = (DLnMemReadLen > nlen) ? nlen : DLnMemReadLen;
			MemDataLoad(pSem, pdat, nStartAddr, DLnLenTmp, nChannelID);  

			//判断是否读取完毕，并返回
			if(nlen == DLnLenTmp) return;

			//计算剩余剩余长度
			nlen -= DLnLenTmp;

			//更新存储地址指针
			pdat += DLnLenTmp;

			//更新开始地址
			nStartAddr = nDynLocalAddr[nChannelID] - DYN_BUFF_LEN;
		}


		//从缓冲区读取数据=================================
		//获取在缓冲区中的相对地址索引，始终小于DYN_BUFF_LEN
		DLnDynBuffStart = nStartAddr - (nDynLocalAddr[nChannelID] - DYN_BUFF_LEN);	

		//加上缓冲区开始地址索引
		DLnDynBuffStart += nDynIndex[nChannelID];
		DLnDynBuffStart &= DYN_BUFF_LEN_MASK;		//防止溢出
		while(nlen--)
		{
			*pdat = g_nDynBuff[nChannelID][DLnDynBuffStart];
			pdat++;
			DLnDynBuffStart++;
			DLnDynBuffStart &= DYN_BUFF_LEN_MASK;		//防止溢出
		}

		return;
	}

	//缓冲区未满=====================================
	else if(0 < nDynLocalAddr[nChannelID])
	{
		memcpy(pdat, g_nDynBuff[nChannelID] + nStartAddr, nlen);
		return;
	}

	//若直接读数，从存储器中直接装载=============
	MemDataLoad(pSem, pdat, nStartAddr, nlen, nChannelID);
	
	return;
}
#endif


/*****************************************************************************************************************
* 名	称：DataAcqStart()
* 功	能：打开传感器
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_START_EN_
void DataAcqStart() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//复位动态数据(静态可能也会使用一点)
	DataInitZero(nDynSum);
	DataInitZero(nDynCodingBuff);
	DataInitZero(nDynCodingPrev);
	DataInitZero(bDataNeedStore);

	DataInitZero(nDynIndex);
	DataInitZero(nCurSectStart);
	DataInitZero(nCurSectLen);
	DataInitZero(g_nStoreSectStart);
	DataInitZero(g_nStoreSectLen);
	DataInitZero(nDynCurGetStore);

	DataInitZero(nStoreAddr);	
	DataInitZero(nDynLocalAddr);

	
	//复位静态数据
	DataAcqResetStaticIndex();


	//重载开始***************************************
	//设置任务 
	OS_ENTER_CRITICAL();
	nDataAcqTaskInner = DATA_ACQ_TASK_START;
	OS_EXIT_CRITICAL();
	
	//给开启信号
	OSSemPost(pDataAcqEvent);	
	//重载结束***************************************  
}
#endif

/*****************************************************************************************************************
* 名	称：DataAcqPauseResume()
* 功	能：从暂停中恢复
* 入口参数：无
* 出口参数：无
* 说	明：无
*****************************************************************************************************************/
#ifdef	_DATA_ACQ_PAUSE_RESUME_
void DataAcqPauseResume() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//设置任务
	OS_ENTER_CRITICAL();
	nDataAcqTaskInner = DATA_ACQ_TASK_START;
	OS_EXIT_CRITICAL();
	
	//发送信号
	pDataAcqEvent->OSEventCnt = 0;
	OSSemPost(pDataAcqEvent);
}
#endif

/****************************************************************************
* 名	称：ADProcess()
* 功	能：AD任务
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
extern BOOL 	g_bDetectionSensor;							//是否检测传感器
extern uint8 	g_nDetectionSensorCount[DATA_ACQ_COUNT];	
void ADProcess(void)
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	int32 nAdValue;

	//无超时等待AD转换完成
	ADWaitDataReady();

	//判断是哪路中断，并读回数据
	//通道1
	if(g_ADC_IT_Channel & 0x01) 
	{
		g_ADC_IT_Channel &= ~0x01;
		nAdValue = ADS1246GetValue(0);

		if((g_nState & COLLECTOR_DYNAMIC) != 0)
		{
			//动态采集
			DataAcqSetDynamic(nAdValue, 0);
		}
		else if((g_nState & COLLECTOR_STATIC) != 0)
		{
			//静态采集
			DataAcqSetStatic(nAdValue, 0);
		}
		else if(g_bDetectionSensor == TRUE)
		{
			//检测传感器是否存在
			ADS1246IsSensorConnected(nAdValue, 0);
		} 
	}

	//通道2
	if(g_ADC_IT_Channel & 0x02) 
	{
		g_ADC_IT_Channel &= ~0x02;
		nAdValue = ADS1246GetValue(1);

		if((g_nState & COLLECTOR_DYNAMIC) != 0)
		{
			//动态采集
			DataAcqSetDynamic(nAdValue, 1);
		}
		else if((g_nState & COLLECTOR_STATIC) != 0)
		{
			//静态采集
			DataAcqSetStatic(nAdValue, 1);
		}
		else if(g_bDetectionSensor == TRUE)
		{
			//检测传感器是否存在
			ADS1246IsSensorConnected(nAdValue, 1);
		}
	}	

	//通道3
	if(g_ADC_IT_Channel & 0x04) 
	{
		g_ADC_IT_Channel &= ~0x04;
		nAdValue = ADS1246GetValue(2);

		if((g_nState & COLLECTOR_DYNAMIC) != 0)
		{
			//动态采集
			DataAcqSetDynamic(nAdValue, 2);
		}
		else if((g_nState & COLLECTOR_STATIC) != 0)
		{
			//静态采集
			DataAcqSetStatic(nAdValue, 2);
		}
		else if(g_bDetectionSensor == TRUE)
		{
			//检测传感器是否存在
			ADS1246IsSensorConnected(nAdValue, 2);
		}  
	}

	//通道4
	if(g_ADC_IT_Channel & 0x08) 
	{
		g_ADC_IT_Channel &= ~0x08;
		nAdValue = ADS1246GetValue(3);

		if((g_nState & COLLECTOR_DYNAMIC) != 0)
		{
			//动态采集
			DataAcqSetDynamic(nAdValue, 3);
		}
		else if((g_nState & COLLECTOR_STATIC) != 0)
		{
			//静态采集
			DataAcqSetStatic(nAdValue, 3);
		}
		else if(g_bDetectionSensor == TRUE)
		{
			//检测传感器是否存在
			ADS1246IsSensorConnected(nAdValue, 3);
		}
	}
}

/****************************************************************************
* 名	称：SampleProcess()
* 功	能：AD任务
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void SampleProcess() reentrant
{
	//需要存储数据的个数
	uint32 nPushDynNum;

	//无超时等待信号量
	WaitSampleEvent();
	
	//计算需要存储的数据个数
	//默认存一个，如果上位机时间比本地时间快，则将数据个数与上位机对齐
	nPushDynNum = 1;
	if(nHostCount > nLocalCount) 
	{
		nPushDynNum = nHostCount - nLocalCount;
	}
	
	//存储动态数据
	DataAcqPushDynamic(nPushDynNum);	
	
}


/****************************************************************************
* 名	称：DataAcqProcess()
* 功	能：数据采集事物处理程序
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
BOOL DataAcqProcess() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif	 

	uint8 i;
	uint8 nTask;

	//等待传感器开启
	WaitDataAcqEvent();

	//获取任务(四通道同步采集，将任务综合)
	OS_ENTER_CRITICAL();
	nTask = GetDataAcqTask();
	SetDataAcqTask(DATA_ACQ_TASK_NONE);
	OS_EXIT_CRITICAL();

	//启动传感器====================================================================================
	if(nTask == DATA_ACQ_TASK_START)
	{
		//开启AD
		ADOpen();
	
		//参数设置
		for(i = 0; i < DATA_ACQ_COUNT; i++)
		{
			//设置采样率
			if((COLLECTOR_STATIC & g_nState) == COLLECTOR_STATIC)
			{
				//静态采集模式
				ADSetSampleRate(20, i);
			}
			else											
			{
				//动态采集模式
				ADSetSampleRate(nBrgSetSampleRate, i);
			}
			
			//设置增益
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
		DataAcqParamCacul();			
	
		//开始采集
		ADStartSample();

		//返回
		return TRUE;
	}
	
	//停止数据采集====================================================================================
	else if(nTask == DATA_ACQ_TASK_STOP)
	{
		//关闭AD
		ADClose();
		
		//返回
		return TRUE;
	}

	//检测传感器是否接入==============================================================================
	else if(nTask == DATA_ACQ_TASK_DETECTION)
	{
		//检测采集器外接传感器
		ADSensorDetection(nDataAcqConnected);

		//设置传感器任务及状态
		g_bTaskBusy = FALSE;

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
void PostDataStore(uint8 nEp) reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	OS_ENTER_CRITICAL();
	bDataNeedStore[nEp] = TRUE;
	OS_EXIT_CRITICAL();

	OSSemPost(pDataStoreEvent);
}

/****************************************************************************
* 名	称：MemEraseProcess()
* 功	能：擦除存储器程序
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void DataStoreProcess() reentrant
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	uint8 i;
	uint8 nDataStoreErr;
	uint8 *pStoreDynBuffTmp;

	//等待数据采集的存储事件
	OSSemPend(pDataStoreEvent, 0, &nDataStoreErr);

	//获取要存储数据的通道
	OS_ENTER_CRITICAL();
	memcpy(bDataNeedStoreClone, bDataNeedStore, sizeof(bDataNeedStore));
	memset(bDataNeedStore, 0, sizeof(bDataNeedStore)); 
	OS_EXIT_CRITICAL();

	//存储相应通道的数据
	for(i = 0; i < DATA_ACQ_COUNT; i++)
	{
		if(bDataNeedStoreClone[i] == FALSE) continue;

		if(i == 1)
		{
			nDataStoreErr = 5;
		}
		
		//!!!!确保缓冲区大小为存储段大小的整数倍!!!!!!!!!
		pStoreDynBuffTmp = (uint8*)(g_nDynBuff[i] + g_nStoreSectStart[i]);			
		MemDataStore(pDataStoreDiskSem, pStoreDynBuffTmp, nStoreAddr[i], 0x0800, i);

		//存储地址增加
		OS_ENTER_CRITICAL();
		nStoreAddr[i] += 0x0800;
		OS_EXIT_CRITICAL();
	}	
}



