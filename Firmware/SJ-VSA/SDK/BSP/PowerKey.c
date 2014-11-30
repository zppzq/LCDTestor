/****************************************Copyright (c)************************************************************
**                              
**                         			深圳市生基科技有限公司
**                                  
**--------------文件信息------------------------------------------------------------------------------------------
**文   件   名: PowerKey.c
**创   建   人: 杨承凯
**创 建 日  期: 2011年1月5日
**最后修改日期: 2011年1月5日
**描        述: 开关控制
*****************************************************************************************************************/
#include "includes.h"
#include "bsp\PowerKey.h"
#include "bsp\Lights.h"

static OS_EVENT 	*pPowerEvent;						    	//事件控制块
static uint8 		nPowerErr;									//错误标志

//按键初始化
void PowerKeyInit(void) reentrant
{
	GPIO_InitTypeDef GPIO_InitStructure;
	//开启电压控制脚，开启
	MakePushPull(POWER_CTR);
	SetHi(POWER_CTR);

	//电压检测脚
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_10MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
	GPIO_Init(GPIOC, &GPIO_InitStructure);
	//映射中断
	GPIO_EXTILineConfig(GPIO_PortSourceGPIOC, GPIO_PinSource2);	  	//ADRDY4

	//配置输入中断
	
	EXTI->FTSR |= EXTI_Line2;
}

void PowerIntDisable(void) reentrant
{
	EXTI->IMR &= EXTI_Line2;		
}

void PowerIntEnable(void) reentrant
{
	EXTI->IMR |= EXTI_Line2;	
}

/****************************************************************************
* 名	称：LightsInit()
* 功	能：指示灯初始化
* 入口参数：无
* 出口参数：无
* 说	明：无
****************************************************************************/
void PowerKeyVarInit(void) reentrant
{
	//创建信号量
	pPowerEvent = OSSemCreate(0);
}

//关机
void PowerDown(void) reentrant
{
	SetLo(POWER_CTR);
}

void PowerEventClear(void) reentrant
{
	pPowerEvent->OSEventCnt = 0;
}

void TaskPowerKey(void *nouse) reentrant
{
	nouse = nouse;

	//事务处理
	while(1)
	{
		PowerIntEnable();
		//等待按键事件
		OSSemPend(pPowerEvent, 0, &nPowerErr);

		PowerIntDisable();
		//延迟半秒确认
		OSTimeDly(OS_TICKS_PER_SEC/2);

		//是否仍然按下
//		if(GPIO_ReadInputDataBit(GPIOC, GPIO_Pin_2) != 0)
		if(GetSignal(POWER_KEY) != 0)
		{
			//已放开，忽略
			continue;
		}

		//指示灯亮1秒后关机
		PowerLightOn();	 
		SigLightOn();
		OSTimeDly(OS_TICKS_PER_SEC);
		PowerLightOff();
		SigLightOff();

		//关机
		PowerDown();
		while(1);
	}
}

extern BOOL	bSleep;
extern int16	nWakeCount;

//按键中断处理
void EXTI2_IRQHandler()
{
	//进入中断		 
	OSIntEnter();
	
	//通道3
	if(EXTI_GetITStatus(EXTI_Line2) != RESET)
	{
		EXTI_ClearITPendingBit(EXTI_Line2);
		bSleep = FALSE;		//退出休眠
		nWakeCount = 0;	
		if(GetSignal(POWER_KEY) == 0)
		{
			OSSemPost(pPowerEvent);
		}
	}
	//退出中断
	OSIntExit();							
}






