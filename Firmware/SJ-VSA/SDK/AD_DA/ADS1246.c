/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: ADS1246.c
**创   建   人: 罗仕强
**创 建 日  期: 2009年4月14日
**最后修改日期: 2009年4月24日
**描        述: 
********************************************************************************************************/
#define _ADS1246_C_

#include "includes.h"
#include "CpuPortAccess.h"
#include "DataAcq.h"
#include "ADS1246.h"
#include "spi.h"


//信号定义
#define ADC1_EXTI_LINE 		EXTI_Line8
#define ADC2_EXTI_LINE 		EXTI_Line6
#define ADC3_EXTI_LINE 		EXTI_Line13
#define ADC4_EXTI_LINE 		EXTI_Line11

			  
#define BREAD 		1								//读		
#define BWRITE		0								//写  

//信号定义**********************************************************************************************
//ADS1246电源开关
#define ADPWEN_PORT			GPIOE
#define ADPWEN				2

//ADS1246电源开关
#define ADSENSOREN_PORT		GPIOG
#define ADSENSOREN			9

//ADS1246复位信号
#define ADRESET_PORT		GPIOE
#define ADRESET				1

//ADS1246AD片选信号
#define ADCS4_PORT			GPIOG
#define ADCS4				12

#define ADCS3_PORT			GPIOG
#define ADCS3				14

#define ADCS2_PORT			GPIOB
#define ADCS2				7

#define ADCS1_PORT			GPIOB
#define ADCS1				9

//ADS1246AD完成信号
/**/
#define ADRDY4_PORT			GPIOG
#define ADRDY4				11

#define ADRDY3_PORT			GPIOG
#define ADRDY3				13

#define ADRDY2_PORT			GPIOB
#define ADRDY2				6

#define ADRDY1_PORT			GPIOB
#define ADRDY1				8


//ADS1246AD开始信号
#define ADSTART_PORT		GPIOE
#define ADSTART				0

//数据定义***********************************************************************
//AD采样信号
OS_EVENT 	*pADS1246Event;					//事件控制块
static uint8 		nADS1246Err;			//错误标志

//数据采样信号
static OS_EVENT 	*pSampleEvent;			//事件控制块
static uint8 		nSampleEventErr;		//错误标志

uint8 g_ADC_IT_Channel = 0x00;

//for ADS1246--------------------------------
//数据参数
uint8 g_nNeedSysInfo[DATA_ACQ_COUNT];		//记录系统信息
uint8 g_nADCurGain[DATA_ACQ_COUNT];			//记录当前增益

//运行状态标志记录
static BOOL bADS1246Open = FALSE;						//是否开启芯片
static BOOL bADSample = FALSE;							//是否正在采集

BOOL g_bDetectionSensor;								//是否监测传感器
uint8 g_nDetectionSensorCount[DATA_ACQ_COUNT];		//传感器计数

//宏定义**********************************************************************************************
//开关ADS1246电源，POWER_DOWN会使AD复位
#define SENSOR_POWER_ON() 		SetHi(ADPWEN);SetHi(ADSENSOREN)
#define SENSOR_POWER_OFF() 		SetLo(ADPWEN);SetLo(ADSENSOREN)

#define	ADSampleStart()			SetHi(ADSTART)
#define	ADSampleStop()			SetLo(ADSTART)

void ADC_IT_Configuration(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;

	//端口配置
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11 | GPIO_Pin_13;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOG, &GPIO_InitStructure);
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_6;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	

	//映射
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource11);	  	//ADRDY4
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOG, GPIO_PinSource13);	  	//ADRDY3
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource6);	  		//ADRDY2
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOB, GPIO_PinSource8);	  		//ADRDY1

	//中断配置为下降沿触发	
    EXTI->FTSR |= (EXTI_Line6 | EXTI_Line8 | EXTI_Line11 | EXTI_Line13);
}

