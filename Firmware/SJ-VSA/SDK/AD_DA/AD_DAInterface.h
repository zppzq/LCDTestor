/************************************************************************************
**																			 
**																			 
**																			 
**--------�� �� �� Ϣ----------------------------------------------------------------
**��   ��   ����AD&DAInterface.h														 
**��   ��   �ˣ�����ǿ															 
**�� ��  ʱ �䣺2009��4��14��													 
**����޸�ʱ�䣺															      
**��        ��������AD�Ľӿ��ļ�
************************************************************************************/
#ifndef _AD_DA_INTERFACE_H_
#define _AD_DA_INTERFACE_H_

#include "DataAcq.h"
#include "STM32ADC.h"
#include "ADS1246.h"

//���ݲɼ�����=========================================================

//
//ʹ��ADS1246
//
#ifdef	_ADS1246_H_

//Ĭ�ϵ�AD������ѹֵ
#define	F_AD_STEP					F_ADS1246_STEP

//��������
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
//ʹ��ADS1234
//
#ifdef	_ADS1234_H_

//Ĭ�ϵ�AD������ѹֵ
#define	F_AD_STEP					F_ADS1234_STEP

//��������
#define	ADPortInit()				ADS1234PortInit()
#define	ADVariInit()				ADS1234VariInit()
#define	ADPreInit()					ADS1234PreInit()
#define	ADOpen()					ADS1234Open()
#define	ADClose()					ADS1234Close()
#define	ADSetSampleRate(x,y)		ADS1234SpeedSelect(x,y)
#define	ADSetRange(x,y)				ADS1234SetRange(x,y)
#define	ADSetGain(x,y)				ADS1234SetGain(x,y)
#define	ADGetGain(x)				ADS1234GetGain(x)
#define	ADStartSample(x)			ADS1234StartSample(x)		//x,ͨ��
#define	ADStopSample(x)				ADS1234StopSample(x)		//x,ͨ��
#define	ADLock()					ADS1234Lock()
#define	ADUnLock()					ADS1234UnLock()
#define	ADGetVol()					ADS1234GetVol()
#define	ADParamUpdate(x,y)			ADS1234SetParam(x,y)		//x,������y,ͨ��

#endif






//��ѹ��ⲿ��=========================================================
#define	ADPowerPortInit()	ADInit()
#define	ADPowerGetVol()		GetADVolValue()



//�ļ�����
#endif
