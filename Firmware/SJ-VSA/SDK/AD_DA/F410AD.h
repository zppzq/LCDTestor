/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------�� �� �� Ϣ--------------------------------------------------------**
**��   ��   ����AD.c														 **
**��   ��   �ˣ�															 **
**�� ��  ʱ �䣺2007.6.1													 **
**����޸�ʱ�䣺													 		 **
**��        ����ADͷ�ļ�												 	 **
******************************************************************************/

#ifndef 	_F410_AD_H_
#define 	_F410_AD_H_

#ifdef		_F410_AD_C_
#define		F410_AD_EXT
#else
#define		F410_AD_EXT		extern
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
extern	void		 	ADInit(void) reentrant;
extern 	unsigned int 	GetADValue(void) reentrant;
extern  float 			GetADVolValue(void) reentrant;
extern  float 			GetVolValue(void) reentrant;
extern	float		 	CalCurrentValue(unsigned int nDAValue) reentrant;
extern	void		 	CurrentAdjust(void) reentrant;


#endif