/****************************************************************************
* 名	称：ADS1246PortInit()
* 功	能：ADS1246初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ADS1246PortInit()
{
	MakePushPull(ADPWEN);
	MakePushPull(ADSENSOREN);
	MakePushPull(ADRESET);
	MakePushPull(ADSTART);
	MakePushPull(ADCS1);
	MakeOpenDrain(ADRDY1);
	SetHi(ADRESET);
	SetHi(ADRDY1);

	//中断配置
	ADC_IT_Configuration();

#ifdef	ADCS2
	MakePushPull(ADCS2);
	MakePushPull(ADCS3);
	MakePushPull(ADCS4);
	MakeOpenDrain(ADRDY2);
	MakeOpenDrain(ADRDY3);
	MakeOpenDrain(ADRDY4);
	SetHi(ADRDY2);
	SetHi(ADRDY3);
	SetHi(ADRDY4);
#endif	
	
	ADIntDisable();//禁止AD中断
	SENSOR_POWER_OFF();		//关闭AD电源
	ADSampleStop();			//关闭采集
	ADS1246ChannelClose();	//AD片选关闭

	ADS1246PortOpen();
	SpiPortInit();	//
	SpiPortOpen();
}

//端口引脚控制
///////////这个函数要处理//////// 
void ADS1246PortShut()
{
	//关传感器电源
	MakePushPull(ADSENSOREN);
	SetLo(ADSENSOREN);

	//关AD电源
	MakePushPull(ADPWEN);
	SetLo(ADPWEN);

	//设置
	MakeOpenDrain(ADRESET);
	MakeOpenDrain(ADSTART);
	MakeOpenDrain(ADCS1);
	MakeOpenDrain(ADRDY1);


	SetLo(ADRESET);
	SetLo(ADSTART);
	SetLo(ADCS1);
	SetLo(ADRDY1);
	
		
#ifdef	ADCS2
	MakeOpenDrain(ADCS2);
	MakeOpenDrain(ADCS3);
	MakeOpenDrain(ADCS4);
	MakeOpenDrain(ADRDY2);
	MakeOpenDrain(ADRDY3);
	MakeOpenDrain(ADRDY4);
	
	
	SetLo(ADCS2);
	SetLo(ADCS3);
	SetLo(ADCS4);
	SetLo(ADRDY2);
	SetLo(ADRDY3);
	SetLo(ADRDY4);
#endif	  	   
}

void ADS1246PortOpen()
{
	//必须开电源，不然采集器无法唤醒??
	//SetHi(ADPWEN);
	
	SetHi(ADRESET);
	SetLo(ADSTART);
	SetHi(ADCS1);
	SetHi(ADRDY1);

	MakePushPull(ADRESET);
	MakePushPull(ADSTART);
	MakePushPull(ADCS1);
	MakeOpenDrain(ADRDY1);

#ifdef	ADCS2
	SetHi(ADCS2);
	SetHi(ADCS3);
	SetHi(ADCS4);
	SetHi(ADRDY2);
	SetHi(ADRDY3);
	SetHi(ADRDY4);


	MakePushPull(ADCS2);
	MakePushPull(ADCS3);
	MakePushPull(ADCS4);
	MakeOpenDrain(ADRDY2);
	MakeOpenDrain(ADRDY3);
	MakeOpenDrain(ADRDY4);
#endif
}

/****************************************************************************
* 名	称：ADS1246VariInit()
* 功	能：ADS1246全局变量初始化函数
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ADS1246VariInit()
{
	pADS1246Event = NULL;
	pSampleEvent = NULL;
	
	//创建AD转换完成信号量
	if(pADS1246Event == NULL) 
	{
		pADS1246Event = OSSemCreate(0);
	}

	//创建采样信号量
	if(pSampleEvent == NULL) 
	{
		pSampleEvent = OSSemCreate(0);
	}	

	//参数初始化
	DataInitZero(g_nNeedSysInfo);
	DataInit(g_nADCurGain,AD_GAIN_SELECT_BUTT);
	g_bDetectionSensor = FALSE;
}


/****************************************************************************
* 名	称：ADS1246SensorDetection()
* 功	能：ADS1246 传感器检查
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
#ifdef	_ADS1246_SENSOR_DETECTION_

//宏定义=====================================================================
#define	VALUE_SENSOR_ON		0x00000200		//传感器在位时的转换数值上限
#define SENSOR_DETECTION_TIME 		50				//检测时间2S
#define MIN_COUNT			20				//小于下限的最少次数

void ADS1246IsSensorConnected(int32 nValue, uint8 nChannelID)
{
	//取绝对值
	nValue = labs(nValue);
	
	//小于监测下限，表示有外接传感器
	if(nValue < VALUE_SENSOR_ON)
	{
		g_nDetectionSensorCount[nChannelID]++;								
	}
}


/****************************
*BCS		电流
*0x01		0
*0x41		0.5uA
*0x81		2uA
*0xC1		10uA		
*****************************/
void ADS1246SensorDetection(uint8 *pSensorConnected)
{
	uint8 i;
	uint8 tmp = 0x41; 		//设定电流

	DataInitZero(g_nDetectionSensorCount);
	
	//开AD电源，不开传感器电源
	SetHi(ADPWEN);
	SetLo(ADSENSOREN);

	bADS1246Open = TRUE;
	
	//ADS1246芯片初始化
	OSTimeDly(20);		

	ADS1246PowerOnInit();	//芯片上电复位

	//设置寄存器，初始电流
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		ADS1246RWReg(ADS1246_BCS, &tmp, 1, BWRITE, i);
		*(pSensorConnected+i) = AD_SENSOR_DETECTION_BUTT;
		ADS1246SetGain(AD_GAIN_SELECT_1, i);
		ADS1246SetSampleRate(80, i);
	}

	//开始采样
	ADS1246StartSample();

	//使能采样统计
	g_bDetectionSensor = TRUE;

	//延时设定时间，AD采样并统计有无传感器中
	OSTimeDly(SENSOR_DETECTION_TIME);

	//禁止采样统计
	g_bDetectionSensor = FALSE;

	//关闭所有传感器电源
	ADS1246Close();

	//判断是否有传感器
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		if(g_nDetectionSensorCount[i] >= MIN_COUNT)  
			*(pSensorConnected+i) = AD_SENSOR_DETECTION_ON;
		else *(pSensorConnected+i) = AD_SENSOR_DETECTION_OFF;
	}
	
}
#endif

