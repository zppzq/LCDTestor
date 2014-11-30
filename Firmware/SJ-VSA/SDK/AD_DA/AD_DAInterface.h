/************************************************************************************
**																			 
**																			 
**																			 
**--------文 件 信 息----------------------------------------------------------------
**文   件   名：AD&DAInterface.h														 
**创   建   人：罗仕强															 
**创 建  时 间：2009年4月14日													 
**最后修改时间：															      
**描        述：用于AD的接口文件
************************************************************************************/
#ifndef _AD_DA_INTERFACE_H_
#define _AD_DA_INTERFACE_H_

#include "DataAcq.h"
#include "STM32ADC.h"
#include "ADS1246.h"

//数据采集部分=========================================================

//
//使用ADS1246
//
#ifdef	_ADS1246_H_

//默认的AD步进电压值
#define	F_AD_STEP					F_ADS1246_STEP

//函数定义
#define	ADPortInit()				ADS1246PortInit()
#define	ADVariInit()				ADS1246VariInit()
#define	ADOpen()					ADS1246Open()
#define	ADClose()					ADS1246Close()
#define	ADSetSampleRate(x,y)		ADS1246SetSampleRate(x,y)
#define	ADSetRange(x,y)				ADS1246SetRange(x,y)
#define	ADSetGain(x,y)				ADS1246SetGain(x,y)
#define	ADGetGain(x)				ADS1246GetGain(x)
#define	ADStartSample()				ADS1246StartSample()		
#define	ADStopSample()				ADS1246StopSample()
#define	ADGetVol(x)					ADS1246GetVol(x)
#define	ADWaitDataReady()			ADS1246WaitDataReady()
#define	ADSensorDetection(x)		ADS1246SensorDetection(x)

#endif


//
//使用ADS1234
//
#ifdef	_ADS1234_H_

//默认的AD步进电压值
#define	F_AD_STEP					F_ADS1234_STEP

//函数定义
#define	ADPortInit()				ADS1234PortInit()
#define	ADVariInit()				ADS1234VariInit()
#define	ADPreInit()					ADS1234PreInit()
#define	ADOpen()					ADS1234Open()
#define	ADClose()					ADS1234Close()
#define	ADSetSampleRate(x,y)		ADS1234SpeedSelect(x,y)
#define	ADSetRange(x,y)				ADS1234SetRange(x,y)
#define	ADSetGain(x,y)				ADS1234SetGain(x,y)
#define	ADGetGain(x)				ADS1234GetGain(x)
#define	ADStartSample(x)			ADS1234StartSample(x)		//x,通道
#define	ADStopSample(x)				ADS1234StopSample(x)		//x,通道
#define	ADLock()					ADS1234Lock()
#define	ADUnLock()					ADS1234UnLock()
#define	ADGetVol()					ADS1234GetVol()
#define	ADParamUpdate(x,y)			ADS1234SetParam(x,y)		//x,参数；y,通道

#endif






//电压监测部分=========================================================
#define	ADPowerPortInit()	ADInit()
#define	ADPowerGetVol()		GetADVolValue()



//文件结束
#endif
