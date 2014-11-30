/****************************************Copyright (c)************************************************************
**                              
**                         			深圳市生基科技有限公司
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: PowerKey.h
**创   建   人: 杨承凯
**创 建 日  期: 2011年1月5日
**最后修改日期: 2011年1月5日
**描        述: 开关控制
*****************************************************************************************************************/
#ifndef 	_POWER_KEY_H_
#define 	_POWER_KEY_H_


//电源开关控制脚
#define	POWER_CTR_PORT		GPIOC
#define	POWER_CTR			3

//电源开关检测脚
#define	POWER_KEY_PORT		GPIOC
#define	POWER_KEY			2



void PowerKeyInit(void) reentrant;
void PowerDown(void) reentrant;
void PowerEventClear(void) reentrant;
void PowerKeyVarInit(void) reentrant;

#endif


