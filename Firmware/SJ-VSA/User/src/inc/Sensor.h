/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: Sensor.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年09月17日
**最后修改日期: 2008年09月17日
**描        述: 传感器头文件
*****************************************************************************************************************/
#ifndef 	_SENSOR_H_
#define 	_SENSOR_H_

#ifdef		_SENSOR_C_
#define		SENSOR_EXT
#else
#define		SENSOR_EXT		extern
#endif

//****************************************************************************************************************
//函数使能
#define		_SENSOR_INIT_EN_
#define 	_SENSOR_START_EN_
#define		_SENSOR_PAUSE_RESUME_
#define		_SENSOR_STOP_EN_
//#define		_SET_SENSOR_STOP_EN_
#define		_SENSOR_GET_STATIC_EN_
#define		_SENSOR_GET_DYNAMIC_EN_
//#define	_SENSOR_UPDATE_PARAM_
#define		_MAIN_PARAM_STORE_
#define		_ZERO_PARAM_STORE_
#define		_POST_SENSOR_PRO_


//传感器任务定义
#define SENSOR_TASK_NONE			0		
#define SENSOR_TASK_START			1
#define SENSOR_TASK_STOP			2
#define SENSOR_TASK_DETECTION		3
#define SENSOR_TASK_PREPARE			4


//测点测量方式
#define	MEASURE_VOL			0x01		//测量电压，使用AD理论步进
#define	MEASURE_STRAIN		0x02		//测量应变，使用校正步进


//静态采集是否使用预采集*********************************************************************************
#define USE_STATIC_PREPARE	


//*******************************************************************************************************
//传感数据个数，(通道数)
#define SENSOR_MAX_COUNT			4				
//#define BUFF_LEN					4


//传感器数据类型定义
#define	DT_SENSOR			float		//传感器测量计算类型
#define	DT_STORE			int16		//存储及传输类型

//宏参数定义
#define	DYN_BUFF_LEN		256			//动态数据缓冲区大小
#define	DYN_STORE_THRES		16			//数据存储阀值

//传感数据个数，(通道数)
#define	F_AD_STEP			F_ADS1246_STEP


//函数申明
void SensorParamCacul(void) ;
void SensorInit(void) ;
void SensorDataInit(void) ;
void MainParamStore(void) ;
uint32 SensorParamStore(void) ;
uint32 SensorParamLoad(void) ;
void MainZeroParamStore(void) ;
uint32 SensorZeroParamStore(void) ;
uint32 SensorZeroParamLoad(void) ;
void SensorUpdateParam(fp32* pParam) ;
void SensorStart(void) ;
void SensorPauseResume(void) ;
void PostSensorPro(void) ;
void WaitSensorEvent(void) ;
void SetSensorStop(void) ;
BOOL IsSensorStop(void) ;
void SensorStop(void) ;
float SensorGetStatic(uint8 nChannelID) ;
void SensorSetStatic(int32 nValue, uint8 nIndex) ;
DT_STORE SensorGetDynamic(uint8 nIndex) ;
void SensorSetDynamic(int32 nValue, uint8 nIndex) ;
void SensorPushDynamic(uint8 nLen) ;
void DataStoreEnd(void) ;
DT_STORE* GetDynDataAddr(uint8 nBlock, uint8 nChannelID) ;
void DataLoad(uint8* pdat,uint32 nStartAddr,uint16 nlen,uint8 nChannelID) ;
void SensorDataStore(void) ;
void SetSensorTask(uint8 nTask) ;
uint8 GetSensorTask(void) ;
void SensorResetStaticIndex(void) ;



//****************************************************************************************************************
#endif
