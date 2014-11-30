/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: DataAcq.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年09月17日
**最后修改日期: 2008年09月17日
**描        述: 传感器头文件
*****************************************************************************************************************/
#ifndef 	_DATA_ACQ_H_
#define 	_DATA_ACQ_H_

#ifdef		_DATA_ACQ_C_
#define		DATA_ACQ_EXT
#else
#define		DATA_ACQ_EXT		extern
#endif

//****************************************************************************************************************
//函数使能
#define		_DATA_ACQ_INIT_EN_
#define 	_DATA_ACQ_START_EN_
#define		_DATA_ACQ_PAUSE_RESUME_
#define		_DATA_ACQ_STOP_EN_
#define		_DATA_ACQ_GET_STATIC_EN_
#define		_DATA_ACQ_GET_DYNAMIC_EN_
#define		_MAIN_PARAM_STORE_
#define		_ZERO_PARAM_STORE_
#define		_POST_DATA_ACQ_PRO_

//*******************************************************************************************************
//采集通道数据个数
#define DATA_ACQ_COUNT		4	

//数据采集数据类型定义
#define	DT_DATA_ACQ			float		//数据采集后用于计算的类型
#define	DT_STORE			int16		//数据采集后用于存储和传输的类型


//结构定义============================================================
typedef	struct tagSTORE_DATA_TYPE_S
{
	uint8 nStoreType;
	uint8 nDataType;
}STORE_DATA_TYPE_S;

//存储数据表示值
typedef	enum tagSTORE_TYPE_E
{
	STORE_TYPE_UE_1,			//存微应变值，全桥
	STORE_TYPE_UE_2,			//半桥
	STORE_TYPE_UE_4,			//1/4桥
	STORE_TYPE_UV,				//存电压值
}STORE_TYPE_E;


//动态数据存储结构体，用于保存当前数据
typedef enum tagDATA_TYPE_E
{
	DATA_TYPE_2F_128G = 0,		//存2小数，增益128
	DATA_TYPE_1F_128G,			//存1小数，增益128
	DATA_TYPE_0F_128G,			//存0小数，增益128
	DATA_TYPE_0F_64G,			//存0小数，增益64
	DATA_TYPE_0F_32G,			//存0小数，增益32
	DATA_TYPE_0F_16G,
	DATA_TYPE_0F_8G,
	DATA_TYPE_0F_4G,
	DATA_TYPE_0F_2G,
	DATA_TYPE_0F_1G,
	DATA_TYPE_BUTT,
}DATA_TYPE_E;


//全局变量导出
extern uint8 g_bTaskBusy;								//任务忙标志


//函数****************************************************************************************************
//初始化
void DataStoreInit(void);
void DataAcqInit(void) reentrant;

//数据采集参数
void DataAcqParamInit(void) reentrant;
void DataAcqParamStore(void) reentrant;
void DataAcqParamLoad(void) reentrant;
void OnHostParamSet(uint8 nEp, uint8 nType, fp32 fSense, fp32 fRange) reentrant;
void DataAcqParamCacul(void) reentrant;



void DataAcqStart(void) reentrant;
void DataAcqPauseResume(void) reentrant;
void PostDataAcqPro(void) reentrant;
void WaitDataAcqEvent(void) reentrant;
void DataAcqStop(void) reentrant;
float DataAcqGetStatic(uint8 nChannelID) reentrant;
void DataAcqSetStatic(int32 nValue, uint8 nIndex) reentrant;
DT_STORE DataAcqGetDynamic(uint8 nIndex) reentrant;
void DataAcqSetDynamic(int32 nValue, uint8 nIndex) reentrant;
void DataAcqPushDynamic(uint8 nLen) reentrant;
void DataStoreEnd(void) reentrant;
DT_STORE* GetDynDataAddr(uint8 nBlock, uint8 nChannelID) reentrant;
void DataLoad(OS_EVENT* pSem, uint8* pdat,uint32 nStartAddr,uint16 nlen,uint8 nChannelID) reentrant;
void DataAcqDataStore(void) reentrant;
void SetDataAcqTask(uint8 nTask) reentrant;
uint8 GetDataAcqTask(void) reentrant;
void DataAcqResetStaticIndex(void) reentrant;


//任务处理
void ADProcess(void) reentrant;
void SampleProcess(void) reentrant;



//****************************************************************************************************************
#endif
