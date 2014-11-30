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
#include "bsp.h"
#include "F410AD.h"

/******************************************************************************
**名	称：AD_Init()																 
**功	能：AD初始化																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef AD_INIT
void ADInit(void) reentrant
{
	ADC0CN &= 0xFC;			// 选择写AD0BUSY作为AD开始信号
	ADC0CN &= 0xBF;			// 选择正常模式

	// tracking mode 用默认方式
	ADC0TK &= 0xF0;			// 使用Pre-Tracking Mode
	ADC0TK |= 0x08;
	ADC0CF &= 0x07;			// 系统时钟为49152000，SAR时钟为49152000/5
	ADC0CF |= 0x20;
//	ADC0CF &= 0xF9;			// 累计采样16次
//	ADC0CF |= 0x06;

	//选择ADC0连接到的引脚
	ADC0MX  = 0x0C;			// 选择P1.4为AD输入

	//电源电压检测
	P1MDIN &= 0xEF;			// P1.4为模拟输入
	P1SKIP |= 0x10;			// 交叉开关跳过P1.4
	P1 	   |= 0x10;

	ADC0CN = 0x80;			// 使能ADC0，Burst Mode禁止，Right justified,AD0BUSY
}
#endif


/******************************************************************************
**名	称：GetADValue()																 
**功	能：AD初始化																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef GET_AD_VALUE
unsigned int GetADValue(void) reentrant
{
	unsigned int nADValue = 0;

	//--------启动ADC0---------------------------------------------------------
	while(AD0BUSY);
	AD0BUSY = 1;

	while(!AD0INT);		//等待转换完毕
	AD0INT = 0;

	nADValue = ADC0H;
	nADValue = nADValue << 8;
	nADValue += ADC0L;

	return nADValue;
}
#endif

/******************************************************************************
**名	称：GetADVolValue()																 
**功	能：AD初始化																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef GET_AD_VOL_VALUE
float GetADVolValue(void) reentrant
{
	float f;
	unsigned int n;
	n = GetADValue();

	f = (float)n * 2.49f / 4096.0f;

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
float GetPowerVolValue(void) reentrant
{
	float f;
	unsigned int n = GetADValue();

	f = (float)n * 2.49f / 4096.0f;

	f *= 11;

	return f;
}
#endif


/******************************************************************************
**名	称：GetTemperValue()																 
**功	能：																 
**入口参数：无														 
**出口参数：无															 
**说	明：无															 
******************************************************************************/
#ifdef GET_TEMPER_VALUE
float GetTemperValue(void) reentrant
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
float CalCurrentValue(unsigned int nADValue) reentrant
{
	float temp;
	float fCurrentValue = 0.0;
	temp = ((float)nADValue/4096)*VOL_REF;

	//计算电流值
	fCurrentValue = ((float)nADValue/4096)*VOL_REF/3.7445/CalResValue(CurrentValue);
	return fCurrentValue*1.06;
}
#endif