/****************************************************************************
* 名	称：ADS1246_rw()
* 功	能：ADS1246 spi 读写操作
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
static void ADS1246PowerOnInit(void)
{
	//使用RESET信号线
	SetLo(ADRESET);		
	OSTimeDly(1);		//ADRESET下跳延时10ms
	SetHi(ADRESET);									
	OSTimeDly(1);	
}

/****************************************************************************
* 名	称：ADS1246ReadReg()
* 功	能：ADS1246读写寄存器
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
static void ADS1246RWReg(uint8 nReg, uint8 *pData, uint8 nLen, BOOL bRW, uint8 nChannelID)
{
	uint8 i;

#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	if(BWRITE == bRW) nReg |= ADS1246_WREG;
	else if(BREAD == bRW) nReg |= ADS1246_RREG;

	OS_ENTER_CRITICAL();
	
	ADS1246ChannelOpen(nChannelID);
	spi_rw(nReg);  
	spi_rw(nLen-1);			//待写长度nLen-1
	for(i=0;i<nLen;i++)										
	{
		if(BWRITE == bRW)
		{
			spi_rw(*(pData+i));
		}
		else if(BREAD == bRW)
		{
			*(pData+i) = spi_rw(ADS1246_NOP);
		}
	}
	ADS1246ChannelClose();
	
	
	OS_EXIT_CRITICAL();
}

/****************************************************************************
* 名	称：ADS1234ChannelOpen()
* 功	能：ADS1234通道开
* 入口参数：nChannelID ：选择的通道，范围：AD_CHANNEL_SELECT_E
* 出口参数：无
* 说	明：无
****************************************************************************/
static void ADS1246ChannelOpen(uint8 nChannelID)
{
	switch(nChannelID)
	{
		case AD_CHANNEL_SELECT_1: 
			SetLo(ADCS1);
			break;
#ifdef	ADCS2			
		case AD_CHANNEL_SELECT_2:
			SetLo(ADCS2);
			break;
		case AD_CHANNEL_SELECT_3:
			SetLo(ADCS3);
			break;
		case AD_CHANNEL_SELECT_4:
			SetLo(ADCS4);
			break;
#endif			
		default :
			SetLo(ADCS1);
			break;
	}  
}

