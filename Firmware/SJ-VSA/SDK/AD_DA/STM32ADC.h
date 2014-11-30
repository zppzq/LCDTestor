/****************************************Copyright (c)**************************************************
**                              
**                                 
**                                  
**--------------文件信息--------------------------------------------------------------------------------
**文   件   名: F340AD.h
**创   建   人: 杨承凯
**创 建 日  期: 2007年11月2日
**最后修改日期: 2007年11月2日
**描        述: 
********************************************************************************************************/
#ifndef		_STM32ADC_h__		
#define		_STM32ADC_h__	

#ifdef		_F340_AD_C_
#define		F340_AD_EXT
#else
#define		F340_AD_EXT		extern
#endif

//ADC相关参数的宏变量========================================================================
#define   	AD0_REF  		((float)2.44)		//参考电压

//--------Compile Control------------------------------------------------------
#define		AD_INIT
#define 	GET_AD_VALUE
#define 	GET_AD_VOL_VALUE
//#define 	GET_POWER_VOL_VALUE
//#define		GET_TEMPER_VALUE

//#define CAL_RES_VALUE
//#define CAL_CURRENT_VALUE
//#define CUURENT_ADJUST

//--------Function Declare-----------------------------------------------------
extern	void		 	ADInit(void) reentrant;
extern 	unsigned int 	GetADValue(void) reentrant;
extern  float 			GetADVolValue(void) reentrant;
extern  float 			GetVolValue(void) reentrant;
extern	float		 	CalCurrentValue(unsigned int nDAValue) reentrant;
extern	void		 	CurrentAdjust(void) reentrant;
void Stm32ADCDisable(void) reentrant;
void Stm32ADCEnable(void) reentrant;


#endif	  //_STM32ADC_h__
