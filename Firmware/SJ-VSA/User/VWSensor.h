/******************************************************************************
**																			 **
**																			 **
**																			 **
**--------文 件 信 息--------------------------------------------------------**
**文   件   名： VWSensor.h												 **
**创   建   人： 蒋小明														 **
**创 建  时 间： 2008-09-28													 **
**最后修改时间：													 		 **
**描        述： 														 	 **
******************************************************************************/

#ifndef 	_VW_SENSOR_H_
#define 	_VW_SENSOR_H_

#ifdef		_VW_SENSOR_C_
#define		VW_SENSOR_EXT
#else
#define		VW_SENSOR_EXT		extern
#endif

// -------- 编译控制 ----------------------------------------------------------
#define _VW_SENSOR_INIT_
#define _VW_SENSOR_GET_VALUE_


#define VWS_MIN_FREQ				400.00
#define VWS_MAX_FREQ				6000.00

#define VWS_SYNT_MAXTIMES			25			// 谐振最多次数，如果都没收敛，则强制退出
#define VWS_COLLECT_COUNT			20			// 要拾取正弦波的个数

#define FREQ_VALID_LIMIT			50			// 筛选频率时的频率范围
//#define FREQ_OVER_VALUE_LIMIT		0.08		// 最终频率有效的范围
#define FREQ_STEP_INVALID			50			// 无有效电平时的频率步进
#define FREQ_STEP_NO_SUCCEED		50			// 不能完成50次测试的频率步进
#define FREQ_STEP_LESS_TIMES		50			// 不能达到有效次数的频率步进


VW_SENSOR_EXT void		VWSWakenSetFreq(float fFreq) reentrant;
VW_SENSOR_EXT void 		VWSWakenStart() reentrant;
VW_SENSOR_EXT void 		VWSWakenStop() reentrant;
VW_SENSOR_EXT void 		VWSensorInit() reentrant;
VW_SENSOR_EXT void 		VWSCollectStart() reentrant;
VW_SENSOR_EXT void 		VWSCollectStop() reentrant;
VW_SENSOR_EXT int16		VWSGetValue() reentrant;
VW_SENSOR_EXT float 	VWSGetFreq() reentrant;
VW_SENSOR_EXT void 		VWSDelay(unsigned long nDly) reentrant;
VW_SENSOR_EXT void 		VWSetParam(fp32* pParam) reentrant;
VW_SENSOR_EXT void 		VWSSetZero() reentrant;

#endif