/****************************************************************************
* 名	称：ADS1234ChannelClose()
* 功	能：ADS1234通道关
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ADS1246ChannelClose()
{
	SetHi(ADCS1);
#ifdef	ADCS2
	SetHi(ADCS2);
	SetHi(ADCS3);
	SetHi(ADCS4);
#endif	
}


/****************************************************************************
* 名	称：ADS1246GainSelect()
* 功	能：ADS1246增益选择函数
* 入口参数：nGainSelect ：选择的增益，范围：AD_GAIN_SELECT_E
* 出口参数：无
* 说	明：无
****************************************************************************/
void ADS1246SetGain(uint8 nGainSelect,uint8 nChannelID)
{
	g_nNeedSysInfo[nChannelID] &= 0x0F;
	g_nNeedSysInfo[nChannelID] |= (nGainSelect << 4);	
	g_nADCurGain[nChannelID] = nGainSelect;
}

/****************************************************************************
* 名	称：ADS1246GetGain()
* 功	能：获取ADC增益
* 入口参数：nGainSelect ：选择的增益，范围：AD_GAIN_SELECT_E
* 出口参数：无
* 说	明：无
****************************************************************************/
uint8 ADS1246GetGain(uint8 nChannelID)
{
	return g_nADCurGain[nChannelID];
}

/****************************************************************************
* 名	称：ADS1246SpeedSelect()
* 功	能：ADS1246采样速率选择函数
* 入口参数：nSpeedSelect ：选择采样速率，范围：AD_SPEED_SELECT_E
* 出口参数：无
* 说	明：无
****************************************************************************/
static void ADS1246SetSpeed(const uint8 nSpeedSelect,uint8 nChannelID)
{
	g_nNeedSysInfo[nChannelID] &= 0xF0;
	g_nNeedSysInfo[nChannelID] |= nSpeedSelect ;	
}

/****************************************************************************
* 名	称：ADS1246SetParam()
* 功	能：ADS1246设置参数
* 入口参数：待设置的参数地址
* 出口参数：无
* 说	明：无
****************************************************************************/
static uint8 ADS1246GetSpeed(uint16 nSpeed)
{
#if	(AD_RATE_MAX > 300)
	//较高的采样率
	if(1000 < nSpeed)		return AD_SPEED_SELECT_2000SPS; 
	else if(640 < nSpeed)	return AD_SPEED_SELECT_1000SPS; 
	else if(320 < nSpeed)	return AD_SPEED_SELECT_640SPS; 
	else if(160 < nSpeed)	return AD_SPEED_SELECT_320SPS; 
#else
	if(160 < nSpeed)	return AD_SPEED_SELECT_320SPS; 
#endif


	else if(80 < nSpeed)	return AD_SPEED_SELECT_160SPS; 
	else if(40 < nSpeed)	return AD_SPEED_SELECT_80SPS; 
	else if(20 < nSpeed) 	return AD_SPEED_SELECT_40SPS; 
	else if(10 < nSpeed) 	return AD_SPEED_SELECT_20SPS; 
	else if(5 < nSpeed) 	return AD_SPEED_SELECT_10SPS;
	else return AD_SPEED_SELECT_5SPS;
}

/****************************************************************************
* 名	称：ADS1246SetSampleRate()
* 功	能：ADS1246设置采样率参数
* 入口参数：采样率，单位Hz
* 出口参数：无
* 说	明：保证已经开启AD
****************************************************************************/
#ifdef	_ADS1246_SET_SAMPLE_RATE_
void ADS1246SetSampleRate(uint16 nRate,uint8 nChannelID)
{
	ADS1246SetSpeed(ADS1246GetSpeed(nRate), nChannelID);
}
#endif

/****************************************************************************
* 名	称：ADS1246SetRange()
* 功	能：ADS1246设置量程
* 入口参数：待测电压量程，单位:mv
* 出口参数：无
* 说	明：设置范围用来设定增益
****************************************************************************/
#ifdef _ADS1246_SET_RANGE_
void ADS1246SetRange(uint16 nRange,uint8 nChannelID)
{
	uint8 nGainTemp;
	nGainTemp = ADS1246GetGain(nRange);

	//设置增益								  
	ADS1246SetGain(nGainTemp,nChannelID);	
}
#endif

