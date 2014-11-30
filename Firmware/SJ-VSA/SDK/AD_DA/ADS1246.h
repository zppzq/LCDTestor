/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: ADS1246.h
**创   建   人: 罗仕强
**创 建 日  期: 2009年04月14日
**最后修改日期: 2009年04月15日
**描        述: 
********************************************************************************************************/
#ifndef		_ADS1246_H_		
#define		_ADS1246_H_	

#ifdef		_ADS1246_C_
#define		_ADS1246_EXT
#else
#define		_ADS1246_EXT		extern
#endif


//编译开关=======================================================================================
//#define	_ADS1246_STANDBY_MODE_
//#define	_ADS1246_SET_PARAM_
//#define	_ADS1246_IS_DATA_READY_
#define	_ADS1246_SET_ZERO_
//#define	_ADS1246_AGC_
//#define	_ADS1246_SET_RANGE_
//#define	_ADS1246_GAIN_CAL_
//#define	_ADS1246_OFFSET_CAL_
#define	_ADS1246_SET_SAMPLE_RATE_
#define	_ADS1246_SENSOR_DETECTION_
#define	_ADS1246_READ_DATA_CON_


#define AD_RATE_MIN			1
#define	AD_RATE_MAX			2000 

#define ADIntEable()		ADC_IT_Eable()			//开AD中断
#define ADIntDisable()		ADC_IT_Disable()		//禁止AD中断


//ADS1246寄存器定义==========================================================================
#define	ADS1246_BCS			0x00
#define	ADS1246_VBIAS		0x01
#define	ADS1246_MUX1		0x02		//偏差校正，增益校正
#define	ADS1246_SYS0		0x03		//增益，采样率设定
#define	ADS1246_OFC0		0x04		//偏差校正系数[7:0]
#define	ADS1246_OFC1		0x05		//[15:8]
#define	ADS1246_OFC2		0x06		//[23:16]
#define	ADS1246_FSC0		0x07		//满刻度校正系数[7:0]
#define	ADS1246_FSC1		0x08		//[15:8]
#define	ADS1246_FSC2		0x09		//[23:16]
#define	ADS1246_ID			0x0A		//ID ，DOUT控制

//ADS1246 SPI命令字定义==========================================================================
#define	ADS1246_WAKEUP		0x00		//Exit sleep mode
#define	ADS1246_SLEEP		0x02		//Enter sleep mode
#define	ADS1246_SYNC		0x04		//Synchronize the AD conversion
#define	ADS1246_RESET		0x06		//Reset to power-up values
#define	ADS1246_PO_INIT		0x0E		//Power-on initialization
#define	ADS1246_NOP			0xFF		//No operation
#define	ADS1246_RDATA		0x12		//Read data once
#define	ADS1246_RDATAC		0x14		//Read data continuously
#define	ADS1246_SDATAC		0x16		//Stop read data continuously
#define	ADS1246_RREG		0x20		//Read from register rrrr
#define	ADS1246_WREG		0x40		//Write to register rrrr
#define	ADS1246_SYSOCAL		0x60		//System offset calibration
#define	ADS1246_SYSGCAL		0x61		//System gain calibration
#define	ADS1246_SELFOCAL	0x62		//Self offset calibration

//ADS1246相关参数宏定义==========================================================================
#define ADS1246_REF 	((float)2.5)	//参考电压

extern uint8 g_ADC_IT_Channel;
//计算AD结果每个步长代表的毫伏数
//不算PGA
//计算公式: 	
//			  0.5 * ADS1246_REF * 10^6
//fVolBit = -----------------------------
//	                  (2^23 - 1) 
//
#define	F_ADS1246_STEP		0.298023f  

//宏定义=========================================================================================
//#define INVALIDFP32 			0.0f


//ADS1246相关参数结构体定义=====================================================================
typedef struct tagADS1246_DCB_S
{
	uint8	nState;
	uint8	nADGain;				//AD增益
	uint8	nADSpeed;				//AD速率
}ADS1246_DCB_S;


//ADS1246相关参数枚举定义========================================================================
//ADS1246通道选择
typedef enum tagAD_STATE_E
{
	AD_STATE_IDLE = 0,			//空闲中
	AD_STATE_ACTIVE,			//操作中
	AD_STATE_BUTT,				//初始值
}AD_STATE_E;

//ADS1246通道选择
typedef enum tagAD_CHANNEL_SELECT_E
{
	AD_CHANNEL_SELECT_1 = 0,	//选择通道1
	AD_CHANNEL_SELECT_2,		//选择通道2
	AD_CHANNEL_SELECT_3,		//选择通道3
	AD_CHANNEL_SELECT_4,		//选择通道4
	AD_CHANNEL_SELECT_BUTT,		//初始值
}AD_CHANNEL_SELECT_E;

