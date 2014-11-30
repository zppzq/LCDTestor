/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------文 件 信 息--------------------------------------------------------**
**文   件   名：AD.c														 **
**创   建   人：															 **
**创 建  时 间：2007.6.1													 **
**最后修改时间：													 		 **
**描        述：AD头文件												 	 **
******************************************************************************/

#ifndef 	_F930_AD_H_
#define 	_F930_AD_H_

#ifdef		_F930_AD_C_
#define		F930_AD_EXT
#else
#define		F930_AD_EXT		extern
#endif


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
extern	void		 	F930ADInit(void) reentrant;
extern 	unsigned int 	F930GetADValue(void) reentrant;
extern	float 		F930GetADVol(void) reentrant;
extern	float 		F930GetPowerVolValue(void) reentrant;
extern	float		 	F930CalCurrentValue(unsigned int nDAValue) reentrant;
extern	void		 	F930CurrentAdjust(void) reentrant;


#endif
