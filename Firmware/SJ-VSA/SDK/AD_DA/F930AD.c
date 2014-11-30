/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------文 件 信 息--------------------------------------------------------**
**文   件   名：AD.c														 **
**创   建   人：															 **
**创 建  时 间：2007.6.1													 **
**最后修改时间：													 		 **
**描        述：AD源文件												 	 **
******************************************************************************/

//--------Includes-------------------------------------------------------------
#include "..\Global.h"
#include "F930AD.h"

#define F_F930AD_STEP	0.00322265625;			// 3.3f / 1024.0f

#if		(PCB_VERSION == SJ_RFQC_SC_4D_3_1)		//3.1改线板

//电源测量开关控制
#define	PW_AD_EN_PORT	PORT(2)
#define	PW_AD_EN		BIT(5)

//测量输入端口
#define	PW_AD_PORT		PORT(2)
#define	PW_AD			BIT(4)

#elif	(PCB_VERSION == SJ_RFQC_SC_4D_3_3)		//3.3板

#define	PW_AD_EN_PORT	PORT(2)
#define	PW_AD_EN		BIT(6)

//测量输入端口
#define	PW_AD_PORT		PORT(2)
#define	PW_AD			BIT(5)

#endif

/******************************************************************************
**名	称：F930ADInit()																 
**功	能：930AD初始化																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef AD_INIT
void F930ADInit(void) reentrant
{
	uint8 tmp = 0;
	MakePushPull(PW_AD_EN);
	SetLo(PW_AD_EN);

	SetADIn(PW_AD);			//模拟输入
	MakeOpenDrain(PW_AD);
	
	ADC0CN &= 0xF8;			// 选择写AD0BUSY作为AD开始信号
	ADC0CN &= 0xBF;			// 选择正常模式

	// tracking mode 用默认方式
//	ADC0TK &= 0xF0;			// 使用Pre-Tracking Mode
//	ADC0TK |= 0x08;
//	ADC0CF &= 0x07;			// 系统时钟为49152000，SAR时钟为49152000/5
	ADC0CF |= 0x01;			// PGA = 1
//	ADC0CF &= 0xF9;			// 累计采样16次
//	ADC0CF |= 0x06;

	//选择ADC0连接到的引脚
	tmp = (GETPORT(PW_AD) << 3);
	tmp += GETPORTIO(PW_AD);
	ADC0MX  = tmp;			// 选择AD输入

	//电源电压检测
	SetHi(PW_AD);

	ADC0CN = 0x80;			// 使能ADC0，Burst Mode禁止，,AD0BUSY

	ADC0AC = 0x00;			// Right justified
	REF0CN	&= 0xEF;			// 使用VDD做参考电压
}
#endif


/******************************************************************************
**名	称：F930GetADValue()																 
**功	能：AD初始化																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef GET_AD_VALUE
unsigned int F930GetADValue(void) reentrant
{
	unsigned int nADValue = 0;

	// --------启动ADC0---------------------------------------------------------
	while(AD0BUSY);
	AD0BUSY = 1;

	while(!AD0INT);		//等待转换完毕
	AD0INT = 0;

	OS_ENTER_CRITICAL();
	nADValue = ADC0H;
	nADValue = nADValue << 8;
	nADValue += ADC0L;
	OS_EXIT_CRITICAL();

	return nADValue;
}
#endif

/******************************************************************************
**名	称：F930GetADVol()																 
**功	能：AD初始化																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef GET_AD_VOL_VALUE
float F930GetADVol(void) reentrant
{
	float f;
	uint16 n;

	// 电源控制------------------------------------
	// 打开内部AD
	ADC0CN |= 0x80;
	
	SetADIn(PW_AD);			//模拟输入
	MakeOpenDrain(PW_AD);

	// 电源电压检测
	SetHi(PW_AD);
	MakePushPull(PW_AD_EN);
	// --------------------------------------------

	
	//开电压监测电源
	SetHi(PW_AD_EN);
	OSTimeDly(1);
	
	//获取AD值
	n = F930GetADValue();

	// 电源控制------------------------------------
	MakeOpenDrain(PW_AD);
	MakeOpenDrain(PW_AD_EN);
	SetLo(PW_AD);
	SetLo(PW_AD_EN);

	// 关闭内部AD
	ADC0CN &= ~0x80;
	// --------------------------------------------

	//计算电压值
	f = (float)n * F_F930AD_STEP;

	//关电压监测电源
	SetLo(PW_AD_EN);


	return f;
}
#endif

/******************************************************************************
**名	称：GetPowerVolValue()																 
**功	能：																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef GET_POWER_VOL_VALUE
float F930GetPowerVolValue(void) reentrant
{
	float f;
	unsigned int n = GetADValue();

	f = (float)n * 2.49f / 4096.0f;

	f *= 11;

	return f;
}
#endif


/******************************************************************************
**名	称：GetPowerVolValue()																 
**功	能：																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef GET_TEMPER_VALUE
float F930GetTemperValue(void) reentrant
{
	float f;
	unsigned int n = GetADValue();

	f = (float)n/4095*2.49;

	f *= 11;

	return f;
}
#endif


/******************************************************************************
**名	称：CalCurrentValue()																 
**功	能：计算电流值																 
**入口参数：AD的采样值																 
**出口参数：电流值															 
**说	明：无														 
******************************************************************************/
#ifdef CAL_CURRENT_VALUE
float F930CalCurrentValue(unsigned int nADValue) reentrant
{
	float temp;
	float fCurrentValue = 0.0;
	temp = ((float)nADValue/4096)*VOL_REF;

	//计算电流值
	fCurrentValue = ((float)nADValue/4096)*VOL_REF/3.7445/CalResValue(CurrentValue);
	return fCurrentValue*1.06;
}
#endif