//ADS1246增益选择
typedef enum tagAD_GAIN_SELECT_E
{
	AD_GAIN_SELECT_1 = 0,		//增益选择1
	AD_GAIN_SELECT_2 = 1,		//增益选择2
	AD_GAIN_SELECT_4 ,			//增益选择4
	AD_GAIN_SELECT_8  ,			//增益选择8
	AD_GAIN_SELECT_16 ,			//增益选择16
	AD_GAIN_SELECT_32 ,			//增益选择32
	AD_GAIN_SELECT_64 ,			//增益选择64
	AD_GAIN_SELECT_128  ,		//增益选择128
	AD_GAIN_SELECT_BUTT,		//初始值，增益为设置为AGC，用于静态
}AD_GAIN_SELECT_E;

//ADS1246采样速率选择
typedef enum tagAD_SPEED_SELECT_E
{
	AD_SPEED_SELECT_5SPS = 0,	
	AD_SPEED_SELECT_10SPS = 1,
	AD_SPEED_SELECT_20SPS,
	AD_SPEED_SELECT_40SPS,
	AD_SPEED_SELECT_80SPS,
	AD_SPEED_SELECT_160SPS,
	AD_SPEED_SELECT_320SPS,
	AD_SPEED_SELECT_640SPS,
	AD_SPEED_SELECT_1000SPS,
	AD_SPEED_SELECT_2000SPS,
	AD_SPEED_SELECT_BUTT,
}AD_SPEED_SELECT_E;

//ADS1246采样数据状态
typedef enum tagAD_DATA_STATUS_E
{
	AD_DATA_STATUS_START_CONVERSION = 0,		//开始转换
	AD_DATA_STATUS_UNSETTLED_VIN,				//VIN未稳定
	AD_DATA_STATUS_DIGITAL_FILTER_USETTLED1,	//数字滤波未稳定
	AD_DATA_STATUS_DIGITAL_FILTER_USETTLED2,	//数字滤波未稳定
	AD_DATA_STATUS_DIGITAL_FILTER_USETTLED3, 	//数字滤波未稳定
	AD_DATA_STATUS_SETTLED,						//数据稳定
	AD_DATA_STATUS_BUTT,						//初始值
}AD_DATA_STATUS_E;

//ADS1246偏差校准状态
typedef enum tagAD_OFFSET_CALIBRATION_E
{
	AD_OFFSET_CALIBRATION_HANDLING = 0,	//校验过程中
	AD_OFFSET_CALIBRATION_OK,			//校验完成
	AD_OFFSET_CALIBRATION_BUTT,			//初始值
}AD_OFFSET_CALIBRATION_E;

//ADS1246低功耗模式状态
typedef enum tagAD_STANDBY_MODE_E
{
	AD_STANDBY_MODE_TRUE = 0,		//低功耗模式中
	AD_STANDBY_MODE_FALSE,			//正常工作模式
	AD_STANDBY_MODE_BUTT,			//初始值
}AD_STANDBY_MODE_E;

//ADS1246偏差校准状态
typedef enum tagAD_SENSOR_DETECTION_E
{
	AD_SENSOR_DETECTION_OFF = 0,	//无传感器
	AD_SENSOR_DETECTION_ON,			//有传感器
	AD_SENSOR_DETECTION_SHORT,		//外部短路
	AD_SENSOR_DETECTION_BUTT = 0x80,
}AD_SENSOR_DETECTION_E;

static void ADS1246PowerOnInit(void);
static void ADS1246RWReg(uint8 nReg, uint8 *pData, uint8 nLen, BOOL bRW, uint8 nChannelID);
static void ADS1246ChannelOpen(uint8 nChannelID);
void ADS1246ChannelClose(void);
void ADS1246SetGain(uint8 nGainSelect,uint8 nChannelID);
uint8 ADS1246GetGain(uint8 nChannelID);
void ADS1246SetSampleRate(uint16 nRate,uint8 nChannelID);
void ADS1246Close(void);
void ADS1246StartSample(void);
void ADS1246StopSample(void);



//函数定义================================================
void ADS1246RWReg(uint8 nReg,uint8 *pData,uint8 nLen,BOOL bRW, uint8 nChannelID) ;

//AD基本控制
int32 ADS1246GetValue(uint8 nChannelID) ;	
void ADS1246PortInit(void) ;
void ADS1246VariInit(void) ;
BOOL IsADS1246Open(void) ;
void ADS1246Open(void) ;
void ADS1246Close(void) ;
void ADS1246StartSample(void) ;
void ADS1246StopSample(void) ;
BOOL ADS1246GetVol(float *fVol) ;
void ADS1246SetSampleRate(uint16 nRate,uint8 nChannelID) ;
void ADS1246SetRange(uint16 nRange,uint8 nChannelID) ;
void ADS1246ChannelClose(void) ;
void ADS1246SetGain(uint8 nGainSelect,uint8 nChannelID) ;
void ADS1246CalTmpValue(void) ;
void PostSampleSem(void) ;
void ADS1246WaitDataReady(void) ;
void WaitSampleEvent(void) ;
void ADS1246IsSensorConnected(int32 nValue, uint8 nChannelID) ;
void ADS1246SensorDetection(uint8 *pSensorConnected) ;
void ADS1246StopStatic(void) ;
void ADC_IT_Disable(void);
void ADC_IT_Eable(void);



//端口引脚控制
void ADS1246PortShut(void) ;
void ADS1246PortOpen(void) ;
   
#endif