/****************************************************************************
* 名	称：ADS1246Open()
* 功	能：打开ADS1246
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ADS1246Open()
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//若AD已经处于打开状态，则返回(必须的)
	if(bADS1246Open == TRUE) return;	
	
	//开AD总电源
	SENSOR_POWER_ON();
	
	//ADS1246芯片初始化，上电后，延时100ms
	OSTimeDly(20);

	//芯片上电复位
	ADS1246PowerOnInit();
	
	//设置AD打开标志
	OS_ENTER_CRITICAL();
	bADS1246Open = TRUE;
	OS_EXIT_CRITICAL();
}

/****************************************************************************
* 名	称：ADS1246StopStatic()
* 功	能：关闭ADS1246
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ADS1246StopStatic()
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif

	//关闭所有传感器电源
	ADS1246StopSample();	

	//关闭传感器电源
	SetHi(ADPWEN);
	SetLo(ADSENSOREN);

	//设置AD开关状态
	OS_ENTER_CRITICAL();
	bADS1246Open = FALSE;
	OS_EXIT_CRITICAL();
}

/****************************************************************************
* 名	称：ADS1246Close()
* 功	能：关闭ADS1246
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void ADS1246Close()
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif
	//关闭所有传感器电源
	ADS1246StopSample();	

	//关闭传感器电源
	SENSOR_POWER_OFF();

	//设置AD开关状态
	OS_ENTER_CRITICAL();
	bADS1246Open = FALSE;
	OS_EXIT_CRITICAL();
}

/****************************************************************************
* 名	称：ADS1246GainCal()
* 功	能：增益校准
* 入口参数：无
* 出口参数：无
* 说	明：Gain = 1, Gain calibration: VREFP – VREFN (full-scale)
****************************************************************************/
#ifdef	_ADS1246_GAIN_CAL_
static void ADS1246GainCal()
{
	uint8 i;
	uint8 tmp;		  

	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		tmp = 0x02;
		ADS1246RWReg(ADS1246_MUX1,&tmp,1, BWRITE,i);
		ADS1246ChannelOpen(i);
		spi_rw(ADS1246_SYSGCAL);
		ADS1246ChannelClose();											
	}
		  
																						
	//等待增益校正完成
	ADS1246WaitDataReady();
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		//ADS1246RWReg(ADS1246_FSC0,((uint8*)(&g_nADGainCal[i])+1),3, BREAD,i);
	}
	//校正处理
	
}
#endif


/****************************************************************************
* 名	称：ADS1246OffsetCal()
* 功	能：偏差自校正
* 入口参数：无
* 出口参数：无
* 说	明：Offset calibration: inputs shorted to midsupply (AVDD + AVSS)/2
****************************************************************************/
#ifdef	_ADS1246_OFFSET_CAL_
static void ADS1246OffsetCal()
{
	uint8 i;
	uint32 nOffsetTmp = 0;
	uint8 tmp;

	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		tmp = 0x01;		//Offset cal
		ADS1246RWReg(ADS1246_MUX1,&tmp, 1,BWRITE,i);		
		ADS1246ChannelOpen(i);
		spi_rw(ADS1246_SYSOCAL);		//自偏差校正
		ADS1246ChannelClose();
	}
	//等待偏差校正完成
	ADS1246WaitDataReady();
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		//ADS1246RWReg(ADS1246_OFC0, ((uint8*)(&g_nADOffsetCal[i])+1), 3, BREAD, i);
	}
	//校正处理

}
#endif


