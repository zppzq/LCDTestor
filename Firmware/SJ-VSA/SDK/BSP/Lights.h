/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------�ļ���Ϣ------------------------------------------------------------------------------------------
**��   ��   ��: Lights.h
**��   ��   ��: ��п�
**�� �� ��  ��: 2008��9��17��
**����޸�����: 2008��9��17��
**��        ��: ָʾ�ƿ��ƺ���
*****************************************************************************************************************/
#ifndef 	_LIGHTS_H_
#define 	_LIGHTS_H_

#ifdef		_LIGHTS_C_
#define		LIGHTS_EXT
#else
#define		LIGHTS_EXT		extern
#endif
//****************************************************************************************************************
void LightsInit(void) reentrant;
void PostLightOn(uint16 nTicksSpan) reentrant;
void LightsProcess(void) reentrant;
void SigLightOn(void) reentrant;
void SigLightOff(void) reentrant;
void PowerLightOn(void) reentrant;
void PowerLightOff(void) reentrant;

//****************************************************************************************************************
#endif
