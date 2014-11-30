/****************************************Copyright (c)************************************************************
**                              
**                          深圳市生基科技有限公司       
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: UserComm.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年09月17日
**最后修改日期: 2008年09月17日
**描        述: 通信应用层头文件
*****************************************************************************************************************/
#ifndef 	_USERCOMM_H_
#define 	_USERCOMM_H_

#ifdef		_USERCOMM_C_
#define		USERCOMM_EXT
#else
#define		USERCOMM_EXT		extern
#endif
//****************************************************************************************************************

//函数使能
#define		_USERCOMM_INIT_EN_
//#define		_DATA_STORE_PRO_


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

typedef	enum tagMEM_TASK_E
{
	MEM_TASK_PUSH_DYN = 0,
	MEM_TASK_STORE_DATA,
	MEM_TASK_BUTT,
}MEM_TASK_E;

//***************************************************************************


/****************************************************************************
* 名	称：UserCommInit(void)
* 功	能：初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void UserCommInit(void);

/****************************************************************************
* 名	称：HostCommProcess(void)
* 功	能：上层通信事务处理
* 入口参数：nLen，数据包长度
* 出口参数：是否成功
* 说	明：无
****************************************************************************/ 
BOOL HostCommProcess(uint8 nLen);

/****************************************************************************
* 名	称：SensorProcess(void)
* 功	能：传感器事务处理
* 入口参数：无
* 出口参数：是否成功
* 说	明：无
****************************************************************************/ 
BOOL SensorProcess(void);

/****************************************************************************
* 名	称：MemProcess(void)
* 功	能：存储事务处理
* 入口参数：无
* 出口参数：是否成功
* 说	明：无
****************************************************************************/ 
BOOL MemProcess(void);

/****************************************************************************
* 名	称：SetNormalRecvTimeOut(void)
* 功	能：设置正常状态时通信接收超时时间
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void SetNormalRecvTimeOut(void);

/****************************************************************************
* 名	称：SetSleepRecvTimeOut(void)
* 功	能：设置休眠时通信接收超时时间
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void SetSleepRecvTimeOut(void);

/****************************************************************************
* 名	称：SetFreqToDefault(void)
* 功	能：设置频率为默认频率
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void SetFreqToDefault(void);

/****************************************************************************
* 名	称：SetFreqToCur(void)
* 功	能：设置频率为调频频率
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void SetFreqToCur(void);

/****************************************************************************
* 名	称：SetStoreType(void)
* 功	能：设置检测参数
* 入口参数：nType, 方式；fSense, 灵敏度系数； fRange, 量程
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void SetStoreType(uint8 nType, fp32 fSense, fp32 fRange);

/****************************************************************************
* 名	称：CommDataClear(void)
* 功	能：清除通信数据
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void CommDataClear(void);

/****************************************************************************
* 名	称：PostDataStore(void)
* 功	能：发送数据存储信号
* 入口参数：nChannelID，通道号
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void PostDataStore(uint8 nChannelID);

/****************************************************************************
* 名	称：ADProcess(void)
* 功	能：AD事务处理
* 入口参数：无
* 出口参数：是否成功
* 说	明：无
****************************************************************************/ 
BOOL ADProcess(void);

/****************************************************************************
* 名	称：SampleProcess(void)
* 功	能：采样事务处理
* 入口参数：无
* 出口参数：是否成功
* 说	明：无
****************************************************************************/ 
BOOL SampleProcess(void);

/****************************************************************************
* 名	称：SensorDetectionAndShow(void)
* 功	能：传感器检测
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 
void SensorDetectionAndShow(void);

/****************************************************************************
* 名	称：UserCommInit(void)
* 功	能：初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/ 



#endif