/****************************************************************************
* 名	称：ADS1246StartSample()
* 功	能：开始采集
* 入口参数：开始静态或动态
* 出口参数：无
* 说	明：开始采样及切换通道时调用
****************************************************************************/
void ADS1246StartSample()
{
	uint8 i;
	uint8 tmp;
	
	//判断状态
	if(TRUE == bADSample) return;

	//确保系统信息设置成功
	for(i=0;i<DATA_ACQ_COUNT;i++)
	{
		while(1)
		{
			//设置
			OSTimeDly(1);
			ADS1246RWReg(ADS1246_SYS0, &g_nNeedSysInfo[i], 1, BWRITE,i);
			
			//读取数据
			tmp = 0;
			ADS1246RWReg(ADS1246_SYS0, &tmp, 1, BREAD,i);
			
			//校验
			if(g_nNeedSysInfo[i] == tmp) break;
			
			//终止
			if(FALSE == bADS1246Open) break;
		}
	}

	//开始采样
	ADSampleStart();
	
	//使能中断
	ADIntEable();


	//校正处理
#ifdef _ADS1246_GAIN_CAL_
	ADS1246GainCal(); errset		//增益校正					
#endif

#ifdef _ADS1246_OFFSET_CAL_
	ADS1246OffsetCal();	errset
#endif
	
	//设置开始采样标志
	g_ADC_IT_Channel = 0;
	bADSample = TRUE;
}

/****************************************************************************
* 名	称：ADS1246StopSample()
* 功	能：开始采集
* 入口参数：开始采集的通道号
* 出口参数：无
* 说	明：开始采样及切换通道时调用
****************************************************************************/
void ADS1246StopSample()
{
	if(bADSample == FALSE) return;

	//停止采集
	ADSampleStop();
	
	//禁止中断
	ADIntDisable();
	
	//关闭采样标志
	bADSample = FALSE;
}

/****************************************************************************
* 名	称：ADS1246AGC()
* 功	能：获取采样电压
* 入口参数：无
* 出口参数：采样电压，单位mv
* 说	明：
****************************************************************************/
#ifdef	_ADS1246_AGC_
static void ADS1246AGC(uint8 nChannelID)
{
	int8 nTmp1;
	int32 tmp;
	uint8 nGainTmp;		 

	ADS1246SetGain(AD_GAIN_SELECT_32,nChannelID);
	ADS1246WaitDataReady();				//等待AD转换完成
	tmp = ADS1246GetValue(nChannelID);
	nTmp1 = LABYTE(tmp,1);
	if(0x01 > abs(nTmp1))
	{								   
		nGainTmp = AD_GAIN_SELECT_128;
	}
	else if(0x02 > abs(nTmp1))
	{
		nGainTmp = AD_GAIN_SELECT_64; 
	}
	else
	{
		nGainTmp = AD_GAIN_SELECT_32; 
	}

	ADS1246SetGain(nGainTmp,nChannelID);
	ADS1246SetReadC();
}
#endif

/****************************************************************************
* 名	称：ADS1246IsDataReady()
* 功	能：获取当前数据状态
* 入口参数：无
* 出口参数：TRUE,已准备好；FALSE,未准备好
* 说	明：
****************************************************************************/
void  ADS1246WaitDataReady()
{
	//等待采样完毕
	OSSemPend(pADS1246Event, 0, &nADS1246Err);
}

/****************************************************************************
* 名	称：WaitSampleEvent()
* 功	能：等待采样信号
* 入口参数：无
* 出口参数：无
* 说	明：
****************************************************************************/
void WaitSampleEvent()
{
	//等待采样完毕
	OSSemPend(pSampleEvent, 0, &nSampleEventErr);
}

/****************************************************************************
* 名	称：PostSampleSem()
* 功	能：发送采样信号
* 入口参数：无
* 出口参数：
* 说	明：
****************************************************************************/
void PostSampleSem()
{
	OSSemPost(pSampleEvent);	 
}



//数据访问-----------------------------------------------------------------
//注：大端编译模式（C51默认为大端编译）
//
//得到一个字的高字节
#define HIBYTE(x)	(*((uint8*)(&x)))

//得到一个字的低字节
#define LOBYTE(x)	(*((uint8*)(&x)+1))

//得到某个变量的第i个字节，低地址开始
#define LABYTE(val,i)		(*((uint8*)(&val)+i))

/****************************************************************************
* 名	称：ADS1246GetValue()
* 功	能：读取采样结果
* 入口参数：无
* 出口参数：采样结果
* 说	明：读ADC 数据寄存器的内容
****************************************************************************/
//#define _AD_CONTINOUS_READ_
static int32 nADBuff = 0;

