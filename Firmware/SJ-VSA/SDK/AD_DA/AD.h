/************************************************************************************
**																			 
**																			 
**																			 
**--------文 件 信 息----------------------------------------------------------------
**文   件   名：														 
**创   建   人：															 
**创 建  时 间：2008.4.10													 
**最后修改时间：															      
**描        述：
************************************************************************************/

#ifndef _AD_H_
#define _AD_H_



//-----------------------------------------------------------------------------------
// Compile Control MACROS
//-----------------------------------------------------------------------------------
#define	_ADC_INIT_
#define _GET_AD_VALUE_
#define _GET_POWER_CURRENT_
//#define _GET_RF_POWER_
#define _FTOA_


//-----------------------------------------------------------------------------------
// functions
//-----------------------------------------------------------------------------------
extern	void 			ADC_Init() reentrant;
extern	unsigned int 	GetADValue(void) reentrant;
extern	float 			GetPowerCurrent(void) reentrant;
extern	float 			GetRFPower(void) reentrant;
extern 	void 			ftoa(float f, INT8U *dat) reentrant;

#endif