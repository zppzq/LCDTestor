/****************************************Copyright (c)************************************************************
**                              
**                                 
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: Lights.h
**创   建   人: 杨承凯
**创 建 日  期: 2008年9月17日
**最后修改日期: 2008年9月17日
**描        述: 指示灯控制函数
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