int32 ADS1246GetValue(uint8 nChannelID)
{
#if OS_CRITICAL_METHOD == 3  
	OS_CPU_SR  cpu_sr = 0;
#endif
	
	//关中断，防止异常
	OS_ENTER_CRITICAL();		
	
	//片选芯片
	ADS1246ChannelOpen(nChannelID);

#ifdef _AD_CONTINOUS_READ_
	if(bADReadContinous[nChannelID] == FALSE)
	{
		//设置连续读
		spi_rw(ADS1246_RDATAC);
		bADReadContinous[nChannelID] = TRUE;

		//取消芯片选择
		ADS1246ChannelClose();

		//开中断
		OS_EXIT_CRITICAL();

		return 0x007FFFFF;
	}
#else
		//设置连续读
		spi_rw(ADS1246_RDATA);		  
#endif	
	

	//读数据
	LABYTE(nADBuff,2) = spi_rw(ADS1246_NOP);
	LABYTE(nADBuff,1) = spi_rw(ADS1246_NOP);		   
	LABYTE(nADBuff,0) = spi_rw(ADS1246_NOP);
	
	//取消芯片选择
	ADS1246ChannelClose();
	
	//开中断
	OS_EXIT_CRITICAL();
	
	//数据扩展为4字节
	if(0 < (LABYTE(nADBuff,2) & 0x80)) LABYTE(nADBuff,3) = 0xFF;
	else LABYTE(nADBuff,3) = 0x00;

	return nADBuff;
}


/*****************************************************************************************************************
* 名	称：Int2ISR()
* 功	能：ADS1246转换完成中断程序
* 入口参数：无
* 出口参数：无
* 说	明：AD中断响应
*****************************************************************************************************************/
void EXTI9_5_IRQHandler()
{
	//进入中断		 
	OSIntEnter();
	
	
	//通道1
	if(EXTI_GetITStatus(ADC1_EXTI_LINE) != RESET)
	{
		g_ADC_IT_Channel |= 0x01;
		EXTI_ClearITPendingBit(ADC1_EXTI_LINE);
		OSSemPost(pADS1246Event);			
	}

	//通道2
	if(EXTI_GetITStatus(ADC2_EXTI_LINE) != RESET)
	{
		g_ADC_IT_Channel |= 0x02;
		EXTI_ClearITPendingBit(ADC2_EXTI_LINE);
		OSSemPost(pADS1246Event);
	}
	
	//退出中断
	OSIntExit();					
}

void EXTI15_10_IRQHandler()
{
	//进入中断		 
	OSIntEnter();
	
	//通道3
	if(EXTI_GetITStatus(ADC3_EXTI_LINE) != RESET)
	{
 		g_ADC_IT_Channel |= 0x04;	
		EXTI_ClearITPendingBit(ADC3_EXTI_LINE);	
		OSSemPost(pADS1246Event);
	}

	//通道4
	if(EXTI_GetITStatus(ADC4_EXTI_LINE) != RESET)
	{
		g_ADC_IT_Channel |= 0x08;
		EXTI_ClearITPendingBit(ADC4_EXTI_LINE);
		OSSemPost(pADS1246Event);
	}
	
	//退出中断
	OSIntExit();							
}

void ADC_IT_Disable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//禁止中断响应
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelCmd = DISABLE;
	NVIC_Init(&NVIC_InitStructure);	 
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_Init(&NVIC_InitStructure);
	
	//禁止中断请求	 
	EXTI->IMR &= ~(EXTI_Line6 | EXTI_Line8 | EXTI_Line11 | EXTI_Line13);
}

void ADC_IT_Eable(void)
{
	NVIC_InitTypeDef NVIC_InitStructure;
	
	//使能中断请求
	EXTI->IMR |= (EXTI_Line6 | EXTI_Line8 | EXTI_Line11 | EXTI_Line13);
	
	//使能中断响应
	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;	 
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);	 
	
	NVIC_InitStructure.NVIC_IRQChannel = EXTI15_10_IRQn;
	NVIC_Init(&NVIC_InitStructure);
}
